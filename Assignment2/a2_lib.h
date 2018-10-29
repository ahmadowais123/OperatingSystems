#ifndef __COMP310_ASSIGNMENT2_TEST_CASE_CONFIG__
#define __COMP310_ASSIGNMENT2_TEST_CASE_CONFIG__

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

#define KEY_MAX_LENGTH   32
#define VALUE_MAX_LENGTH 256
#define NUMBER_OF_PODS 256
#define SIZE_OF_KV_PAIR 288
#define SIZE_OF_POD 73728

char *memory;
char *databaseName;
int fd;

sem_t *mutex;
sem_t *sem_read;

typedef struct {
    int writeCounters[256];
    int readCounters[256];
    int initialized;
    int readCounter;
}data;


/* if you use the default interface uncomment below */
extern int  kv_store_create(const char *name);
extern int  kv_store_write(const char *key, const char *value);
extern char *kv_store_read(const char *key);
extern char **kv_store_read_all(const char *key);
extern void kv_delete_db();
extern unsigned long hashFunction(const char *key);

#endif