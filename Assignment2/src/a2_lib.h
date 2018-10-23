#ifndef __COMP310_ASSIGNMENT2_TEST_CASE_CONFIG__
#define __COMP310_ASSIGNMENT2_TEST_CASE_CONFIG__

#include <stdlib.h>

#define DATA_BASE_NAME   "database"
#define KEY_MAX_LENGTH   32
#define VALUE_MAX_LENGTH 256

#define numberPods 256
#define pairSize 288
#define keySize 32
#define valueSize 256
#define podSize 73728

char *memory;
int fd;

typedef struct {
    char key[KEY_MAX_LENGTH];
    char value[VALUE_MAX_LENGTH];
}kvpair;

typedef struct {
    int writeCounters[256];
    int readCounters[256];
    int initialized;
}data;


/* if you use the default interface uncomment below */
extern int  kv_store_create(const char *name);
extern int  kv_store_write(const char *key, const char *value);
extern char *kv_store_read(const char *key);
extern char **kv_store_read_all(const char *key);
extern void kv_delete_db();
extern unsigned long hashFunction(const char *key);

/* if you write your own interface, please fill the following adaptor */
//int    (*kv_store_create)(const char*)             = NULL;
//int    (*kv_store_write)(const char*, const char*) = NULL;
//char*  (*kv_store_read)(const char*)               = NULL;
//char** (*kv_store_read_all)(const char* key)       = NULL;

#endif