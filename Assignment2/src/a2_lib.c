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

//#include "config.h"
//int fd;
//SM* sharedMemory;
////sem_t * db;
////sem_t * mutex;
//
//int main() {
//    kv_store_create("databaseOwais");
//
//    //kv_store_write("hello", "world");
//    printf("%ld\n",sizeof(KVpair));
//}
//
///*
//Create or find the shared memory object if already exist and get memory location from mmap.
//smName : given name to create shared memory Object.
//*/
//int kv_store_create(char *smName){
//    fd = shm_open(smName, O_CREAT | O_RDWR, S_IRWXU);
//
//    if (fd == -1){
//        printf("Shared memory initialization failed\n");
//        return -1;
//    }
//
//    ftruncate(fd,sizeof(SM) + 15*numberOfPods * maximumOfRecords * sizeof(KVpair));
//    setSharedMemoryAddress();
//    initializeShareMemoryStruct();
//
//    return 0;
//}
//
////Map the shared memory struct with the address given by the kernel.
//int setSharedMemoryAddress(){
//
//    sharedMemory = (SM*) mmap(NULL, sizeof(SM) +  15*numberOfPods * maximumOfRecords * sizeof(KVpair), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
//
//    if (sharedMemory == MAP_FAILED){
//        printf("setting mmap() failed\n");
//        return -1;
//    }
//    return 0;
//}
//
///*Hash function to hash string key to a specific set of pods.
//word : key value of kv pair to be hash.
//return : integer value between 0 - number of pods.
//*/
//int hashFunction(char *word){
//    int hashAddress = 5381;
//    int counter =0;
//
//    for (counter = 0; word[counter] != '\0'; counter++){
//        hashAddress = ((hashAddress <<5) + hashAddress) + word[counter];
//    }
//    return hashAddress % numberOfPods < 0 ? -hashAddress % numberOfPods : hashAddress % numberOfPods;
//}
//
///*Write kv pair into store using a circular array approach for FIFO.
//key : key string to be written in store
//value : value string to be written in store
//*/
//int kv_store_write(char *key, char *value){
//
//    int podIndex = hashFunction(key);
//    //offset used to determine next location of kv pair
//    size_t offset = sizeOfRecord * sharedMemory->podCounters[podIndex];
//    //pod location offset depending on pod Index given by hash function
//    size_t podLocation = podSpace * podIndex;
//
////    if(sem_trywait(db) == -1){
////        sem_wait(db);
////    }
//    //copy key string into shared memory then value string.
//    memcpy(sharedMemory + sizeof(SM) + offset + podLocation, key, keySize);
//    memcpy(sharedMemory + sizeof(SM) + offset + podLocation + keySize, value , valueSize);
//
//    sharedMemory->podCounters[podIndex]++;
//    sharedMemory->podCounters[podIndex] = sharedMemory->podCounters[podIndex] % 256;
//
////    sem_post(db);
//    return 0;
//}
//
///*Find location of value depending on the key. If read all of specific pod's kv pairs and did not find key, returns a NULL
//If found a key in the pod, then next read will first check another value for the same key.
//Using circular array FIFO to read through all the pod's kv pairs.
//key : key string to be used to search for a value.
//*/
//char* kv_store_read(char *key){
//    int podIndex = hashFunction(key);
//    int i;
//    size_t offset;
//    size_t podLocation = podSpace * podIndex;
//    char* duplicated;
//
//    for (i = 0; i < podSize ; i++){
//        offset = sizeOfRecord * sharedMemory->podRead[podIndex];
//
//        if( memcmp( sharedMemory + sizeof(SM) + offset + podLocation , key, strlen(key)) == 0){
//            duplicated = strdup((char*) (sharedMemory + sizeof(SM) + podLocation +  offset + keySize));
//            sharedMemory->podRead[podIndex]++;
//            sharedMemory->podRead[podIndex] = sharedMemory->podRead[podIndex] % 256;
//            return duplicated;
//        }
//        else {
//            sharedMemory->podRead[podIndex]++;
//            sharedMemory->podRead[podIndex] = sharedMemory->podRead[podIndex] % 256;
//        }
//    }
//    return NULL;
//}
//
///*Find all values depending on the key. Reads all the specific pod's kv pair. If no kv pair found in pod, returns a NULL.
//Iterate through multiple kv_store_read call to get each value for a key in a specific pod.
//key : key string to be used to search for all value for that key.
//Return set of values for the key.
//*/
//char ** kv_store_read_all(char *key){
//
//    int podIndex = hashFunction(key);
//
//    char** allValuesOfKey = malloc(sizeof(char*));
//    char* value;
//
//    int index = 0;
//
//    while(1){
//
//        if(index == 0){
//            value = kv_store_read(key);
//            allValuesOfKey[index] = value;
//        }
//        else{
//            allValuesOfKey[index] = kv_store_read(key);
//        }
//
//        index++;
//        allValuesOfKey = realloc(allValuesOfKey, sizeof(char*) * (index + 1));
//
//        if (value == NULL){
//            return NULL;
//        }
//        else if (index != 1 && strcmp(value, allValuesOfKey[index - 1]) == 0){
//            allValuesOfKey[index - 1] = NULL;
//            return allValuesOfKey;
//        }
//    }
//    return NULL;
//}
//
////Initialize values for pod writes/reads and create semaphore if shared memory is first created.
//int initializeShareMemoryStruct(){
//    int i = 0;
//    //initialize the semaphores
////    db = sem_open("db", O_CREAT, S_IRUSR ,1);
////    mutex = sem_open("mutex", O_CREAT, S_IRUSR ,1);
//
//    //initialize the counters for the pods
//    for(i = 0 ; i < numberOfPods ; i++){
//        sharedMemory->podCounters[i] = 0;
//        sharedMemory->podRead[i] = 0;
//    }
//    return 0;
//}
////Delete the shared memory when function is called.
//int kv_delete_db(){
//
//    if(munmap(sharedMemory, sizeof(SM) + 15*numberOfPods * maximumOfRecords * sizeof(KVpair)) == -1){
//        printf("Error when deleting shared memory\n");
//        return -1;
//    }
//    return 0;
//}
