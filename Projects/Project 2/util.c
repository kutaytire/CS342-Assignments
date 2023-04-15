#include "util.h"
#include "queue.h"
#include <stdio.h>
#include <sys/time.h>

long long gettimeofday_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

void print_pcb(pcb_t* pcb) {
    printf("PID: %d, Burst Length: %d, Arrival Time: %lld, Remaining Time: %d, Finish Time: "
           "%lld, Turnaround Time: %d, Waiting Time: %lld, ID of Processor: %d\n",
           pcb->pid, pcb->burst_length, pcb->arrival_time, pcb->remaining_time, pcb->finish_time,
           pcb->turnaround_time, pcb->waiting_time, pcb->id_of_processor);
}

void print_for_outmode(pcb_t* pcb, long long time, char outmode) {
    if (outmode == '2') {
        printf("time=%lld, cpu=%d, pid=%d, burstlen=%d, remainingtime=%d\n", time,
               pcb->id_of_processor, pcb->pid, pcb->burst_length, pcb->remaining_time);
    }
}

void print_history_queue(queue_t* queue) {
    queue_sort(queue);

    if (queue->size == 0) {
        printf("No processes in history queue\n");
        return;
    }

    int total_turnaround = 0;

    printf("pid\tcpu\tburstlen\tarv\tfinish\twaitingtime\tturnaround\n");
    queue_node_t* cur = queue->head;

    while (cur != NULL) {
        total_turnaround += cur->item.turnaround_time;
        printf("%d\t%d\t%d\t\t%lld\t%lld\t%lld\t\t%d\n", cur->item.pid, cur->item.id_of_processor,
               cur->item.burst_length, cur->item.arrival_time, cur->item.finish_time,
               cur->item.waiting_time, cur->item.turnaround_time);
        cur = cur->next;
    }
    printf("average turnaround time: %f ms\n", total_turnaround / (double)queue->size);
}
