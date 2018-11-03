#include "a2_lib.h"

/**
 * Creates a shared memory object and maps it to memory
 * Initializes semaphores for synchronization and bookkeeping
 * information for reading and writing to the kv store
 * @param name Name of shared memory object
 * @return 0 on success, -1 on failure
 */
int kv_store_create(const char *name) {
	databaseName = malloc(sizeof(char)*strlen(name));
	strcpy(databaseName, name);
	
	//Create shared memory object
	fd = shm_open(databaseName, O_RDWR | O_CREAT, S_IRWXU);

	if(fd == -1) {
		perror("Error creating shared memory object.");
		return -1;
	}

	//Map memory to the share memory object
	size_t size = sizeof(data) + (NUMBER_OF_PODS*NUMBER_OF_PODS*SIZE_OF_KV_PAIR);
	ftruncate(fd, size);
	memory = (char *)mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);

	if(memory == MAP_FAILED) {
		perror("Failed to map memory");
		exit(1);
	}

	//Initiate semaphores
	writeLock = sem_open("writeLock", O_CREAT, S_IRWXU, 1);
	readLock = sem_open("readLock", O_CREAT, S_IRWXU, 1);


	//Initialize book keeping information
	data *bookKeeping = (data *)memory;
	if(bookKeeping->isInitialized == 0) {
		for(int i=0; i<256; i++) {
			bookKeeping->writeCounters[i] = 0;
			bookKeeping->readCounters[i] = 0;
		}
		bookKeeping->isInitialized = 1;
		bookKeeping->readers = 0;
	}

	return 0;
}

/**
 * This function maps a string to an index in the kvstore
 * @param key used for mapping
 * @return index of pod
 */
unsigned long hashFunction(const char *key) {
	unsigned long hash = 5381;
	int c;

	while(c = *key++) {
	    hash = ((hash << 5) + hash) + c;
	}

    hash = (hash>0) ? hash : -(hash);
	return hash%256;
}

/**
 * This method writes a key value pair to the store in the next available spot
 * in its respective pod
 * @param key
 * @param value
 * @return
 */
int kv_store_write(const char *key, const char *value) {

    //Obtain write lock
	sem_wait(writeLock);
	int index = hashFunction(key);

	//Calculate offset of position to write to
	data *bookKeeping = (data *)memory;
	size_t pairOffset = SIZE_OF_KV_PAIR * bookKeeping->writeCounters[index];
	size_t podOffset = SIZE_OF_POD*index;

	//Write to the store
	memcpy(memory+sizeof(data)+podOffset+pairOffset, key, KEY_MAX_LENGTH);
	memcpy(memory+sizeof(data)+podOffset+pairOffset+KEY_MAX_LENGTH, value, VALUE_MAX_LENGTH);

	//Update book keeping information
	bookKeeping->writeCounters[index]++;
	bookKeeping->writeCounters[index] = bookKeeping->writeCounters[index]%NUMBER_OF_PODS;

	//Release write lock
	sem_post(writeLock);

	return 0;
}

/**
 * This method reads the next value corresponding to the key
 * @param key
 * @return
 */
char *kv_store_read(const char *key) {

    //Obtain read lock
	sem_wait(readLock);
	data *bookKeeping = (data *)memory;
	bookKeeping->readers++;
	int rc = bookKeeping->readers;
	//If first reader then also obtain write lock
	if(rc == 1) {
		sem_wait(writeLock);
	}
	//Release read lock
	sem_post(readLock);

	int index = hashFunction(key);
	char *duplicateValue;

    size_t podOffset = SIZE_OF_POD*index;

    //Iterate over the pod to look for desired key
    for(int i=0;i<256;i++) {
        size_t pairOffset = SIZE_OF_KV_PAIR*bookKeeping->readCounters[index];

        //Compare input key with key in store
        if(memcmp(memory + sizeof(data) + podOffset + pairOffset, key, strlen(key)) == 0) {
            //Make duplicate of value and return
            duplicateValue = strdup(memory + sizeof(data) + podOffset + pairOffset + KEY_MAX_LENGTH);
			//Obtain read lock and update book keeping information
            sem_wait(readLock);
			bookKeeping->readCounters[index]++;
			bookKeeping->readCounters[index] = bookKeeping->readCounters[index]%256;

            bookKeeping->readers--;
            rc = bookKeeping->readers;
            //If last reader then release write lock
            if(rc == 0) {
            	sem_post(writeLock);
            }
            sem_post(readLock);

            return duplicateValue;
        }

        //Obtain read lock to update book keeping information
		sem_wait(readLock);
        bookKeeping->readCounters[index]++;
        bookKeeping->readCounters[index] = bookKeeping->readCounters[index]%256;
		sem_post(readLock);
    }

    //No value found for input key so return null
	sem_wait(readLock);
	bookKeeping->readers--;
	rc = bookKeeping->readers;
	if(rc == 0) {
		sem_post(writeLock);
	}
	sem_post(readLock);
    return NULL;
};

/**
 * This method reads all the values associated with a particular key
 * @param key
 * @return
 */
char **kv_store_read_all(const char *key) {
    //Obtain read lock
	sem_wait(readLock);
	data *bookKeeping = (data *)memory;
	bookKeeping->readers++;
	int rc = bookKeeping->readers;
	//If first reader obtain write lock as well
	if(rc == 1) {
		sem_wait(writeLock);
	}
	sem_post(readLock);

    char **allStrings = malloc(sizeof(char*));
    char *nextValue;

    int count = 0;
    int index = hashFunction(key);
	int currentReadPointer = bookKeeping->readCounters[index];
	size_t podOffset = SIZE_OF_POD * index;

	//Iterate through pod and copy all values corresponding to the input key
    for(int i=0; i<256; i++) {
        size_t pairOffset = currentReadPointer*SIZE_OF_KV_PAIR;
		currentReadPointer++;
		currentReadPointer %= 256;

        if(strcmp(memory+sizeof(data)+podOffset+pairOffset, key) == 0) {
			count++;
			allStrings = realloc(allStrings, sizeof(char*)*(count+1));
			nextValue = strdup(memory + sizeof(data) + podOffset + pairOffset + KEY_MAX_LENGTH);
			allStrings[count-1] = nextValue;
        }
    }

	sem_wait(readLock);
	bookKeeping->readers--;
	rc = bookKeeping->readers;
	if(rc == 0) {
		sem_post(writeLock);
	}
	sem_post(readLock);
	allStrings[count] = NULL;
	if(count == 0) return NULL;
    return allStrings;
}

/**
 * Delete the shared memory object
 */
void kv_delete_db() {
	long size = sizeof(data) + (NUMBER_OF_PODS*SIZE_OF_POD);
	sem_unlink("writeLock");
	sem_unlink("readLock");
	if(munmap(memory, size) == -1){
		perror("Failed to delete database");
		exit(1);
	};
	shm_unlink(databaseName);
	free(databaseName);
}