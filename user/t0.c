#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "common.h"
#include "common_threads.h"

void *mythread(void *arg) {
    printf("%s\n", (char *) arg);
    return NULL;
}

int main(int argc, char *argv[]) {                    
    if (argc != 1) {
    	fprintf(stderr, "usage: t0\n");
    	exit(1);
    }

    pthread_t p1, p2;
    printf("main: begin\n");
    Pthread_create(&p1, NULL, mythread, "A"); 
    Pthread_create(&p2, NULL, mythread, "B");
    // join waits for the threads to finish
    Pthread_join(p1, NULL); 
    printf("main: Termina A\n");
    Pthread_join(p2, NULL); 
    printf("main: Termina B\n");
    printf("main: end\n");
    return 0;
}

