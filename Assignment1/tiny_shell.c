//
// Author: Owais Khan
// McGill ID: 260617913
//
#include "myheader.h"

//Stack size for cloned process
#define STACK_SIZE 1024

//Function Prototypes
static int clone_function(void *arg);
void my_system(char *line);
int my_system_f(char *command);
int my_system_v(char *command);
int my_system_c(char *command);
int my_system_fifo_read(char *command);
int my_system_fifo_write(char *command);


//Global Variables
char *fifo;
char *exitString = "exit\n";

int main(int argc, char *argv[]) {

//Check if fifo specified as a command line argument
//when running the FIFO version
#if defined(PIPE_READ) || defined(PIPE_WRITE)
    if( argc != 2) {
        printf("Usage error %s <path_to_fifo>\n", argv[0]);
        exit(0);
    }
#endif

//Store fifo name is global variable
    fifo = argv[1];
    setbuf(stdout, NULL);

    //Infinite loop to keep prompting for commands
    while(1) {
        //Get the current working directory to form the prompt message
        char *cwd = get_current_working_dir();
        //Print the prompt message
        printf("%s$ ", cwd);
        //Get command to run from user
        char *line = get_a_line();

        //If command is valid then run
        if(length(line) > 1) {

            //Check if user wants to exit the shell
            if(strcasecmp(line, exitString) == 0) {
                exit(0);
            }

            //The commented code that surrounds the my_system call are used to calculate execution times
            //They are commented out to increase readability of the output
            /*double startTime = getTime();*/

            //Call the my_system() function
            //The implementation that gets called depends on the flag passed to the compiler
            //flag: implementation
            //system: system(), fork: fork(), vfork: vfork(), clone: clone()
            my_system(line);
            /*double endTime = getTime();*/
            /*printf("%f ms\n", endTime-startTime);*/

            free(line);
            free(cwd);
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
        if(status == -1) {
            exit(EXIT_FAILURE);
        }
    } else  {
        status = execvp(args[0], args);
        if(status == -1) {
            exit(EXIT_FAILURE);
        }
    }
    free(line);
    return status;
}

/**
 * Implementation of the system() method
 * @param line The command input by the user in the shell
 */
void my_system(char *line) {
#ifdef FORK
    my_system_f(line);
#elif VFORK
    my_system_v(line);
#elif CLONE
    my_system_c(line);
#elif PIPE_READ
    my_system_fifo_read(line);
#elif PIPE_WRITE
    my_system_fifo_write(line);
#else
    system(line);
    return;
#endif
}

/**
 * Implementation of the system() method using fork
 * @param line The command input by the user in the shell
 */
int my_system_f(char *line) {
    int status;
    char *args[20];

    //Spawn the child process
    int pid = fork();

    if(pid == 0) {
        //CHILD PROCESS


        //Tokenize the input command
        initialize(args);
        parse_command(line, args);

        //Execute the command
        status = execvp(args[0], args);
        if(status == -1) {
            perror("Command failed");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    } else {
        //PARENT PROCESS

        //Wait for child to finish execution then return to main to get next command
        waitpid(pid, &status, 0);
        if(status == -1) {
            perror("Child process failed during execution\n");
        }
    }
}

/**
 * Implementation of the system() method using vfork
 * @param line The command input by the user in the shell
 */
int my_system_v(char *line) {
    int status;
    char *args[20];

    //Spawn the child process
    int pid = vfork();

    if(pid == 0) {
        //CHILD PROCESS

        //Tokenize the input command
        initialize(args);
        parse_command(line, args);

        //Execute the command and exit with success or failure status
        status = execvp(args[0], args);
        if(status == -1) {
            perror("Command Failed");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    } else {
        //PARENT PROCESS

        //Wait for child to finish execution then return to main to get next command
        waitpid(pid, &status, 0);
        if(status == -1) {
            perror("Child process failed during execution\n");
        }
    }
}

/**
 * Implementation of the system() method using clone
 * @param line The command input by the user in the shell
 */
int my_system_c(char *line) {
    void *stack, *stackTop;
    int flags;
    pid_t pid;
    int status = 0;
    flags = CLONE_FS;


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
    waitpid(pid, &status, __WCLONE);
    if(status == -1) {
        perror("Child process failed during execution\n");
    }

    //Free up memory allocated to the stack
    free(stack);
    return status;
}

/**
 * Implementation of the read version of FIFO piping
 * @param line The command input by the user in the shell
 */
int my_system_fifo_read(char *line) {
    int status;
    char *args[20];

    int pid = fork();

    if(pid == 0) {
        //CHILD PROCESS     

        //Create a file descriptor for the FIFO
        int fd_in = open(fifo, O_RDONLY);

        //Duplicate the file descriptor to the stdin file descriptor
        dup2(fd_in, 0);

        //Close the FIFO file descriptor since we only need one
        //although both can be used interchangeably
        close(fd_in);

        //Tokenize the input command
        initialize(args);
        parse_command(line, args);

        //Execute the command and exit with success or failure status
        status = execvp(args[0], args);
        if(status == -1) {
            perror("Command Failed");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    } else {
        //PARENT PROCESS

        //Wait for child to finish execution then return to main to get next command
        waitpid(pid, &status, 0);
        if(status == -1) {
            perror("Child process failed during execution\n");
        }
    }
}

/**
 * Implementation of the write version of FIFO piping
 * @param line The command input by the user in the shell
 */
int my_system_fifo_write(char *line) {
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

        //Tokenize the input command
        initialize(args);
        parse_command(line, args);

        //Execute the command and exit with success or failure status
        status = execvp(args[0], args);
        if(status == -1) {
            perror("Command failed.");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    } else {
        //PARENT PROCESS

        //Wait for child to finish execution then return to main to get next command
        waitpid(pid, &status, 0);
        if(status == -1) {
            perror("Child process failed during execution\n");
        }
    }
}
