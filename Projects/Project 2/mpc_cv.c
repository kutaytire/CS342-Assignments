#include "common_defs.h"
#include "pcb.h"
#include "queue.h"
#include "schedulers.h"
#include "util.h"
#include <pthread.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Function Definitions
void update_queue_s(char* tasks_source);
void update_queue_m(char* tasks_source);

// Variables
int number_of_processors = DEFAULT_N;
char scheduling_approach = DEFAULT_SAP;
char* queue_selection_method = DEFAULT_QS;
char* algorithm = DEFAULT_ALG;
int time_quantum = DEFAULT_Q;
char* input_file = DEFAULT_INFILE;
char outmode = DEFAULT_OUTMODE;
char* outfile = DEFAULT_OUTFILE;
int t = DEFAULT_T;
int t1 = DEFAULT_T1;
int t2 = DEFAULT_T2;
int l = DEFAULT_L;
int l1 = DEFAULT_L1;
int l2 = DEFAULT_L2;

pcb_t dummy_pcb = {.pid = -1,
                   .burst_length = -1,
                   .arrival_time = -1,
                   .remaining_time = -1,
                   .finish_time = -1,
                   .turnaround_time = -1,
                   .waiting_time = -1,
                   .id_of_processor = -1,
                   .is_dummy = 1};

long long start_time;

// Single queue related variables
queue_t* queue;
queue_t* history_queue;
pthread_mutex_t queue_generator_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_generator_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t history_queue_lock = PTHREAD_MUTEX_INITIALIZER;

// Multiple queue related variables
queue_t** processor_queues;
pthread_mutex_t* processor_queue_locks;
pthread_cond_t* processor_queue_conds;

// Main Program
int main(int argc, char* argv[]) {
    // Parse the arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            if (argv[i + 1] != NULL && atoi(argv[i + 1]) > 0) {
                number_of_processors = atoi(argv[i + 1]);
                printf("Number of processors: %d\n", number_of_processors);
            } else {
                printf("Invalid or not specified number of processors: %d, falling back to "
                       "default: %d\n",
                       argv[i + 1] ? atoi(argv[i + 1]) : -1, DEFAULT_N);
            }
        }

        else if (strcmp(argv[i], "-a") == 0) {
            if (argv[i + 1] == NULL) {
                goto a_end;
            } else {
                scheduling_approach = argv[i + 1][0];
            }

            if (scheduling_approach == 'S') {
                printf("Scheduling approach: Single Queue\n");

                if (argv[i + 2] == NULL || strcmp(argv[i + 2], "NA") != 0) {
                    queue_selection_method = malloc(sizeof(char) * 3);
                    strcpy(queue_selection_method, "NA");
                    printf("Queue selection method: %s\n", queue_selection_method);
                } else if (strcmp(argv[i + 2], "NA") == 0) {
                    queue_selection_method = malloc(sizeof(char) * 3);
                    strcpy(queue_selection_method, argv[i + 2]);
                    printf("Queue selection method: %s\n", queue_selection_method);
                } else {
                    printf("Queue selection method other than 'NA' (not applicable) is invalid for "
                           "single queue scheduling approach, falling back to default: %s\n",
                           "NA");
                    queue_selection_method = malloc(sizeof(char) * 3);
                    strcpy(queue_selection_method, "NA");
                }

            } else if (scheduling_approach == 'M') {
                printf("Scheduling approach: Multiple Queues\n");
                if (argv[i + 2] != NULL &&
                    (strcmp(argv[i + 2], "RM") == 0 || strcmp(argv[i + 2], "LM") == 0)) {
                    queue_selection_method = malloc(sizeof(char) * 3);
                    strcpy(queue_selection_method, argv[i + 2]);
                    printf("Queue selection method: %s\n", queue_selection_method);
                } else {
                    printf("Invalid queue selection method: %s, falling back to default: %s\n",
                           argv[i + 2] ? argv[i + 2] : "(not specified)", DEFAULT_QS);
                }
            } else {
            a_end:
                printf("Invalid or not specified scheduling approach: %c, falling back to default: "
                       "%c\n",
                       argv[i + 1] ? scheduling_approach : '-', DEFAULT_SAP);
            }
        }

        else if (strcmp(argv[i], "-s") == 0) {
            if (argv[i + 1] != NULL &&
                (strcmp(argv[i + 1], "FCFS") == 0 || strcmp(argv[i + 1], "SJF") == 0 ||
                 strcmp(argv[i + 1], "RR") == 0)) {
                algorithm = malloc(sizeof(char) * 5);
                strcpy(algorithm, argv[i + 1]);
                printf("Scheduling algorithm: %s\n", algorithm);
            } else {
                printf("Invalid scheduling algorithm: %s, falling back to default: %s\n",
                       argv[i + 1] ? argv[i + 1] : "(not specified)", DEFAULT_ALG);
            }

            if (strcmp(algorithm, "RR") == 0) {
                if (argv[i + 2] != NULL && atoi(argv[i + 2]) > 0) {
                    time_quantum = atoi(argv[i + 2]);
                    printf("Time quantum: %d\n", time_quantum);
                } else {
                    printf(
                        "Invalid or not specified time quantum: %d, falling back to default: %d\n",
                        argv[i + 2] ? atoi(argv[i + 2]) : -1, DEFAULT_Q);
                }
            }

            else if ((strcmp(algorithm, "SJF") == 0 || strcmp(algorithm, "FCFS") == 0) &&
                     argv[i + 2] != NULL && strcmp(argv[i + 2], "0") != 0) {
                printf("Time quantum other than 0 is invalid for %s scheduling algorithm, falling "
                       "back to: %s\n",
                       algorithm, "0");
            }
        }

        else if (strcmp(argv[i], "-i") == 0) {
            if (argv[i + 1] != NULL) {
                input_file = malloc(sizeof(char) * strlen(argv[i + 1]) + 1);
                strcpy(input_file, argv[i + 1]);
                printf("Input file: %s\n", input_file);
            } else {
                printf("Invalid or not specified input file: %s, falling back to default: %s\n",
                       argv[i + 1] ? argv[i + 1] : "(not specified)", DEFAULT_INFILE);
            }
        }

        else if (strcmp(argv[i], "-m") == 0) {
            if (argv[i + 1] != NULL) {
                char mode = argv[i + 1][0];

                if (mode == '1' || mode == '2' || mode == '3') {
                    printf("Outmode: %c\n", mode);
                    outmode = mode;
                } else {
                    printf("Invalid outmode: %c, falling back to default outmode: %c\n", mode,
                           outmode);
                }

            } else {
                printf("Outmode not specified, falling back to default outmode: %c\n", outmode);
            }
        }

        // } else if (strcmp(argv[i], "-o") == 0) {
        //     i++;
        //     strcpy(outfile, argv[i]);

        // } else if (strcmp(argv[i], "-r") == 0) {
        //     i++;
        //     t = atoi(argv[i]);
        //     i++;
        //     t1 = atoi(argv[i]);
        //     i++;
        //     t2 = atoi(argv[i]);
        //     i++;
        //     l = atoi(argv[i]);
        //     i++;
        //     l1 = atoi(argv[i]);
        //     i++;
        //     l2 = atoi(argv[i]);
        // }
    }

    // Common queue for single-queue scheduling algorithm
    queue = queue_create();
    history_queue = queue_create();
    pthread_t threads[number_of_processors];

    // Create n number of queues for multiple-queue scheduling algorithm
    processor_queues = malloc(sizeof(queue_t*) * number_of_processors);
    processor_queue_locks = malloc(sizeof(pthread_mutex_t) * number_of_processors);
    processor_queue_conds = malloc(sizeof(pthread_cond_t) * number_of_processors);

    processor_queues = malloc(sizeof(queue_t*) * number_of_processors);
    for (int i = 0; i < number_of_processors; i++) {
        processor_queues[i] = queue_create();
        pthread_mutex_init(&processor_queue_locks[i], NULL);
        pthread_cond_init(&processor_queue_conds[i], NULL);
    }

    // Create n number of threads to simulate processors
    for (int i = 0; i < number_of_processors; i++) {
        if (strcmp(algorithm, "FCFS") == 0) {
            scheduler_args_t* args = malloc(sizeof(scheduler_args_t));
            args->source_queue = (scheduling_approach == 'S') ? queue : processor_queues[i];
            args->time_quantum = -1;
            args->history_queue = history_queue;
            args->id_of_processor = i + 1;
            args->outmode = outmode;
            args->queue_generator_cond =
                (scheduling_approach == 'S') ? &queue_generator_cond : &processor_queue_conds[i];
            args->queue_generator_lock =
                (scheduling_approach == 'S') ? &queue_generator_lock : &processor_queue_locks[i];
            args->history_queue_lock = &history_queue_lock;
            // scheduler_args_t args = {.source_queue = queue, .time_quantum = -1, .history_queue =
            // history_queue, .id_of_processor = i};

            if (pthread_create(&threads[i], NULL, fcfs, (void*)args) != 0) {
                printf("Error: Thread creation failed.\n");
                exit(-1);
            }
        }

        else if (strcmp(algorithm, "SJF") == 0) {
            scheduler_args_t* args = malloc(sizeof(scheduler_args_t));
            args->source_queue = (scheduling_approach == 'S') ? queue : processor_queues[i];
            args->time_quantum = -1;
            args->history_queue = history_queue;
            args->id_of_processor = i + 1;
            args->outmode = outmode;
            args->queue_generator_cond =
                (scheduling_approach == 'S') ? &queue_generator_cond : &processor_queue_conds[i];
            args->queue_generator_lock =
                (scheduling_approach == 'S') ? &queue_generator_lock : &processor_queue_locks[i];
            args->history_queue_lock = &history_queue_lock;

            if (pthread_create(&threads[i], NULL, sjf, (void*)args) != 0) {
                printf("Error: Thread creation failed.\n");
                exit(-1);
            }
        }

        else if (strcmp(algorithm, "RR") == 0) {
            scheduler_args_t* args = malloc(sizeof(scheduler_args_t));
            args->source_queue = (scheduling_approach == 'S') ? queue : processor_queues[i];
            args->time_quantum = time_quantum;
            args->history_queue = history_queue;
            args->id_of_processor = i + 1;
            args->outmode = outmode;
            args->queue_generator_cond =
                (scheduling_approach == 'S') ? &queue_generator_cond : &processor_queue_conds[i];
            args->queue_generator_lock =
                (scheduling_approach == 'S') ? &queue_generator_lock : &processor_queue_locks[i];
            args->history_queue_lock = &history_queue_lock;

            if (pthread_create(&threads[i], NULL, rr, (void*)args) != 0) {
                printf("Error: Thread creation failed.\n");
                return -1;
            }
        }
    }

    // Update the queue in the main thread by reading from the input file
    if (scheduling_approach == 'S') {
        update_queue_s(input_file);
    } else if (scheduling_approach == 'M') {
        update_queue_m(input_file);
    }

    // Wait for all the threads to finish
    for (int i = 0; i < number_of_processors; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print the history queue
    pthread_mutex_lock(&history_queue_lock);

    print_history_queue(history_queue);

    pthread_mutex_unlock(&history_queue_lock);

    // TODO: Free memory allocated for dynamically allocated strings

    return 0;
}

void update_queue_s(char* tasks_source) {
    // Open the file
    FILE* fp = fopen(tasks_source, "r");

    if (fp == NULL) {
        printf("Failed to open the input file: %s\n", tasks_source);
        exit(1);
    }

    char* line = NULL;
    size_t len = 0;

    regex_t regex; // To check if the line is in the correct format

    int current_iat = 0;
    int last_pid = 1;
    start_time = gettimeofday_ms();

    while (getline(&line, &len, fp) != -1) {
        // find new line
        char* ptr = strchr(line, '\n');
        // if new line found replace with null character
        if (ptr) {
            *ptr = '\0';
        }

        // printf("line: %s\n", (line));

        // Each line should follow the format:
        // PL <int> or IAT <int>
        // If the line is not in the correct format, exit the program
        regcomp(&regex, "^(PL|IAT) [0-9]+$", REG_EXTENDED);
        if (regexec(&regex, line, 0, NULL, 0) != 0) {
            printf(
                "Invalid line encountered in the input file: %s => Line: %s\nExiting the program!",
                tasks_source, line);
            exit(-1);
        }

        else {
            if (strncmp(line, "PL", 2) == 0) {
                if (atoi(line + 3) < 0) {
                    printf("Invalid burst length in the input file: %s => Line: %s\nExiting the "
                           "program!",
                           tasks_source, line);
                }

                // Create a new PCB
                pcb_t pcb = {.pid = last_pid,
                             .burst_length = atoi(line + 3),
                             .arrival_time = -1,
                             .remaining_time = atoi(line + 3),
                             .finish_time = -1,
                             .turnaround_time = -1,
                             .waiting_time = -1,
                             .id_of_processor = -1,
                             .is_dummy = 0};

                pthread_mutex_lock(&queue_generator_lock);

                // printf("Enqueueing a new PCB\n");
                // print_pcb(&pcb);
                pcb.arrival_time = gettimeofday_ms() - start_time;
                if (strcmp(algorithm, "SJF") == 0) {
                    queue_sorted_enqueue(queue, pcb);
                } else {
                    queue_enqueue(queue, pcb);
                }

                pthread_mutex_unlock(&queue_generator_lock);

                pthread_cond_signal(&queue_generator_cond);

                last_pid++;

            } else if (strncmp(line, "IAT", 3) == 0) {
                int iat = atoi(line + 4);

                if (iat < 0) {
                    printf("Invalid IAT in the input file: %s => Line: %s\nExiting the "
                           "program!",
                           tasks_source, line);
                    exit(-1);
                }

                current_iat += iat;

                pthread_mutex_unlock(&queue_generator_lock);

                usleep(iat * 1000);

            } else {
                printf("Invalid line encountered in the input file: %s\nLine: %s\n", tasks_source,
                       line);
                exit(-1);
            }
        }
    }

    pthread_mutex_lock(&queue_generator_lock);

    // Add a dummy PCB to the queue to indicate the end of the file
    // printf("Enqueueing a new PCB\n");
    // print_pcb(&dummy_pcb);
    queue_enqueue(queue, dummy_pcb);

    pthread_mutex_unlock(&queue_generator_lock);

    pthread_cond_signal(&queue_generator_cond);

    fclose(fp);
    free(line);
}

void update_queue_m(char* tasks_source) { 

    // Open the file
    FILE* fp = fopen(tasks_source, "r");

    if (fp == NULL) {
        printf("Failed to open the input file: %s\n", tasks_source);
        exit(1);
    }

    char* line = NULL;
    size_t len = 0;

    regex_t regex; // To check if the line is in the correct format

    int current_iat = 0;
    int last_pid = 1;
    start_time = gettimeofday_ms();

    while (getline(&line, &len, fp) != -1) {
        // find new line
        char* ptr = strchr(line, '\n');
        // if new line found replace with null character
        if (ptr) {
            *ptr = '\0';
        }

        // printf("line: %s\n", (line));

        // Each line should follow the format:
        // PL <int> or IAT <int>
        // If the line is not in the correct format, exit the program
        regcomp(&regex, "^(PL|IAT) [0-9]+$", REG_EXTENDED);
        if (regexec(&regex, line, 0, NULL, 0) != 0) {
            printf(
                "Invalid line encountered in the input file: %s => Line: %s\nExiting the program!",
                tasks_source, line);
            exit(-1);
        }

        else {
            if (strncmp(line, "PL", 2) == 0) {
                if (atoi(line + 3) < 0) {
                    printf("Invalid burst length in the input file: %s => Line: %s\nExiting the "
                           "program!",
                           tasks_source, line);
                }

                // Create a new PCB
                pcb_t pcb = {.pid = last_pid,
                             .burst_length = atoi(line + 3),
                             .arrival_time = -1,
                             .remaining_time = atoi(line + 3),
                             .finish_time = -1,
                             .turnaround_time = -1,
                             .waiting_time = -1,
                             .id_of_processor = -1,
                             .is_dummy = 0};

                if (strcpy(queue_selection_method, "RM") == 0) {

                    int queue_id = last_pid % number_of_processors;
                    pthread_mutex_lock(&processor_queue_locks[queue_id]);

                    pcb.arrival_time = gettimeofday_ms() - start_time;
                    if (strcmp(algorithm, "SJF") == 0) {

                        queue_sorted_enqueue(processor_queues[queue_id], pcb);
                    } 
                    else {

                        queue_enqueue(processor_queues[queue_id], pcb);
                    }

                    pthread_mutex_unlock(&processor_queue_locks[queue_id]);
                    pthread_cond_signal(&processor_queue_conds[queue_id]);

                }
                else {
                    
                    pthread_mutex_lock(&processor_queue_locks[0]);
                    int min_load = get_queue_load(processor_queues[0]);
                    int id_of_min = 0;
                    pthread_mutex_unlock(&processor_queue_locks[0]);

                    for (int i = 1; i < number_of_processors; i++) {

                        pthread_mutex_lock(&processor_queue_locks[i]);
                        int load = get_queue_load(processor_queues[i]);

                        if(load < min_load) {

                            min_load = load;
                            id_of_min = i;
                        }

                        else if (load == min_load) {

                            id_of_min = (i < id_of_min) ? i:id_of_min;
                        }

                        pthread_mutex_unlock(&processor_queue_locks[i]);
                    }

                    pthread_mutex_lock(&processor_queue_locks[id_of_min]);

                    pcb.arrival_time = gettimeofday_ms() - start_time;
                    if (strcmp(algorithm, "SJF") == 0) {

                        queue_sorted_enqueue(processor_queues[id_of_min], pcb);
                    } 
                    else {

                        queue_enqueue(processor_queues[id_of_min], pcb);
                    }

                    pthread_mutex_unlock(&processor_queue_locks[id_of_min]);
                    pthread_cond_signal(&processor_queue_conds[id_of_min]);

                }
                last_pid++;
            }
            else if (strncmp(line, "IAT", 3) == 0) {
                int iat = atoi(line + 4);

                if (iat < 0) {
                    printf("Invalid IAT in the input file: %s => Line: %s\nExiting the "
                           "program!",
                           tasks_source, line);
                    exit(-1);
                }

                current_iat += iat;

                usleep(iat * 1000);

            } 
            else {
                printf("Invalid line encountered in the input file: %s\nLine: %s\n", tasks_source,
                       line);
                exit(-1);
            }
        }
    }

    // Add a dummy item to each queue

    for (int i = 0; i < number_of_processors; i++) {

        pthread_mutex_lock(&processor_queue_locks[i]);
        queue_enqueue(processor_queues[i], dummy_pcb);

        pthread_mutex_unlock(&processor_queue_locks[i]);
        pthread_cond_signal(&processor_queue_conds[i]);
    }
    fclose(fp);
    free(line);
}