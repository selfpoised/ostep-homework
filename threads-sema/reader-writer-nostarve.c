#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common_threads.h"
#include "zem.c"
#include <pthread.h>

//
// Your code goes in the structure and functions below
//

typedef struct __rwlock_t {
    Zem_t lock;
    int readers;
    int writers;
    int mode;
    Zem_t writelock;
} rwlock_t;


void rwlock_init(rwlock_t *rw) {
    rw->readers = 0;
    rw->writers = 0;
    rw->mode = 0;
    Zem_init(&rw->lock, 1);
    Zem_init(&rw->writelock, 1);
}

void rwlock_acquire_readlock(rwlock_t *rw) {
    Zem_wait(&rw->lock);
    rw->readers++;
    
    printf("thread:%li read acquire %d %d:\n", (unsigned long int)pthread_self(), rw->readers, rw->writers);
    if (rw->writers == 0){
        if(rw->readers == 1){
            Zem_wait(&rw->writelock);
            rw->mode = 1;
        }
    } else{
        if(rw->mode == 0){
            Zem_post(&rw->lock);
            Zem_wait(&rw->writelock);
            Zem_wait(&rw->lock);    
        }
    }
    
    Zem_post(&rw->lock);
}

void rwlock_release_readlock(rwlock_t *rw) {
    Zem_wait(&rw->lock);
    rw->readers--;

    printf("thread:%li read release %d %d:\n", (unsigned long int)pthread_self(), rw->readers, rw->writers);
    if (rw->mode == 1){
        if (rw->readers == 0) {
            Zem_post(&rw->writelock);
            rw->mode = 0;
        }
    } else{
        Zem_post(&rw->writelock);
    }

    Zem_post(&rw->lock);
}

void rwlock_acquire_writelock(rwlock_t *rw) {
    Zem_wait(&rw->lock);
    rw->writers++;
    printf("thread:%li write acquire %d %d:\n", (unsigned long int)pthread_self(), rw->readers, rw->writers);
    Zem_post(&rw->lock);
    
    Zem_wait(&rw->writelock);
}

void rwlock_release_writelock(rwlock_t *rw) {
    Zem_wait(&rw->lock);
    rw->writers--;
    printf("thread:%li write release %d %d:\n", (unsigned long int)pthread_self(), rw->readers, rw->writers);
    Zem_post(&rw->lock);

    Zem_post(&rw->writelock);
}

//
// Don't change the code below (just use it!)
// 

int loops;
int value = 0;

rwlock_t lock;

void *reader(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
        rwlock_acquire_readlock(&lock);
        printf("thread:%li read %d\n", (unsigned long int)pthread_self(), value);
        rwlock_release_readlock(&lock);
        if(i % 2 == 0){
            sleep(1);
        }
    }
    return NULL;
}

void *writer(void *arg) {
    int i;
    for (i = 0; i < loops; i++) {
        if(i % 3 == 0){
            sleep(1);
        }
        rwlock_acquire_writelock(&lock);
        value++;
        printf("thread:%li write %d\n", (unsigned long int)pthread_self(), value);
        rwlock_release_writelock(&lock);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    assert(argc == 4);
    int num_readers = atoi(argv[1]);
    int num_writers = atoi(argv[2]);
    loops = atoi(argv[3]);

    pthread_t pr[num_readers], pw[num_writers];

    rwlock_init(&lock);

    printf("begin\n");

    int i;
    for (i = 0; i < num_readers; i++)
	Pthread_create(&pr[i], NULL, reader, NULL);
    for (i = 0; i < num_writers; i++)
	Pthread_create(&pw[i], NULL, writer, NULL);

    for (i = 0; i < num_readers; i++)
	Pthread_join(pr[i], NULL);
    for (i = 0; i < num_writers; i++)
	Pthread_join(pw[i], NULL);

    printf("end: value %d\n", value);

    return 0;
}

// gcc -o reader-writer-nostarve reader-writer-nostarve.c -Wall -pthread
// ./reader-writer-nostarve 2 1 5