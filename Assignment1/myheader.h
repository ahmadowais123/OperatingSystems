#ifndef _MYHEADER
#define _MYHEADER

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sched.h>
#include <fcntl.h>
#include <time.h>

/**
 * Method to get the current time in milliseconds
 * @return Return the current time in milliseconds
 */
double getTime() {
	struct timespec *ts = malloc(sizeof(struct timespec));
	if(clock_gettime(CLOCK_REALTIME, ts) == -2) {
		perror("Error getting time");
	}

	double time = (ts->tv_sec*1000.0) + (ts->tv_nsec/1000000.0);
	return time; 
}

/**
 * This function parses and tokenizes the input command
 * @param line The command input by the user in the shell
 * @param args The array of strings in which the tokens are stored
 */
void parse_command(char *line, char *args[]) {
    char *token;
    int i =0;

    while ((token = strsep(&line, " \t\n")) != NULL) {
        for (int j = 0; j < strlen(token); j++)
            if (token[j] <= 32)
                token[j] = '\0';
        if (strlen(token) > 0)
            args[i++] = token;
    }
    return;
}

/**
 * Helper method to initialize the arguments array to NULL pointers
 * @param args The array to be initialized
 */
void initialize(char *args[]) {
    int i;
    for(i=0; i<20; i++) {
        args[i] = NULL;
    }
}

/**
 * Returns the length of a string
 * @param line The command input by the user in the shell
 * @return Length of string
 */
int length(char *line) {
    return strlen(line);
}

/**
 * Uses the stdin to get the command that the user wants to run
 * @return The command input by the user in the shell
 */
char* get_a_line() {
    char *line;
    size_t lineSize = 0;
    getline(&line, &lineSize, stdin);
    return line;
}



#endif