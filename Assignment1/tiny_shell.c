//
// Created by lapdoc on 27/09/18.
//
#include "myheader.h"

#define STACK_SIZE 1024

//Function Prototypes
void my_system(char *line);
static int clone_function(void *arg);

//Global Variables
char *fifo;
char *exitString = "exit\n";

int main(int argc, char *argv[]) {

#if defined(PIPE_READ) || defined(PIPE_WRITE)
    if( argc != 2) {
        printf("Usage error %s <path_to_fifo>\n", argv[0]);
        exit(0);
    }
#endif

    fifo = argv[1];
    setbuf(stdout, NULL);
    while(1) {
        char *cwd = get_current_working_dir();
        printf("%s$ ", cwd);
        char *line = get_a_line();
        if(length(line) > 1) {

            //Check if user wants to exit the shell
            if(strcasecmp(line, exitString) == 0) {
                exit(0);
            }

            my_system(line);
            free(line);
            free(cwd);
//            double startTime = getTime();
//            double endTime = getTime();
//            printf("Time taken for command %f milliseconds\n", endTime-startTime);

        } else {
            exit(0);
        }
    }
}

/**
 * Definition of function that is run by the cloned process
 * @param arg The command input by the user in the shell
 * @return returns the status code of the executed command
 */
static int clone_function(void *arg) {
    char *line = (char *)arg;
    char *args[20];
    initialize(args);
    parse_command(line, args);

    int status = 0;
    if(strcmp(args[0], "cd") == 0) {
        status = chdir(args[1]);
    } else  {
        status = execvp(args[0], args);
    }
    free(line);
    return status;
}

/**
 * Implementation of the system() method using clone
 * @param line The command input by the user in the shell
 */
void my_system(char *line) {
#ifdef FORK
    int status;
    char *args[20];

    //Spawn the child process
    int pid = fork();

    if(pid == 0) {
        initialize(args);
        parse_command(line, args);

        execvp(args[0], args);
        return;
    } else {
        waitpid(pid, &status, 0);
        if(status == -1) {
            perror("Child process failed during execution\n");
        }
    }
#elif VFORK
    int status;
    char *args[20];

    //Spawn the child process
    int pid = vfork();

    if(pid == 0) {
        initialize(args);
        parse_command(line, args);

        execvp(args[0], args);
        return;
    } else {
        waitpid(pid, &status, 0);
        if(status == -1) {
            perror("Child process failed during execution\n");
        }
    }
#elif CLONE
    void *stack, *stackTop;
    int flags;
    pid_t pid;
    int status = 0;
    flags |= CLONE_FS;

    //Allocate the stack for the child process
    stack = malloc(STACK_SIZE);
    if(stack == NULL) {
        perror("Could not allocate memory for the stack\n");
        exit(EXIT_FAILURE);
    }

    //Point stack pointer to the top of the stack
    stackTop = stack + STACK_SIZE;

    //Spawn the child process
    pid = clone(clone_function , stackTop, flags, line);

    //Wait for the child process to finish
    pid_t wait = waitpid(pid, &status, 0);
    if(status == -1) {
        perror("Child process failed during execution\n");
    }

    //Free up memory allocated to the stack
    free(stack);
    return;
#elif PIPE_READ
    int status;
    char *args[20];

    int pid = fork();

    if(pid == 0) {
        //Create a file descriptor for the FIFO
        int fd_in = open(fifo, O_RDONLY);

        //Duplicate the file descriptor to the stdin file descriptor
        dup2(fd_in, 0);

        //Close the FIFO file descriptor since we only need one
        //although both can be used interchangeably
        close(fd_in);
        initialize(args);
        parse_command(line, args);

        execvp(args[0], args);
        return;
    } else {
        waitpid(pid, &status, 0);
        if(status == -1) {
            perror("Child process failed during execution\n");
        }
    }
#elif PIPE_WRITE
    int status;
    char *args[20];

    int pid = fork();

    if(pid == 0) {
        //Create a file descriptor for the FIFO
        int fd_out = open(fifo, O_WRONLY);

        //Duplicate the file descriptor to the stdout file descriptor
        dup2(fd_out, 1);

        //Close the FIFO file descriptor since we only need one
        //although both can be used interchangeably
        close(fd_out);

        initialize(args);
        parse_command(line, args);

        execvp(args[0], args);
        return;
    } else {
        waitpid(pid, &status, 0);
        if(status == -1) {
            perror("Child process failed during execution\n");
        }
    }
#else
    system(line);
    return;
#endif
}