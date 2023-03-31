#ifndef __SCHEDULERS_H__
#define __SCHEDULERS_H__

#include "queue.h"

typedef struct {
    queue_t* source_queue;
    int time_quantum;
    queue_t* history_queue;
} scheduler_args_t;

void* fcfs(void* args);
void* round_robin(void* args);
void* sjf();

#endif // __SCHEDULERS_H__
