/**
 *  @title      :   sr_container.c
 *  @author     :   Shabir Abdul Samadh (shabirmean@cs.mcgill.ca)
 *  @date       :   20th Nov 2018
 *  @purpose    :   COMP310/ECSE427 Operating Systems (Assingment 3) - Phase 2
 *  @description:   A template C code to be filled in order to spawn container instances
 *  @compilation:   Use "make container" with the given Makefile
*/

#include "sr_container.h"

/**
 *  The cgroup setting to add the writing task to the cgroup
 *  '0' is considered a special value and writing it to 'tasks' asks for the wrinting 
 *      process to be added to the cgroup. 
 *  You must add this to all the controls you create so that it is added to the task list.
 *  See the example 'cgroups_control' added to the array of controls - 'cgroups' - below
 **/  
struct cgroup_setting self_to_task = {
	.name = "tasks",
	.value = "0"
};

/**
 *  ------------------------ TODO ------------------------
 *  An array of different cgroup-controllers.
 *  One controller has been been added for you.
 *  You should fill this array with the additional controls from commandline flags as described 
 *      in the comments for the main() below
 *  ------------------------------------------------------
 **/
struct cgroups_control *cgroups[6] = {NULL, NULL, NULL, NULL, NULL, NULL};

/**
 *  ------------------------ TODO ------------------------
 *  The SRContainer by default suppoprts three flags:
 *          1. m : The rootfs of the container
 *          2. u : The userid mapping of the current user inside the container
 *          3. c : The initial process to run inside the container
 *
 *   You must extend it to support the following flags:
 *          1. C : The cpu shares weight to be set (cpu-cgroup controller)                              DONE
 *          2. s : The cpu cores to which the container must be restricted (cpuset-cgroup controller)   DONE
 *          3. p : The max number of process's allowed within a container (pid-cgroup controller)       DONE
 *          4. M : The memory consumption allowed in the container (memory-cgroup controller)           DONE
 *          5. r : The read IO rate in bytes (blkio-cgroup controller)                                  DONE
 *          6. w : The write IO rate in bytes (blkio-cgroup controller)                                 DONE
 *          7. H : The hostname of the container                                                        DONE
 *
 *   You can follow the current method followed to take in these flags and extend it.
 *   Note that the current implementation necessitates the "-c" flag to be the last one.
 *   For flags 1-6 you can add a new 'cgroups_control' to the existing 'cgroups' array
 *   For 7 you have to just set the hostname parameter of the 'child_config' struct in the header file
 *  ------------------------------------------------------
 **/
int main(int argc, char **argv)
{
    struct child_config config = {0};
    void *stack, *stackTop;
    char temp[256];
    int option = 0;
    int sockets[2] = {0};
    pid_t child_pid = 0;
    int last_optind = 0;
    bool found_cflag = false;

    while ((option = getopt(argc, argv, "m:u:c:C:s:p:M:r:w:H:")))
    {
        if (found_cflag)
            break;

        switch (option)
        {
            case 'm':
                config.mount_dir = optarg;
                break;
            case 'u':
                if (sscanf(optarg, "%d", &config.uid) != 1)
                {
                    fprintf(stderr, "UID not as expected: %s\n", optarg);
                    cleanup_stuff(argv, sockets);
                    return EXIT_FAILURE;
                }
                break;
            case 'c':
                addCgroupSetting(CGRP_BLKIO_CONTROL, "blkio.weight", "64");
                config.argc = argc - last_optind - 1;
                config.argv = &argv[argc - config.argc];
                found_cflag = true;
                break;
            case 'C':
                addCgroupSetting(CGRP_CPU_CONTROL, "cpu.shares", optarg);
                break;
            case 's':
                addCgroupSetting(CGRP_CPU_SET_CONTROL, "cpuset.mems", "0");
                addCgroupSetting(CGRP_CPU_SET_CONTROL, "cpuset.cpus", optarg);
                break;
            case 'p':
                addCgroupSetting(CGRP_PIDS_CONTROL, "pids.max", optarg);
                break;
            case 'M':
                addCgroupSetting(CGRP_MEMORY_CONTROL, "memory.limit_in_bytes", optarg);
                break;
            case 'r':
                memset(temp, 0, strlen(temp));
                strcat(temp, "8:0 ");
                strcat(temp, optarg);
                addCgroupSetting(CGRP_BLKIO_CONTROL, "blkio.throttle.read_bps_device", temp);
                break;
            case 'w':
                memset(temp, 0, strlen(temp));
                strcat(temp, "8:0 ");
                strcat(temp, optarg);
                addCgroupSetting(CGRP_BLKIO_CONTROL, "blkio.throttle.write_bps_device", temp);
                break;
            case 'H':
                config.hostname = optarg;
                break;
            default:
                cleanup_stuff(argv, sockets);
                return EXIT_FAILURE;
        }
        last_optind = optind;
    }

    if (!config.argc || !config.mount_dir){
        cleanup_stuff(argv, sockets);
        return EXIT_FAILURE;
    }

    fprintf(stderr, "####### > Checking if the host Linux version is compatible...");
    struct utsname host = {0};
    if (uname(&host))
    {
        fprintf(stderr, "invocation to uname() failed: %m\n");
        cleanup_sockets(sockets);
        return EXIT_FAILURE;
    }
    int major = -1;
    int minor = -1;
    if (sscanf(host.release, "%u.%u.", &major, &minor) != 2)
    {
        fprintf(stderr, "major minor version is unknown: %s\n", host.release);
        cleanup_sockets(sockets);
        return EXIT_FAILURE;
    }
    if (major != 4 || (minor < 7))
    {
        fprintf(stderr, "Linux version must be 4.7.x or minor version less than 7: %s\n", host.release);
        cleanup_sockets(sockets);
        return EXIT_FAILURE;
    }
    if (strcmp(ARCH_TYPE, host.machine))
    {
        fprintf(stderr, "architecture must be x86_64: %s\n", host.machine);
        cleanup_sockets(sockets);
        return EXIT_FAILURE;
    }
    fprintf(stderr, "%s on %s.\n", host.release, host.machine);

    if (socketpair(AF_LOCAL, SOCK_SEQPACKET, 0, sockets))
    {
        fprintf(stderr, "invocation to socketpair() failed: %m\n");
        cleanup_sockets(sockets);
        return EXIT_FAILURE;
    }
    if (fcntl(sockets[0], F_SETFD, FD_CLOEXEC))
    {
        fprintf(stderr, "invocation to fcntl() failed: %m\n");
        cleanup_sockets(sockets);
        return EXIT_FAILURE;
    }
    config.fd = sockets[1];

    /**
     * ------------------------ TODO ------------------------
     * This method here is creating the control groups using the 'cgroups' array
     * Make sure you have filled in this array with the correct values from the command line flags 
     * Nothing to write here, just caution to ensure the array is filled
     * ------------------------------------------------------
     **/
    if (setup_cgroup_controls(&config, cgroups))
    {
        clean_child_structures(&config, cgroups, NULL);
        cleanup_sockets(sockets);
        return EXIT_FAILURE;
    }

    /**
     * ------------------------ TODO ------------------------
     * Setup a stack and create a new child process using the clone() system call
     * Ensure you have correct flags for the following namespaces:
     *      Network, Cgroup, PID, IPC, Mount, UTS (You don't need to add user namespace)
     * Set the return value of clone to 'child_pid'
     * Ensure to add 'SIGCHLD' flag to the clone() call
     * You can use the 'child_function' given below as the function to run in the cloned process
     * HINT: Note that the 'child_function' expects struct of type child_config.
     * ------------------------------------------------------
     **/

    //Allocate the stack for our cloned process
    stack = malloc(STACK_SIZE);
    if(stack == NULL) {
        perror("Failed to allocate memory for the stack.");
        clean_child_structures(&config, cgroups, NULL);
        cleanup_sockets(sockets);
        return EXIT_FAILURE;
    }
    stackTop = stack + STACK_SIZE;

    //Set the appropriate flags to isolate the namespaces for the container
    int flags = CLONE_NEWNET | CLONE_NEWCGROUP | CLONE_NEWIPC | CLONE_NEWNS | CLONE_NEWUTS | CLONE_NEWPID | SIGCHLD;
    child_pid = clone(child_function, stackTop, flags, &config);

    /**
     *  ------------------------------------------------------
     **/ 
    if (child_pid == -1)
    {
        fprintf(stderr, "####### > child creation failed! %m\n");
        clean_child_structures(&config, cgroups, stack);
        cleanup_sockets(sockets);
        return EXIT_FAILURE;
    }
    close(sockets[1]);
    sockets[1] = 0;

    if (setup_child_uid_map(child_pid, sockets[0]))
    {
        if (child_pid)
            kill(child_pid, SIGKILL);
    }

    int child_status = 0;
    waitpid(child_pid, &child_status, 0);
    int exit_status = WEXITSTATUS(child_status);

    clean_child_structures(&config, cgroups, stack);
    cleanup_sockets(sockets);
    return exit_status;
}


int child_function(void *arg)
{
    struct child_config *config = arg;
    if (sethostname(config->hostname, strlen(config->hostname)) || \
                setup_child_mounts(config) || \
                setup_child_userns(config) || \
                setup_child_capabilities() || \
                setup_syscall_filters()
        )
    {
        close(config->fd);
        return -1;
    }
    if (close(config->fd))
    {
        fprintf(stderr, "invocation to close() failed: %m\n");
        return -1;
    }
    if (execve(config->argv[0], config->argv, NULL))
    {
        fprintf(stderr, "invocation to execve() failed! %m.\n");
        return -1;
    }
    return 0;
}

/**
* Adds a new control group to the cgroups array if it does not already exist.
* Returns the index of the new cgroup or of the existing one and adds the tasks
* setting to the new cgroup
* @param controlBlock Name of the control block
* @return index of cgroup
*/
int addCgroup(char *controlBlock) {
    int index = 0;
    while(cgroups[index] != NULL) {
        if(strcmp(controlBlock, cgroups[index]->control) == 0) return index;
        index++;
    }

    struct cgroups_control *newCgroup = malloc(sizeof(struct cgroups_control));
    struct cgroup_setting **temp;
    temp = malloc(sizeof(struct cgroup_setting *)*2);
    for(int i=0; i<2; i++) {
        *(temp + i) = malloc(sizeof(struct cgroup_setting));
    }

    strcpy(newCgroup->control, controlBlock);

    temp[0] = &self_to_task;
    temp[1] = NULL;

    newCgroup->settings = temp;
    cgroups[index] = newCgroup;
    cgroups[index+1] = NULL;
    return index;
}

/**
 * Adds the control group setting to the approprate cgroup
 * @param controlBlock name of control group
 * @param settingName name of setting
 * @param settingValue value of setting
 */
void addCgroupSetting(char *controlBlock, char *settingName, char *settingValue) {
    int index = addCgroup(controlBlock);
    int settingsIndex = 0;
    struct cgroup_setting **settingsPointer = cgroups[index]->settings;

    while(strcmp(settingsPointer[settingsIndex]->name, "tasks") != 0) {
        settingsIndex++;
    }

    settingsPointer[settingsIndex+2] = malloc(sizeof(struct cgroup_setting));

    settingsPointer[settingsIndex] = settingsPointer[settingsIndex+2];
    strcpy(settingsPointer[settingsIndex]->name, settingName);
    strcpy(settingsPointer[settingsIndex]->value, settingValue);
    settingsPointer[settingsIndex+1] = &self_to_task;
    settingsPointer[settingsIndex+2] = NULL;
    return;
}