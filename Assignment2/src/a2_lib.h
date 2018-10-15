#ifndef __COMP310_ASSIGNMENT2_TEST_CASE_CONFIG__
#define __COMP310_ASSIGNMENT2_TEST_CASE_CONFIG__

#include <stdlib.h>

#define DATA_BASE_NAME   "database"
#define KEY_MAX_LENGTH   32
#define VALUE_MAX_LENGTH 256

typedef struct {
    char key[KEY_MAX_LENGTH];
    char value[VALUE_MAX_LENGTH];
}kvpair;

typedef struct {
    int writeCounters[256];
    int readCounters[256];
}data;

kvpair *pointer;

/* if you use the default interface uncomment below */
extern int  kv_store_create(const char *name);
extern int  kv_store_write(const char *key, const char *value);
extern char *kv_store_read(const char *key);
extern char **kv_store_read_all(const char *key);
extern void kv_delete_db();

/* if you write your own interface, please fill the following adaptor */
//int    (*kv_store_create)(const char*)             = NULL;
//int    (*kv_store_write)(const char*, const char*) = NULL;
//char*  (*kv_store_read)(const char*)               = NULL;
//char** (*kv_store_read_all)(const char* key)       = NULL;

#endif

//#ifndef _INCLUDE_A2_LIB_H_
//#define _INCLUDE_A2_LIB_H_
//
//#include <stdlib.h>
//#include <stdio.h>
//#include <unistd.h>
//#include <string.h>
//
//#include <fcntl.h>
//#include <sys/mman.h>
//#include <sys/stat.h>
//
//#include <semaphore.h>
//
//#define SharedMemoryName "/WN"
//
//#define keySize 32
//#define valueSize 256
//#define sizeOfRecord 288
//
//#define maximumOfRecords 65536
//
//#define numberOfPods 256
//#define podSize 256
//#define podSpace 73728
//
//typedef struct{
//    char key[32];
//    char value[256];
//
//}KVpair;
//
//typedef struct {
//
//    int rc;
//    int podCounters[256];
//    int podRead[256];
//    int initialized;
//}SM;
//
//
//int  kv_store_create(char *name);
//int  kv_store_write(char *key,char *value);
//char *kv_store_read(char *key);
//char **kv_store_read_all(char *key);
//int kv_delete_db();
//
//int setSharedMemoryAddress();
//int initializeShareMemoryStruct();
//
//
//#endif
