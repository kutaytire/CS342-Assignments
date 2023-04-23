#include "schedulers.h"
#include "pcb.h"
#include "queue.h"
#include "util.h"
#include <pthread.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern long long start_time;
int flag = 1;

void* fcfs(void* args) {
    scheduler_args_t* scheduler_args = (scheduler_args_t*)args;

    // Get the queue
    queue_t* queue = scheduler_args->source_queue;
    // Get the history queue
    queue_t* history_queue = scheduler_args->history_queue;

    int count = 0;

    while (1) {
        // Lock the queue
        pthread_mutex_lock(scheduler_args->queue_generator_lock);

        while (!(queue->size > 0)) {
            printf("%s%d%s\n", "Cpu ", scheduler_args->id_of_processor, " is waiting ");
            pthread_cond_wait(scheduler_args->queue_generator_cond,
                              scheduler_args->queue_generator_lock);
        }

        // Get the next process
        pcb_t item = queue_dequeue(queue);
        if (item.is_dummy != 0) {
            queue_enqueue(queue, item);
            printf("%s%d%s\n", "Cpu ", scheduler_args->id_of_processor, " is exiting ");
            pthread_cond_signal(scheduler_args->queue_generator_cond);
            pthread_mutex_unlock(scheduler_args->queue_generator_lock);
            break;
        }

        item.id_of_processor = scheduler_args->id_of_processor;
        // printf("PICKING UP PCB\n");
        // print_pcb(&item);

        if (scheduler_args->outmode == '3') {
            printf("%s%d%s%d%s%lld\n", "Process ", item.pid, " is picked up by processor ",
                   scheduler_args->id_of_processor, " at ", gettimeofday_ms() - start_time);
        }

        pthread_mutex_unlock(scheduler_args->queue_generator_lock);

        print_for_outmode(&item, gettimeofday_ms() - start_time, scheduler_args->outmode, OUTMODE_3_SETTINGS_NONE);
        usleep(item.burst_length * 1000);

        long long current_time = gettimeofday_ms() - start_time;
        item.remaining_time = 0;
        item.finish_time = current_time;
        item.turnaround_time = item.finish_time - item.arrival_time;
        item.waiting_time = item.turnaround_time - item.burst_length;

        if (scheduler_args->outmode == '3') {
            printf("%s%d%s%d%s%lld\n", "Process ", item.pid, " is finished by processor ",
                   scheduler_args->id_of_processor, " at ", gettimeofday_ms() - start_time);
        }

        // printf("FINISHED PCB\n");
        // print_pcb(&item);

        pthread_mutex_lock(scheduler_args->history_queue_lock);
        queue_enqueue(history_queue, item);
        pthread_mutex_unlock(scheduler_args->history_queue_lock);
        count++;
        printf("%s%d%s%d\n", "Cpu ", scheduler_args->id_of_processor, " finished iteration ",
               count);
    }

    printf("%s%d%s\n", "Cpu ", scheduler_args->id_of_processor, " is exiting due flag");
    return NULL;
}

void* sjf(void* args) {
    scheduler_args_t* scheduler_args = (scheduler_args_t*)args;

    // Get the queue
    queue_t* queue = scheduler_args->source_queue;
    // Get the history queue
    queue_t* history_queue = scheduler_args->history_queue;

    while (1) {
        // Lock the queue
        pthread_mutex_lock(scheduler_args->queue_generator_lock);

        while (!(queue->size > 0)) {
            pthread_cond_wait(scheduler_args->queue_generator_cond,
                              scheduler_args->queue_generator_lock);
        }

        // Get the next process
        pcb_t item = queue_dequeue(queue);
        if (item.is_dummy != 0) {
            queue_enqueue(queue, item);
            printf("%s%d%s\n", "Cpu ", scheduler_args->id_of_processor, " is exiting ");
            pthread_cond_signal(scheduler_args->queue_generator_cond);
            pthread_mutex_unlock(scheduler_args->queue_generator_lock);

            break;
        }

        item.id_of_processor = scheduler_args->id_of_processor;
        // printf("PICKING UP PCB\n");
        // print_pcb(&item);

        if (scheduler_args->outmode == '3') {
            printf("%s%d%s%d%s%lld\n", "Process ", item.pid, " is picked up by processor ",
                   scheduler_args->id_of_processor, " at ", gettimeofday_ms() - start_time);
        }

        pthread_mutex_unlock(scheduler_args->queue_generator_lock);

        print_for_outmode(&item, gettimeofday_ms() - start_time, scheduler_args->outmode, OUTMODE_3_SETTINGS_NONE);
        usleep(item.burst_length * 1000);

        long long current_time = gettimeofday_ms() - start_time;
        item.remaining_time = 0;
        item.finish_time = current_time;
        item.turnaround_time = item.finish_time - item.arrival_time;
        item.waiting_time = item.turnaround_time - item.burst_length;

        // printf("FINISHED PCB\n");
        // print_pcb(&item);

        if (scheduler_args->outmode == '3') {
            printf("%s%d%s%d%s%lld\n", "Process ", item.pid, " is finished by processor ",
                   scheduler_args->id_of_processor, " at ", gettimeofday_ms() - start_time);
        }

        pthread_mutex_lock(scheduler_args->history_queue_lock);
        queue_enqueue(history_queue, item);
        pthread_mutex_unlock(scheduler_args->history_queue_lock);
    }

    return NULL;
}

void* rr(void* args) {
    scheduler_args_t* scheduler_args = (scheduler_args_t*)args;

    // Get the queue
    queue_t* queue = scheduler_args->source_queue;
    // Get the history queue
    queue_t* history_queue = scheduler_args->history_queue;

    while (1) {
        // Lock the queue
        pthread_mutex_lock(scheduler_args->queue_generator_lock);

        while (!(queue->size > 0)) {
            printf("%s%d%s\n", "Cpu ", scheduler_args->id_of_processor, " is waiting ");
            pthread_cond_wait(scheduler_args->queue_generator_cond,
                              scheduler_args->queue_generator_lock);
        }

        // Get the next process
        pcb_t item = queue_dequeue(queue);
        if (queue->size < 1 && item.is_dummy != 0) {
            queue_enqueue(queue, item);
            printf("%s%d%s\n", "Cpu ", scheduler_args->id_of_processor, " is exiting ");
            pthread_cond_signal(scheduler_args->queue_generator_cond);
            pthread_mutex_unlock(scheduler_args->queue_generator_lock);
            break;
        } else if (item.is_dummy != 0) {
            pcb_t temp_item = item;
            item = queue_dequeue(queue);
            queue_enqueue(queue, temp_item);
        }

        item.id_of_processor = scheduler_args->id_of_processor;
        // printf("PICKING UP PCB\n");
        // print_pcb(&item);

        if (scheduler_args->outmode == '3') {
            printf("%s%d%s%d%s%lld\n", "Process ", item.pid, " is picked up by processor ",
                   scheduler_args->id_of_processor, " at ", gettimeofday_ms() - start_time);
        }

        pthread_mutex_unlock(scheduler_args->queue_generator_lock);
        print_for_outmode(&item, gettimeofday_ms() - start_time, scheduler_args->outmode, OUTMODE_3_SETTINGS_NONE);

        if (item.remaining_time > scheduler_args->time_quantum) {
            usleep(scheduler_args->time_quantum * 1000);
            item.remaining_time -= scheduler_args->time_quantum;
            pthread_mutex_lock(scheduler_args->queue_generator_lock);
            queue_enqueue(queue, item);
            pthread_mutex_unlock(scheduler_args->queue_generator_lock);
        } else {
            usleep(item.remaining_time * 1000);
            long long current_time = gettimeofday_ms() - start_time;
            item.remaining_time = 0;
            item.finish_time = current_time;
            item.turnaround_time = item.finish_time - item.arrival_time;
            item.waiting_time = item.turnaround_time - item.burst_length;

            if (scheduler_args->outmode == '3') {
                printf("%s%d%s%d%s%lld\n", "Process ", item.pid, " is finished by processor ",
                       scheduler_args->id_of_processor, " at ", gettimeofday_ms() - start_time);
            }

            // printf("FINISHED PCB\n");
            // print_pcb(&item);

            pthread_mutex_lock(scheduler_args->history_queue_lock);
            queue_enqueue(history_queue, item);
            pthread_mutex_unlock(scheduler_args->history_queue_lock);
        }
    }

    return NULL;
}