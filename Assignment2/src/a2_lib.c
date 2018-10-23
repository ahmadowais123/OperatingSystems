#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "a2_lib.h"

int kv_store_create(const char *name) {
	fd = shm_open(name, O_RDWR | O_CREAT, S_IRWXU);

	if(fd == -1) {
		perror("Error creating shared memory object.");
		return -1;
	}

	size_t size = sizeof(data) + (numberPods*numberPods*pairSize);
	ftruncate(fd, size);
	memory = (char *)mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);

	if(memory == MAP_FAILED) {
		perror("Failed to map memory");
		exit(1);
	}


	data *bookKeeping = (data *)memory;
	for(int i=0; i<256; i++) {
		bookKeeping->writeCounters[i] = 0;
		bookKeeping->readCounters[i] = 0;
	}

	return 0;
}

unsigned long hashFunction(const char *key) {
	unsigned long hash = 5381;
	int c;

	while(c = *key++) {
	    hash = ((hash << 5) + hash) + c;
	}

    hash = (hash>0) ? hash : -(hash);
	return hash%256;
}


int kv_store_write(const char *key, const char *value) {
	int index = hashFunction(key);

	data *bookKeeping = (data *)memory;
	size_t pairOffset = pairSize * bookKeeping->writeCounters[index];
	size_t podOffset = podSize*index;


	memcpy(memory+sizeof(data)+podOffset+pairOffset, key, keySize);
	memcpy(memory+sizeof(data)+podOffset+pairOffset+keySize, value, valueSize);

	bookKeeping->writeCounters[index]++;
	bookKeeping->writeCounters[index] = bookKeeping->writeCounters[index]%numberPods;

	return 0;
}

char *kv_store_read(const char *key) {
    int index = hashFunction(key);
    char *duplicateValue;

    data *bookKeeping = (data *)memory;
    size_t podOffset = podSize*index;

    for(int i=0;i<256;i++) {
        size_t pairOffset = pairSize*bookKeeping->readCounters[index];

        if(memcmp(memory + sizeof(data) + podOffset + pairOffset, key, strlen(key)) == 0) {
            duplicateValue = strdup(memory + sizeof(data) + podOffset + pairOffset + keySize);
            bookKeeping->readCounters[index]++;
            bookKeeping->readCounters[index] = bookKeeping->readCounters[index]%256;
            return duplicateValue;
        }

        bookKeeping->readCounters[index]++;
        bookKeeping->readCounters[index] = bookKeeping->readCounters[index]%256;
    }

    return NULL;
};

//char **kv_store_read_all(const char *key) {
//    char **allStrings = malloc(sizeof(char*));
//    char *nextValue;
//
//    int count = 0;
//    int index = hashFunction(key);
//    size_t podOffset = podSize * index;
//
//    for(int i=0; i<256; i++) {
//        size_t pairOffset = i*pairSize;
//
//        if(strcmp(memory+sizeof(data)+podOffset+pairOffset, "") == 0) {
//            if(i == 0) return NULL;
//            free(allStrings[count]);
//            allStrings[count] = NULL;
//            return allStrings;
//        } else if(strcmp(memory+sizeof(data)+podOffset+pairOffset, key) == 0) {
//            nextValue = strdup(memory + sizeof(data) + podOffset + pairOffset + keySize);
//            allStrings[count] = nextValue;
//            count++;
//            allStrings = realloc(allStrings, sizeof(char*)*(count+1));
//        }
//    }
//	allStrings[count] = NULL;
//    return allStrings;
//}

char **kv_store_read_all(const char *key) {
    char **allStrings = malloc(sizeof(char*));
    char *nextValue;

    data *bookKeeping = (data *)memory;

    int count = 0;
    int index = hashFunction(key);
	int currentReadPointer = bookKeeping->readCounters[index];
	size_t podOffset = podSize * index;

    for(int i=0; i<256; i++) {
        size_t pairOffset = currentReadPointer*pairSize;
		currentReadPointer++;
		currentReadPointer %= 256;
        if(strcmp(memory+sizeof(data)+podOffset+pairOffset, "") == 0) {
            if(i == 0) return NULL;
        } else if(strcmp(memory+sizeof(data)+podOffset+pairOffset, key) == 0) {
			count++;
			allStrings = realloc(allStrings, sizeof(char*)*(count+1));
			nextValue = strdup(memory + sizeof(data) + podOffset + pairOffset + keySize);
			allStrings[count-1] = nextValue;
        }
    }
//    free(allStrings[count]);
	allStrings[count] = NULL;
    return allStrings;
}

void kv_delete_db() {
	long size = sizeof(data) + (numberPods*podSize);
	if(munmap(memory, size) == -1){
		perror("Failed to delete database");
		exit(1);
	}
}