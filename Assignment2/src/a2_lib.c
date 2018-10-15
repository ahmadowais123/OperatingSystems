#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "a2_lib.h"

#define numberPods 256
#define keySize 32
#define memorySize 73000000000

data *memory;
int fd;

int main(int argc, char **argv) {
	if(kv_store_create(DATA_BASE_NAME) == -1) {
		exit(1);
	}

//	kv_store_write("0", "value1key0");
//	kv_store_write("0", "value2key0");
//	kv_store_write("255", "value1key255");
//	kv_store_write("255", "value2key255");

	printf("%s\n",kv_store_read("0"));
	printf("%s\n",kv_store_read("0"));
	printf("%s\n",kv_store_read("255"));
	printf("%s\n",kv_store_read("255"));
}

int  kv_store_create(const char *name) {
	fd = shm_open(name, O_RDWR | O_CREAT, S_IRWXU);

	if(fd == -1) {
		perror("Error creating shared memory object.");
		return -1;
	}

	ftruncate(fd, memorySize);
	memory = (data *)mmap(NULL, memorySize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	if(memory == MAP_FAILED) {
		perror("Failed to map memory");
		exit(1);
	}

//	for(int i=0; i<256; i++) {
//		memory->writeCounters[i] = 0;
//		memory->readCounters[i] = 0;
//	}

	return 0;
}

int hashFunction(char *key) {
	int hash = 5381;
	int c;

	while (c = *key++) {
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}

	return hash;
}

int kv_store_write(const char *key, const char *value) {
	int index = atoi(key);
	long pairOffset = sizeof(kvpair) * memory->writeCounters[index];
	long podOffset = sizeof(kvpair)*numberPods*index;


	memcpy(memory+sizeof(data)+podOffset+pairOffset, key, keySize);
	memcpy(memory+sizeof(data)+podOffset+pairOffset+keySize, value, numberPods);

	memory->writeCounters[index] = memory->writeCounters[index] + 1;
	memory->writeCounters[index] = memory->writeCounters[index]%numberPods;

	return 0;
}

char *kv_store_read(const char *key) {
	int index = atoi(key);
	int pairOffset = sizeof(kvpair)*memory->readCounters[index];
	int podOffset = sizeof(kvpair)*numberPods*index;

	char *dupString = (char *)malloc(sizeof(char)*256);
	memcpy(dupString, memory+sizeof(data)+podOffset+pairOffset+keySize, 256);

	memory->readCounters[index] = memory->readCounters[index] + 1;
	memory->readCounters[index] = memory->readCounters[index]%256;

	return dupString;
};

char **kv_store_read_all(const char *key) {
	char **string;
	return string;
}

void kv_delete_db() {
	if(munmap(memory, memorySize) == -1){
		perror("Failed to delete database");
		exit(1);
	}
}