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

static int childFunc(void *arg) {
    char *line = (char *)arg;
    char *args[20];
    initialize(args);
    parse_command(line, args);
    execvp(args[0], args);
    free(line);
}

void my_system_version_c(char *line) {
    void *stack, *stackTop;
    int flags;
    pid_t pid;
    pid_t wait;
    int status;
    flags |= CLONE_VFORK;

    stack = malloc(STACK_SIZE);
    if(stack == NULL) {
        perror("Could not allocate memory for the stack\n");
        exit(EXIT_FAILURE);
    }

    stackTop = stack + STACK_SIZE;

    pid = clone(childFunc , stackTop, flags, line);

    wait = waitpid(pid, &status, 0);
    if(status == -1 || wait == -1) {
        perror("Child process failed during execution\n");
    }
    free(stack);
}