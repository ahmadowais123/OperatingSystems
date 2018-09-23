#include "myheader.h"

#define STACK_SIZE 1024

//Function Prototypes
void my_system_version_c(char *line);
static int childFunc(void *arg);

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    while(1) {
        printf("Enter Command>");
        char *line = get_a_line();
        if(length(line) > 1) {
            double startTime = getTime();
            my_system_version_c(line);
            double endTime = getTime();
            printf("Time taken for command %f milliseconds\n", endTime-startTime);
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
static int childFunc(void *arg) {
    char *line = (char *)arg;
    char *args[20];
    initialize(args);
    parse_command(line, args);
    int status = execvp(args[0], args);
    free(line);
    return status;
}

/**
 * Implementation of the system() method using clone
 * @param line The command input by the user in the shell
 */
void my_system_version_c(char *line) {
    void *stack, *stackTop;
    int flags;
    pid_t pid;
    pid_t wait;
    int status;
    flags |= CLONE_VFORK;

    //Allocate the stack for the child process
    stack = malloc(STACK_SIZE);
    if(stack == NULL) {
        perror("Could not allocate memory for the stack\n");
        exit(EXIT_FAILURE);
    }

    //Point stack pointer to the top of the stack
    stackTop = stack + STACK_SIZE;

    //Spawn the child process
    pid = clone(childFunc , stackTop, flags, line);

    //Wait for the child process to finish
    wait = waitpid(pid, &status, 0);
    if(status == -1 || wait == -1) {
        perror("Child process failed during execution\n");
    }

    //Free up memory allocated to the stack
    free(stack);
}