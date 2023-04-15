// Queue ADT

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "pcb.h"

typedef pcb_t queue_item_t;

typedef struct queue_node {
    queue_item_t item;
    struct queue_node* next;
} queue_node_t;

typedef struct {
    queue_node_t* head;
    int size;
} queue_t;

queue_t* queue_create();
void queue_destroy(queue_t* queue);
void queue_enqueue(queue_t* queue, queue_item_t item);
void queue_sorted_enqueue(queue_t* queue, queue_item_t item);
queue_item_t queue_dequeue(queue_t* queue);
void queue_sort(queue_t* queue);
void print_queue(queue_t* queue);

#endif // __QUEUE_H__
