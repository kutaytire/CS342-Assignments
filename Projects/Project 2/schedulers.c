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

    while (flag) {
        // Lock the queue
        pthread_mutex_lock(scheduler_args->queue_generator_lock);

        while (!(queue->size > 0)) {
            pthread_cond_wait(scheduler_args->queue_generator_cond,
                              scheduler_args->queue_generator_lock);
        }

        // Get the next process
        pcb_t item = queue_dequeue(queue);
        if (item.is_dummy != 0) {
            flag = 0;

            pthread_mutex_unlock(scheduler_args->queue_generator_lock);

            break;
        }

        item.id_of_processor = scheduler_args->id_of_processor;
        // printf("PICKING UP PCB\n");
        // print_pcb(&item);

        pthread_mutex_unlock(scheduler_args->queue_generator_lock);

        print_for_outmode(&item, gettimeofday_ms() - start_time, scheduler_args->outmode);
        usleep(item.burst_length * 1000);

        long long current_time = gettimeofday_ms() - start_time;
        item.remaining_time = 0;
        item.finish_time = current_time;
        item.turnaround_time = item.finish_time - item.arrival_time;
        item.waiting_time = item.turnaround_time - item.burst_length;

        // printf("FINISHED PCB\n");
        // print_pcb(&item);

        pthread_mutex_lock(scheduler_args->history_queue_lock);
        queue_enqueue(history_queue, item);
        pthread_mutex_unlock(scheduler_args->history_queue_lock);
    }

    return NULL;
}

void* sjf(void* args) {
    scheduler_args_t* scheduler_args = (scheduler_args_t*)args;

    // Get the queue
    queue_t* queue = scheduler_args->source_queue;
    // Get the history queue
    queue_t* history_queue = scheduler_args->history_queue;

    while (flag) {
        // Lock the queue
        pthread_mutex_lock(scheduler_args->queue_generator_lock);

        while (!(queue->size > 0)) {
            pthread_cond_wait(scheduler_args->queue_generator_cond,
                              scheduler_args->queue_generator_lock);
        }

        // Get the next process
        pcb_t item = queue_dequeue(queue);
        if (item.is_dummy != 0) {
            flag = 0;

            pthread_mutex_unlock(scheduler_args->queue_generator_lock);

            break;
        }

        item.id_of_processor = scheduler_args->id_of_processor;
        // printf("PICKING UP PCB\n");
        // print_pcb(&item);

        pthread_mutex_unlock(scheduler_args->queue_generator_lock);

        print_for_outmode(&item, gettimeofday_ms() - start_time, scheduler_args->outmode);
        usleep(item.burst_length * 1000);

        long long current_time = gettimeofday_ms() - start_time;
        item.remaining_time = 0;
        item.finish_time = current_time;
        item.turnaround_time = item.finish_time - item.arrival_time;
        item.waiting_time = item.turnaround_time - item.burst_length;

        // printf("FINISHED PCB\n");
        // print_pcb(&item);

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

    while (flag) {
        // Lock the queue
        pthread_mutex_lock(scheduler_args->queue_generator_lock);

        while (!(queue->size > 0)) {
            pthread_cond_wait(scheduler_args->queue_generator_cond,
                              scheduler_args->queue_generator_lock);
        }

        // Get the next process
        pcb_t item = queue_dequeue(queue);
        if (queue->size <= 1 && item.is_dummy != 0) {
            flag = 0;

            pthread_mutex_unlock(scheduler_args->queue_generator_lock);

            break;
        } else if (item.is_dummy != 0) {
            queue_enqueue(queue, item);

            pthread_mutex_unlock(scheduler_args->queue_generator_lock);

            continue;
        }

        item.id_of_processor = scheduler_args->id_of_processor;
        // printf("PICKING UP PCB\n");
        // print_pcb(&item);

        pthread_mutex_unlock(scheduler_args->queue_generator_lock);
        print_for_outmode(&item, gettimeofday_ms() - start_time, scheduler_args->outmode);

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

            // printf("FINISHED PCB\n");
            // print_pcb(&item);

            pthread_mutex_lock(scheduler_args->history_queue_lock);
            queue_enqueue(history_queue, item);
            pthread_mutex_unlock(scheduler_args->history_queue_lock);
        }
    }

    return NULL;
}
