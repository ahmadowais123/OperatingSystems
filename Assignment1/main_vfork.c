#include "myheader.h"

//Function prototype
void my_system_version_v(char *line);

int main(int argc, char *argv[]) {

    setbuf(stdout, NULL);
    while(1) {
        printf("Enter Command>");
        char *line = get_a_line();
        if(length(line) > 1) {
            double startTime = getTime();
            my_system_version_v(line);
            double endTime = getTime();
            printf("Time taken for command %f milliseconds\n", endTime-startTime);
        } else {
            exit(0);
        }
    }
}

/**
 * Implemation of the system() function using vfork
 * @param line The command input by the user in the shell
 */
void my_system_version_v(char *line) {
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
}