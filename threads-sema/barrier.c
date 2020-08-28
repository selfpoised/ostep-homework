#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common_threads.h"
#include "zem.c"

// If done correctly, each child should print their "before" message
// before either prints their "after" message. Test by adding sleep(1)
// calls in various locations.

// You likely need two semaphores to do this correctly, and some
// other integers to track things.

typedef struct __barrier_t {
    // add semaphores and other information here
    Zem_t z1; 
    Zem_t z2; 
    int count;
    int loop;
} barrier_t;


// the single barrier we are using for this program
barrier_t b;

void barrier_init(barrier_t *b, int num_threads) {
    // initialization code goes here
    Zem_init(&(b->z1), 1);
    Zem_init(&(b->z2), 0);
    b->count = num_threads;
    b->loop = num_threads;
}

void barrier(barrier_t *b) {
    // barrier code goes here
    Zem_wait(&(b->z1));
    int t = 0;
    b->count--;
    t = b->count;
    Zem_post(&(b->z1));

    if(t > 0){
        Zem_wait(&(b->z2));
    }else{
        for(int i=0;i<b->loop;i++){
            // sem_wait() decrements (locks) the semaphore pointed to by sem. 
            // If the semaphore's value is greater than zero, then the decrement proceeds, 
            // and the function returns, immediately. If the semaphore currently has the value zero, 
            // then the call blocks until either it becomes possible to perform the decrement (i.e., 
            // the semaphore value rises above zero), or a signal handler interrupts the call.
            Zem_post(&(b->z2));
        }
    }
}

//
// XXX: don't change below here (just run it!)
//
typedef struct __tinfo_t {
    int thread_id;
} tinfo_t;

void *child(void *arg) {
    tinfo_t *t = (tinfo_t *) arg;
    printf("child %d: before\n", t->thread_id);
    barrier(&b);
    printf("child %d: after\n", t->thread_id);
    return NULL;
}


// run with a single argument indicating the number of 
// threads you wish to create (1 or more)
int main(int argc, char *argv[]) {
    assert(argc == 2);
    int num_threads = atoi(argv[1]);
    assert(num_threads > 0);

    pthread_t p[num_threads];
    tinfo_t t[num_threads];

    printf("parent: begin\n");
    barrier_init(&b, num_threads);
    
    int i;
    for (i = 0; i < num_threads; i++) {
	t[i].thread_id = i;
	Pthread_create(&p[i], NULL, child, &t[i]);
    }

    for (i = 0; i < num_threads; i++) 
	Pthread_join(p[i], NULL);

    printf("parent: end\n");
    return 0;
}

// gcc -o barrier barrier.c -Wall -pthread