#include "util.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

void print_for_outmode(pcb_t* pcb, long long time, char outmode, enum outmode_3_settings settings,
                       int where, FILE* fp) {
    if (outmode == '1') {
        return;
    }

    else if (outmode == '2') {
        if (pcb != NULL) {
            if (pcb->is_dummy != 1 && pcb->id_of_processor != -1 && settings == OUTMODE_3_SETTINGS_PCB_PICKED_FROM_READY_QUEUE) {
                if (fp != NULL) {
                    fprintf(fp, "time=%lld, cpu=%d, pid=%d, burstlen=%d, remainingtime=%d\n", time,
                            pcb->id_of_processor, pcb->pid, pcb->burst_length, pcb->remaining_time);
                } else {
                    printf("time=%lld, cpu=%d, pid=%d, burstlen=%d, remainingtime=%d\n", time,
                           pcb->id_of_processor, pcb->pid, pcb->burst_length, pcb->remaining_time);
                }
            } else if (pcb->id_of_processor != -1 && settings == OUTMODE_3_SETTINGS_PCB_PICKED_FROM_READY_QUEUE) {
                if (fp != NULL) {
                    fprintf(
                        fp,
                        "time=%lld, cpu=%d, pid=dummy process, burstlen=NaN, remainingtime=NaN\n",
                        time, pcb->id_of_processor);
                } else {
                    printf(
                        "time=%lld, cpu=%d, pid=dummy process, burstlen=NaN, remainingtime=NaN\n",
                        time, pcb->id_of_processor);
                }
            }
        }
    }

    else if (outmode == '3') {
        if (settings == OUTMODE_3_SETTINGS_PCB_ADDED_TO_READY_QUEUE) {
            if (where == -999) {
                if (fp != NULL) {
                    fprintf(fp, "[CPU: main (received process input)] %s (process details): ",
                            "A burst is to be added to a (ready) queue");
                } else {
                    printf("[CPU: main (received process input)] %s (process details): ",
                           "A burst is to be added to a (ready) queue");
                }
            } else {
                if (fp != NULL) {
                    fprintf(fp, "[CPU: %d] %s (process details): ", where,
                            "A burst is to be added to a (ready) queue");
                } else {
                    printf("[CPU: %d] %s (process details): ", where,
                           "A burst is to be added to a (ready) queue");
                }
            }
            if (pcb != NULL) {
                if (where == -999) {
                    if (pcb->is_dummy != 1) {
                        if (fp != NULL) {
                            fprintf(fp,
                                    "time=%lld, cpu=main, pid=%d, burstlen=%d, remainingtime=%d\n",
                                    time, pcb->pid, pcb->burst_length, pcb->remaining_time);
                        } else {
                            printf("time=%lld, cpu=main, pid=%d, burstlen=%d, remainingtime=%d\n",
                                   time, pcb->pid, pcb->burst_length, pcb->remaining_time);
                        }
                    } else {
                        if (fp != NULL) {
                            fprintf(fp,
                                    "time=%lld, cpu=main, pid=dummy process, burstlen=NaN, "
                                    "remainingtime=NaN\n",
                                    time);
                        } else {
                            printf("time=%lld, cpu=main, pid=dummy process, burstlen=NaN, "
                                   "remainingtime=NaN\n",
                                   time);
                        }
                    }
                } else {
                    if (pcb->is_dummy != 1) {
                        if (fp != NULL) {
                            fprintf(fp,
                                    "time=%lld, cpu=%d, pid=%d, burstlen=%d, remainingtime=%d\n",
                                    time, where, pcb->pid, pcb->burst_length, pcb->remaining_time);
                        } else {
                            printf("time=%lld, cpu=%d, pid=%d, burstlen=%d, remainingtime=%d\n",
                                   time, where, pcb->pid, pcb->burst_length, pcb->remaining_time);
                        }
                    } else {
                        if (fp != NULL) {
                            fprintf(fp,
                                    "time=%lld, cpu=%d, pid=dummy process, burstlen=NaN, "
                                    "remainingtime=NaN\n",
                                    time, where);
                        } else {
                            printf("time=%lld, cpu=%d, pid=dummy process, burstlen=NaN, "
                                   "remainingtime=NaN\n",
                                   time, where);
                        }
                    }
                }
            }
        }

        else if (settings == OUTMODE_3_SETTINGS_CPU_EXITING) {
            if (where == -999) {
                if (fp != NULL) {
                    fprintf(fp, "[CPU: main] %s at time: %lld\n", "A CPU is exiting", time);
                } else {
                    printf("[CPU: main] %s at time: %lld\n", "A CPU is exiting", time);
                }

            } else {
                if (fp != NULL) {
                    fprintf(fp, "[CPU: %d] %s at time: %lld\n", where, "A CPU is exiting", time);
                } else {
                    printf("[CPU: %d] %s at time: %lld\n", where, "A CPU is exiting", time);
                }
            }
        }

        else if (settings == OUTMODE_3_SETTINGS_PCB_PICKED_FROM_READY_QUEUE) {
            if (fp == NULL) {
                printf("[CPU: %d] %s (process details): ", where, "A burst is picked for CPU");
            } else {
                fprintf(fp, "[CPU: %d] %s (process details): ", where, "A burst is picked for CPU");
            }

            if (pcb != NULL) {
                if (pcb->is_dummy != 1) {
                    if (fp != NULL) {
                        fprintf(fp, "time=%lld, cpu=%d, pid=%d, burstlen=%d, remainingtime=%d\n",
                                time, where, pcb->pid, pcb->burst_length, pcb->remaining_time);
                    } else {
                        printf("time=%lld, cpu=%d, pid=%d, burstlen=%d, remainingtime=%d\n", time,
                               where, pcb->pid, pcb->burst_length, pcb->remaining_time);
                    }
                } else {
                    if (fp != NULL) {
                        fprintf(fp,
                                "time=%lld, cpu=%d, pid=dummy process, burstlen=NaN, "
                                "remainingtime=NaN\n",
                                time, where);
                    } else {
                        printf("time=%lld, cpu=%d, pid=dummy process, burstlen=NaN, "
                               "remainingtime=NaN\n",
                               time, where);
                    }
                }
            }
        }

        else if (settings == OUTMODE_3_SETTINGS_PCB_FINISHED) {
            if (fp == NULL) {
                printf("[CPU: %d] %s (process details): ", where, "A burst has finished");

            } else {
                fprintf(fp, "[CPU: %d] %s (process details): ", where, "A burst has finished");
            }

            if (pcb != NULL) {
                if (pcb->is_dummy != 1) {
                    if (fp != NULL) {
                        fprintf(fp, "time=%lld, cpu=%d, pid=%d, burstlen=%d, remainingtime=%d\n",
                                time, where, pcb->pid, pcb->burst_length, pcb->remaining_time);
                    } else {
                        printf("time=%lld, cpu=%d, pid=%d, burstlen=%d, remainingtime=%d\n", time,
                               where, pcb->pid, pcb->burst_length, pcb->remaining_time);
                    }
                } else {
                    if (fp != NULL) {
                        fprintf(fp,
                                "time=%lld, cpu=%d, pid=dummy process, burstlen=NaN, "
                                "remainingtime=NaN\n",
                                time, where);
                    } else {
                        printf("time=%lld, cpu=%d, pid=dummy process, burstlen=NaN, "
                               "remainingtime=NaN\n",
                               time, where);
                    }
                }
            }
        }

        else if (settings == OUTMODE_3_SETTINGS_PCB_TIME_SLICE_EXPIRED) {
            if (fp != NULL) {
                fprintf(fp, "[CPU: %d] %s (process details): ", where,
                        "The time slice expired for a burst");
            } else {
                printf("[CPU: %d] %s (process details): ", where,
                       "The time slice expired for a burst");
            }

            if (pcb != NULL) {
                if (pcb->is_dummy != 1) {
                    if (fp != NULL) {
                        fprintf(fp, "time=%lld, cpu=%d, pid=%d, burstlen=%d, remainingtime=%d\n",
                                time, where, pcb->pid, pcb->burst_length, pcb->remaining_time);
                    } else {
                        printf("time=%lld, cpu=%d, pid=%d, burstlen=%d, remainingtime=%d\n", time,
                               where, pcb->pid, pcb->burst_length, pcb->remaining_time);
                    }
                } else {
                    if (fp != NULL) {
                        fprintf(fp,
                                "time=%lld, cpu=%d, pid=dummy process, burstlen=NaN, "
                                "remainingtime=NaN\n",
                                time, where);
                    } else {
                        printf("time=%lld, cpu=%d, pid=dummy process, burstlen=NaN, "
                               "remainingtime=NaN\n",
                               time, where);
                    }
                }
            }
        }

        else if (settings == OUTMODE_3_SETTINGS_PCB_ADDED_TO_READY_QUEUE_MULTI) {
            if (fp != NULL) {
                fprintf(fp, "[CPU: main] A burst is to be added to the (ready) queue of CPU %d: ",
                        where);
            } else {
                printf("[CPU: main] A burst is to be added to the (ready) queue of CPU %d: ",
                       where);
            }

            if (pcb != NULL) {
                if (pcb->is_dummy != 1) {
                    if (fp != NULL) {
                        fprintf(fp, "time=%lld, cpu=main, pid=%d, burstlen=%d, remainingtime=%d\n",
                                time, pcb->pid, pcb->burst_length, pcb->remaining_time);
                    } else {
                        printf("time=%lld, cpu=main, pid=%d, burstlen=%d, remainingtime=%d\n", time,
                               pcb->pid, pcb->burst_length, pcb->remaining_time);
                    }
                } else {
                    if (fp != NULL) {
                        fprintf(fp,
                                "time=%lld, cpu=main, pid=dummy process, burstlen=NaN, "
                                "remainingtime=NaN\n",
                                time);
                    } else {
                        printf("time=%lld, cpu=main, pid=dummy process, burstlen=NaN, "
                               "remainingtime=NaN\n",
                               time);
                    }
                }
            }
        }
    }
}

void print_history_queue(queue_t* queue, FILE* fp) {
    queue_sort(queue);

    if (queue->size == 0) {
        printf("No processes in history queue\n");
        return;
    }

    int total_turnaround = 0;

    if (fp == NULL) {
        printf("pid\tcpu\tburstlen\tarv\tfinish\twaitingtime\tturnaround\n");
    } else {
        fprintf(fp, "pid\tcpu\tburstlen\tarv\t\tfinish\twaitingtime\tturnaround\n");
    }
    queue_node_t* cur = queue->head;

    while (cur != NULL) {
        total_turnaround += cur->item.turnaround_time;

        if (fp == NULL) {
            printf("%d\t%d\t%d\t\t%lld\t%lld\t%lld\t\t%d\n", cur->item.pid,
                   cur->item.id_of_processor, cur->item.burst_length, cur->item.arrival_time,
                   cur->item.finish_time, cur->item.waiting_time, cur->item.turnaround_time);
        } else {
            fprintf(fp, "%d\t%d\t%d\t\t\t%lld\t\t%lld\t\t%lld\t\t\t%d\n", cur->item.pid,
                    cur->item.id_of_processor, cur->item.burst_length, cur->item.arrival_time,
                    cur->item.finish_time, cur->item.waiting_time, cur->item.turnaround_time);
        }

        cur = cur->next;
    }

    if (fp == NULL) {
        printf("average turnaround time: %f ms\n", total_turnaround / (double)queue->size);
    } else {
        fprintf(fp, "average turnaround time: %f ms\n", total_turnaround / (double)queue->size);
    }
}
