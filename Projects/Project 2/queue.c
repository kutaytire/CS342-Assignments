
#include "queue.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

queue_t* queue_create() {
    queue_t* q = (queue_t*)malloc(sizeof(queue_t));
    q->head = NULL;
    q->size = 0;
    return q;
}

void queue_destroy(queue_t* q) {
    queue_node_t* current = q->head;
    while (current != NULL) {
        queue_node_t* next = current->next;
        free(current);
        current = next;
    }
    free(q);
}

void queue_enqueue(queue_t* q, queue_item_t item) {
    if (q->head == NULL || q->size == 0) {
        queue_node_t* new_node = (queue_node_t*)malloc(sizeof(queue_node_t));
        new_node->item = item;
        new_node->next = NULL;
        q->head = new_node;
        q->size++;
    }

    else {
        queue_node_t* current = q->head;
        while (current->next != NULL) {
            current = current->next;
        }
        queue_node_t* new_node = (queue_node_t*)malloc(sizeof(queue_node_t));
        new_node->item = item;
        new_node->next = NULL;
        current->next = new_node;
        q->size++;
    }
}

void queue_sorted_enqueue(queue_t* q, queue_item_t item) {
    queue_node_t* new_node = (queue_node_t*)malloc(sizeof(queue_node_t));
    new_node->item = item;
    new_node->next = NULL;

    if (q->head == NULL || q->size == 0 || item.burst_length < q->head->item.burst_length) {
        new_node->next = q->head;
        q->head = new_node;
    } else {
        queue_node_t* current = q->head;
        while (current->next != NULL && current->next->item.burst_length <= item.burst_length) {
            if (current->next->item.burst_length == item.burst_length) {
                current = current->next;
            } else {
                break;
            }
        }
        new_node->next = current->next;
        current->next = new_node;
    }

    q->size++;
}

queue_item_t queue_dequeue(queue_t* q) {
    queue_node_t* head = q->head;
    queue_node_t* next = head->next;
    queue_item_t item = head->item;

    // printf("DEQUEUEING\n");
    // print_pcb(&item);

    q->head = next;

    head->next = NULL;
    free(head);

    q->size--;

    return item;
}

void queue_sort(queue_t* q) {
    if (q == NULL || q->size <= 1) {
        return;
    }
    queue_t* q1 = queue_create();
    queue_t* q2 = queue_create();
    int i, half_size = q->size / 2;
    for (i = 0; i < half_size; i++) {
        queue_enqueue(q1, queue_dequeue(q));
    }
    while (q->size > 0) {
        queue_enqueue(q2, queue_dequeue(q));
    }
    queue_sort(q1);
    queue_sort(q2);
    while (q1->size > 0 && q2->size > 0) {
        queue_item_t item1 = q1->head->item;
        queue_item_t item2 = q2->head->item;
        if (item1.pid <= item2.pid) {
            queue_enqueue(q, queue_dequeue(q1));
        } else {
            queue_enqueue(q, queue_dequeue(q2));
        }
    }
    while (q1->size > 0) {
        queue_enqueue(q, queue_dequeue(q1));
    }
    while (q2->size > 0) {
        queue_enqueue(q, queue_dequeue(q2));
    }
    queue_destroy(q1);
    queue_destroy(q2);
}

void print_queue(queue_t* q) {
    queue_node_t* current = q->head;
    while (current != NULL) {
        print_pcb(&current->item);
        current = current->next;
    }
}

int get_queue_load(queue_t* q) {

    int total_load = 0;
    queue_node_t* current = q->head;

     while (current != NULL) {
        
        total_load = total_load + current->item->remaining_time;
        current = current->next;
    }

    return total_load;

}
