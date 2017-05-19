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

static void handler(int sig, siginfo_t *si, void *unused) {
    printf("Got SIGSEGV at address: 0x%lx\n",(long) si->si_addr);
    exit(1);
}

void* exploit(void* arg) {
    volatile char* ptr = (volatile char*) arg;
    start_exploit = 1;
    while(do_exploit) {
        ptr[4] ^= 'm';
    }
    start_exploit = 0;
    return NULL;
}

void do_double_fetch(char* string)
{
    char buffer[8];
    size_t len;
#ifndef DISABLE_DROPIT
    doublefetch_t config = doublefetch_init(1000000);
    doublefetch_start(config);
#endif
        // first access to string
        len = strlen(string);
        if(len < 8) {
            // idle for demonstration purposes
            //sched_yield();
            usleep(10);
            // second access to string
            strcpy(buffer, string);
        }
#ifndef DISABLE_DROPIT
    doublefetch_end(config, { printf("Too many tries to exploit the double-fetch, could not perform safe copy!\n"); exit(-1);});
#endif
}

int main(int argc, char* argv[]) {
    char string[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vestibulum molestie euismod justo egestas faucibus. Vestibulum posuere.";

    // register SIGSEGV handler
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = handler;
    sigaction(SIGSEGV, &sa, NULL);

    pthread_t p;
    pthread_create(&p, NULL, exploit, (void*)string);
    
    // wait for exploitation thread
    while(!start_exploit) {
        sched_yield();
    }

    // do the double fetch
    for(size_t try = 0; try < 10; try++) {
        do_double_fetch(string);
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
