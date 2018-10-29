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
	fd = shm_open(databaseName, O_RDWR | O_CREAT, S_IRWXU);

	if(fd == -1) {
		perror("Error creating shared memory object.");
		return -1;
	}

	size_t size = sizeof(data) + (NUMBER_OF_PODS*NUMBER_OF_PODS*SIZE_OF_KV_PAIR);
	ftruncate(fd, size);
	memory = (char *)mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);

	if(memory == MAP_FAILED) {
		perror("Failed to map memory");
		exit(1);
	}

	mutex = sem_open("mutex", O_CREAT, S_IRWXU, 1);
	sem_read = sem_open("sem_read", O_CREAT, S_IRWXU, 1);


	data *bookKeeping = (data *)memory;
	if(bookKeeping->initialized == 0) {
		for(int i=0; i<256; i++) {
			bookKeeping->writeCounters[i] = 0;
			bookKeeping->readCounters[i] = 0;
		}
		bookKeeping->initialized = 1;
		bookKeeping->readCounter = 0;
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

	sem_wait(mutex);
	int index = hashFunction(key);

	data *bookKeeping = (data *)memory;
	size_t pairOffset = SIZE_OF_KV_PAIR * bookKeeping->writeCounters[index];
	size_t podOffset = SIZE_OF_POD*index;

	memcpy(memory+sizeof(data)+podOffset+pairOffset, key, KEY_MAX_LENGTH);
	memcpy(memory+sizeof(data)+podOffset+pairOffset+KEY_MAX_LENGTH, value, VALUE_MAX_LENGTH);

	bookKeeping->writeCounters[index]++;
	bookKeeping->writeCounters[index] = bookKeeping->writeCounters[index]%NUMBER_OF_PODS;

	sem_post(mutex);

	return 0;
}

/**
 * This method reads the next value corresponding to the key
 * @param key
 * @return
 */
char *kv_store_read(const char *key) {

	sem_wait(sem_read);
	data *bookKeeping = (data *)memory;
	bookKeeping->readCounter++;
	int rc = bookKeeping->readCounter;
	if(rc == 1) {
		sem_wait(mutex);
	}
	sem_post(sem_read);

	int index = hashFunction(key);
	char *duplicateValue;

    size_t podOffset = SIZE_OF_POD*index;

    for(int i=0;i<256;i++) {
        size_t pairOffset = SIZE_OF_KV_PAIR*bookKeeping->readCounters[index];

        if(memcmp(memory + sizeof(data) + podOffset + pairOffset, key, strlen(key)) == 0) {
            duplicateValue = strdup(memory + sizeof(data) + podOffset + pairOffset + KEY_MAX_LENGTH);
            bookKeeping->readCounters[index]++;
            bookKeeping->readCounters[index] = bookKeeping->readCounters[index]%256;

            sem_wait(sem_read);
            bookKeeping->readCounter--;
            rc = bookKeeping->readCounter;
            if(rc == 0) {
            	sem_post(mutex);
            }
            sem_post(sem_read);

            return duplicateValue;
        }

        bookKeeping->readCounters[index]++;
        bookKeeping->readCounters[index] = bookKeeping->readCounters[index]%256;
    }

	sem_wait(sem_read);
	bookKeeping->readCounter--;
	rc = bookKeeping->readCounter;
	if(rc == 0) {
		sem_post(mutex);
	}
	sem_post(sem_read);
    return NULL;
};

/**
 * This method reads all the values associated with a particular key
 * @param key
 * @return
 */
char **kv_store_read_all(const char *key) {
	sem_wait(sem_read);
	data *bookKeeping = (data *)memory;
	bookKeeping->readCounter++;
	int rc = bookKeeping->readCounter;
	if(rc == 1) {
		sem_wait(mutex);
	}
	sem_post(sem_read);

    char **allStrings = malloc(sizeof(char*));
    char *nextValue;

    int count = 0;
    int index = hashFunction(key);
	int currentReadPointer = bookKeeping->readCounters[index];
	size_t podOffset = SIZE_OF_POD * index;

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

	sem_wait(sem_read);
	bookKeeping->readCounter--;
	rc = bookKeeping->readCounter;
	if(rc == 0) {
		sem_post(mutex);
	}
	sem_post(sem_read);
	allStrings[count] = NULL;
	if(count == 0) return NULL;
    return allStrings;
}

/**
 * Delete the shared memory object
 */
void kv_delete_db() {
	long size = sizeof(data) + (NUMBER_OF_PODS*SIZE_OF_POD);
	sem_unlink("mutex");
	sem_unlink("sem_read");
	if(munmap(memory, size) == -1){
		perror("Failed to delete database");
		exit(1);
	};
	shm_unlink(databaseName);
	free(databaseName);
}