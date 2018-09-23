#include "myheader.h"

//Function prototype
void my_system_version_f(char *line, char *myfifo);

int main(int argc, char *argv[]) {
    char *myfifo;

    if(argc != 2) {
        printf("Usage error %s <path_to_fifo>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    myfifo = argv[1];

    setbuf(stdout, NULL);
    while(1) {
        printf("Enter Command>");
        char *line = get_a_line();
        if(length(line) > 1) {
            double startTime = getTime();
            my_system_version_f(line, myfifo);
            double endTime = getTime();
            printf("Time taken for command %f microseconds\n", endTime-startTime);
        } else {
            exit(0);
        }
    }
}

/**
 * Implementation of the system() function using fork
 * @param line The command input by the user in the shell
 * @param myfifo Name of the FIFO to be used for inter process communication
 */
void my_system_version_f(char *line, char *fifo) {
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
}