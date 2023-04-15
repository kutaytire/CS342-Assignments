#ifndef __UTIL_H__
#define __UTIL_H__

#include "pcb.h"
#include "queue.h"

long long gettimeofday_ms();
void print_pcb(pcb_t* pcb);
void print_for_outmode(pcb_t* pcb, long long time, char outmode);
void print_history_queue(queue_t* queue);

#endif // __UTIL_H__