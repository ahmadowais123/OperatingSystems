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

double getTime() {
	struct timespec *ts = malloc(sizeof(struct timespec));
	if(clock_gettime(CLOCK_REALTIME, ts) == -2) {
		perror("Error getting time");
	}

	double time = (ts->tv_sec*1000.0) + (ts->tv_nsec/1000000.0);
	return time; 
}

int parse_command(char *line, char *args[]) {
    char *token;
    int i =0;

    while ((token = strsep(&line, " \t\n")) != NULL) {
        for (int j = 0; j < strlen(token); j++)
            if (token[j] <= 32)
                token[j] = '\0';
        if (strlen(token) > 0)
            args[i++] = token;
    }
    return i;
}

void initialize(char *args[]) {
    int i;
    for(i=0; i<20; i++) {
        args[i] = NULL;
    }
}

int length(char *line) {
    return strlen(line);
}

char* get_a_line() {
    char *line;
    size_t lineSize = 0;
    getline(&line, &lineSize, stdin);
    return line;
}



#endif