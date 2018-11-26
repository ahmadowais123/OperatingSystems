//
// Created by lapdoc on 24/11/18.
//
#define _GNU_SOURCE
#define STACK_SIZE 4096

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sched.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int childFunc(void *arg) {
    printf("Inside Child\n");
}

int main(int argc, char **argv) {
    void *stack = malloc(1024);
    void *stackTop = stack + 1024;
    printf("Clone with CLONE_NEWUSER | CLONE_NEWPID\n");
    clone(childFunc, stackTop, CLONE_NEWUSER | CLONE_NEWPID, NULL);
    return 0;
}


