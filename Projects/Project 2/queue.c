
#include "queue.h"
#include <stdlib.h>

queue_t* queue_create() {
    queue_node_t* head = (queue_node_t*)malloc(sizeof(queue_node_t));
    head->next = NULL;
    queue_t* queue = (queue_t*)malloc(sizeof(queue_t));
    queue->head = head;
    queue->size = 0;
    return queue;
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

queue_item_t queue_dequeue(queue_t* q) {
    queue_node_t* head = q->head;
    queue_node_t* next = head->next;
    queue_item_t item = head->item;

    q->head = next;

    head->next = NULL;
    free(head);

    q->size--;

    return item;
}
