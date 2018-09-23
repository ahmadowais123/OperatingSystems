#include "myheader.h"

//Fucntion prototype
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

void my_system_version_v(char *line) {
    int pid = vfork();
    int status;
    char *args[20];

    initialize(args);
    parse_command(line, args);

    if(pid == 0) {
        execvp(args[0], args);
        return;
    } else {
        waitpid(pid, &status, 0);
        if(status == -1) {
            perror("Child process failed during execution\n");
        }
    }
}