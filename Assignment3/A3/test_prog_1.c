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

int main(int argc, char **argv) {
    printf("chmod with S_IRUSR | S_ISGID\n");
    chmod("hello.txt", S_IRUSR | S_ISGID);
    return 0;
}



