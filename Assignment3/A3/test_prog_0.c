//
// Created by lapdoc on 24/11/18.
//
#define _GNU_SOURCE
#define STACK_SIZE 4096
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <sched.h>
#include <seccomp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/capability.h>
#include <sys/mount.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <linux/capability.h>
#include <linux/limits.h>
#include <stdbool.h>

int childFunc(void *arg) {
    while(1) {
        printf("In child\n");
        sleep(5);
    }
}

int main(int argc, char **argv) {
    printf("chmod with S_ISUID\n");
    chmod("hello.txt", S_ISUID);
    return 0;
}

