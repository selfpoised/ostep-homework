#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "common_threads.h"

typedef struct __Zem_t {
    int value;
    pthread_cond_t cond;
    pthread_mutex_t lock;
} Zem_t;

Zem_t z; 

// only one thread can call this
 void Zem_init(Zem_t *s, int value) {
    s->value = value;
    Cond_init(&s->cond);
    Mutex_init(&s->lock);
 }

 void Zem_wait(Zem_t *s) {
    Mutex_lock(&s->lock);
    while (s->value <= 0)
        Cond_wait(&s->cond, &s->lock);
    s->value--;
    Mutex_unlock(&s->lock);
 }

 void Zem_post(Zem_t *s) {
    Mutex_lock(&s->lock);
    s->value++;
    Cond_signal(&s->cond);
    Mutex_unlock(&s->lock);
 }
 
void *child(void *arg) {
    sleep(1);

    printf("child\n");

    // use semaphore here
    Zem_post(&z);

    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t p;
    printf("parent: begin\n");

    // init semaphore here
    Zem_init(&z, 0);

    Pthread_create(&p, NULL, child, NULL);

    // use semaphore here
    Zem_wait(&z);

    printf("parent: end\n");
    return 0;
}

// gcc -o fork-join fork-join.c -Wall -pthread