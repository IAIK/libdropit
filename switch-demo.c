#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdint.h>
#include <sched.h>
#include <sys/mman.h>
#include <signal.h>
#include <unistd.h>

#include "libdropit.h"

typedef volatile int atomic_t;

atomic_t do_exploit = 1;
atomic_t start_exploit = 0;

typedef struct {
    unsigned int value;
} double_fetch_struct;

static void handler(int sig, siginfo_t *si, void *unused) {
    printf("Got SIGSEGV at address: 0x%lx\n",(long) si->si_addr);
    exit(1);
}

void* exploit(void* arg) {
    volatile int* ptr = &(((double_fetch_struct*) arg)->value);
    start_exploit = 1;
    while(do_exploit) {
        (*ptr) = 4;
        (*ptr) = 10000;
    }
    start_exploit = 0;
    return NULL;
}



void do_double_fetch(double_fetch_struct* s)
{
    volatile int ret = -1;

#ifndef DISABLE_DROPIT
    doublefetch_t config = doublefetch_init(1000000);
    doublefetch_start(config);
#endif
    // compiler generates a double fetch for "s->value"
    switch(s->value)
    {
            case 0: { ret = 0; break; }
            case 1: { ret = 1; break; }
            case 2: { ret = 2; break; }
            case 3: { ret = 3; break; }
            case 4: { ret = 4; break; }
            default: { ret = 5; break; }
    }
#ifndef DISABLE_DROPIT
    doublefetch_end(config, { printf("Too many tries to exploit the double-fetch, could not perform safe copy!\n"); exit(-1);});
#endif
    // idle for demo purposes
    sched_yield();
}

int main(int argc, char* argv[]) {
    double_fetch_struct s;

    // register SIGSEGV handler
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = handler;
    sigaction(SIGSEGV, &sa, NULL);

    pthread_t p;
    pthread_create(&p, NULL, exploit, (void*)&s);
    
    // wait for exploitation thread
    while(!start_exploit) {
        sched_yield();
    }

    // do the double fetch
    for(size_t try = 0; try < 10000; try++) {
        do_double_fetch(&s);
    }
    
    // wait for exploitation thread to end
    do_exploit = 0;
    while(start_exploit) {
        sched_yield();
    }
    
    // done
    printf("Double fetch was not exploited!\n");
    return 0;
}
