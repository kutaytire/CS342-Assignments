#ifndef __PCB_H__
#define __PCB_H__

#include <pthread.h>
#include <sys/_types/_pid_t.h>

typedef struct {
    int pid;
    int burst_length; // the length of the process in ms (PL)
    long long arrival_time;
    int remaining_time;
    long long finish_time;
    int turnaround_time;
    long long waiting_time;
    int id_of_processor; // id of the thread that is running the process
    int is_dummy;
} pcb_t;

#endif // __PCB_H__
