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

	kv_store_write("0", "My name is Owais");
	kv_store_write("256", "My name is Khan");

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

int kv_store_write(const char *key, const char *value) {
	int index = atoi(key);
	long pairOffset = sizeof(kvpair) * memory->writeCounters[index];
	long podOffset = sizeof(kvpair)*numberPods*index;


	memcpy(memory+sizeof(data)+podOffset+pairOffset, key, keySize);
	memcpy(memory+sizeof(data)+podOffset+pairOffset+keySize, value, numberPods);

	memory->writeCounters[index] = memory->writeCounters[index] + 1;
	memory->writeCounters[index] = memory->writeCounters[index]%numberPods;
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

}

void kv_delete_db() {
	munmap(memory, memorySize);
}