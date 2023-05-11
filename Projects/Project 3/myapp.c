#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>
#include "rm.h"

#define NUMR 3        // number of resource types
#define NUMP 5        // number of threads

int AVOID = 1;
int exist[3] =  {10,5,7};  // resources existing in the system

void pr (int tid, char astr[], int m, int r[])
{
    int i;
    printf ("thread %d, %s, [", tid, astr);
    for (i=0; i<m; ++i) {
        if (i==(m-1))
            printf ("%d", r[i]);
        else
            printf ("%d,", r[i]);
    }
    printf ("]\n");
}


void setarray (int r[MAXR], int m, ...)
{
    va_list valist;
    int i;

    va_start (valist, m);
    for (i = 0; i < m; i++) {
        r[i] = va_arg(valist, int);
    }
    va_end(valist);
    return;
}

// t0
void *threadfunc0 (void *a)
{
    int tid;
    int request1[MAXR];
    int request2[MAXR];
    int claim[MAXR];

    tid = *((int*)a);
    rm_thread_started (tid);

    setarray(claim, NUMR, 7, 5, 3);
    rm_claim (claim);

    setarray(request1, NUMR, 0, 1, 0);
    pr (tid, "REQ", NUMR, request1);
    rm_request (request1);

    sleep(4);

    // setarray(request2, NUMR, 3);
    // pr (tid, "REQ", NUMR, request2);
    // rm_request (request2);

    // rm_release (request1);
    // rm_release (request2);

    printf("Thread ended execution time 0!");

    rm_thread_ended();
    rm_print_state("This one is for 0");

    pthread_exit(NULL);
}

// t1
void *threadfunc1 (void *a)
{
    int tid;
    int request1[MAXR];
    int request2[MAXR];
    int claim[MAXR];

    tid = *((int*)a);
    rm_thread_started (tid);

    setarray(claim, NUMR, 3, 2, 2);
    rm_claim (claim);

    setarray(request1, NUMR, 2, 0, 0);
    pr (tid, "REQ", NUMR, request1);
    rm_request (request1);

    sleep(15);

    setarray(request2, NUMR, 1,0,0);
    pr (tid, "REQ", NUMR, request2);
    rm_request (request2);

    // rm_release (request1);
    // rm_release (request2);

    printf("Thread ended execution time 1!");

    rm_thread_ended ();

    rm_print_state("This one is for 1");
    pthread_exit(NULL);
}

// t2
void *threadfunc2 (void *a)
{
    int tid;
    int request1[MAXR];
    int request2[MAXR];
    int claim[MAXR];

    tid = *((int*)a);
    rm_thread_started (tid);

    setarray(claim, NUMR, 9, 0, 2);
    rm_claim (claim);

    setarray(request1, NUMR, 3, 0, 2);
    pr (tid, "REQ", NUMR, request1);
    rm_request (request1);

    sleep(2);

    // setarray(request2, NUMR, 4);
    // pr (tid, "REQ", NUMR, request2);
    // rm_request (request2);

    // rm_release (request1);
    // rm_release (request2);

    printf("Thread ended execution time 2!");

    rm_thread_ended ();
    rm_print_state("This one is for 2");
    pthread_exit(NULL);
}

// t3
void *threadfunc3 (void *a)
{
    int tid;
    int request1[MAXR];
    int request2[MAXR];
    int claim[MAXR];

    tid = *((int*)a);
    rm_thread_started (tid);

    setarray(claim, NUMR, 2, 2, 2);
    rm_claim (claim);

    setarray(request1, NUMR, 2, 1, 1);
    pr (tid, "REQ", NUMR, request1);
    rm_request (request1);

    sleep(2);

    // setarray(request2, NUMR, 4);
    // pr (tid, "REQ", NUMR, request2);
    // rm_request (request2);

    // rm_release (request1);
    // rm_release (request2);

    printf("Thread ended execution time 3!");

    rm_thread_ended ();

    rm_print_state("This one is for 3");
    pthread_exit(NULL);
}

// t4
void *threadfunc4 (void *a)
{
    int tid;
    int request1[MAXR];
    int request2[MAXR];
    int claim[MAXR];

    tid = *((int*)a);
    rm_thread_started (tid);

    setarray(claim, NUMR, 4, 3, 3);
    rm_claim (claim);

    setarray(request1, NUMR, 0, 0, 2);
    pr (tid, "REQ", NUMR, request1);
    rm_request (request1);

    sleep(10);

    setarray(request2, NUMR, 3,3,0);
    pr (tid, "REQ", NUMR, request2);
    rm_request (request2);

    // rm_release (request1);
    // rm_release (request2);

    printf("Thread ended execution time 4!");

    rm_thread_ended ();

    rm_print_state("This one is for 4");
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    int i;
    int tids[NUMP];
    pthread_t threadArray[NUMP];
    int count;
    int ret;

    if (argc != 2) {
        printf ("usage: ./app avoidflag\n");
        exit (1);
    }

    AVOID = atoi (argv[1]);

    if (AVOID == 1)
        rm_init (NUMP, NUMR, exist, 1);
    else
        rm_init (NUMP, NUMR, exist, 0);

    i = 0;  // we select a tid for the thread
    tids[i] = i;
    pthread_create (&(threadArray[i]), NULL,
                    (void *) threadfunc0, (void *)
                    (void*)&tids[i]);

    i = 1;  // we select a tid for the thread
    tids[i] = i;
    pthread_create (&(threadArray[i]), NULL,
                    (void *) threadfunc1, (void *)
                    (void*)&tids[i]);

    i = 2;  // we select a tid for the thread
    tids[i] = i;
    pthread_create (&(threadArray[i]), NULL,
                    (void *) threadfunc2, (void *)
                    (void*)&tids[i]);

    i = 3;  // we select a tid for the thread
    tids[i] = i;
    pthread_create (&(threadArray[i]), NULL,
                    (void *) threadfunc3, (void *)
                    (void*)&tids[i]);

    i = 4;  // we select a tid for the thread
    tids[i] = i;
    pthread_create (&(threadArray[i]), NULL,
                    (void *) threadfunc4, (void *)
                    (void*)&tids[i]);

    count = 0;
    while ( count < 10) {
        sleep(5);
        rm_print_state("The current state");
        ret = rm_detection();
        printf("\n\nrm_detection=%d\n\n", ret);
        if (ret > 0) {
            printf ("deadlock detected, count=%d\n", ret);
            rm_print_state("state after deadlock");
        }
        count++;
    }

    if (ret == 0) {
        for (i = 0; i < NUMP; ++i) {
            pthread_join (threadArray[i], NULL);
            rm_print_state("After thread is finished\n");
            printf ("joined\n");
        }
    }
}