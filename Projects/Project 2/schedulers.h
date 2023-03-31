#ifndef __SCHEDULERS_H__
#define __SCHEDULERS_H__

void* fcfs(void* queue);
void* round_robin(void* time_quantum);
void* sjf();

#endif // __SCHEDULERS_H__
