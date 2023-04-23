#ifndef __SCHEDULERS_H__
#define __SCHEDULERS_H__

#include "queue.h"
#include <pthread.h>
#include <stdio.h>

typedef struct {
    queue_t* source_queue;
    int time_quantum;
    queue_t* history_queue;
    int id_of_processor;
    char outmode;
    FILE* outfile;
    pthread_cond_t* queue_generator_cond;
    pthread_mutex_t* queue_generator_lock;
    pthread_mutex_t* history_queue_lock;
} scheduler_args_t;

void* fcfs(void* args);
void* sjf(void* args);
void* rr(void* args);

#endif // __SCHEDULERS_H__
