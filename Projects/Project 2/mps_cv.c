#include "common_defs.h"
#include "pcb.h"
#include "queue.h"
#include "schedulers_cv.h"
#include "util.h"
#include <math.h>
#include <pthread.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Function Definitions
void update_queue_s(char* tasks_source);
void update_queue_m(char* tasks_source);
void update_queue_s_random();
void update_queue_m_random();

// Variables
int number_of_processors = DEFAULT_N;
char scheduling_approach = DEFAULT_SAP;
char* queue_selection_method = DEFAULT_QS;
int queue_selection_method_exists = 0;
char* algorithm = DEFAULT_ALG;
int algorithm_exists = 0;
int time_quantum = DEFAULT_Q;
char* input_file = DEFAULT_INFILE;
char outmode = DEFAULT_OUTMODE;
char* outfile = DEFAULT_OUTFILE;
FILE* outfp = NULL;
int t = DEFAULT_T;
int t1 = DEFAULT_T1;
int t2 = DEFAULT_T2;
int l = DEFAULT_L;
int l1 = DEFAULT_L1;
int l2 = DEFAULT_L2;
int pc = DEFAULT_PC;
int random_generate = DEFAULT_RANDOM_GENERATE;
int input_file_exists = DEFAULT_INPUT_FILE_EXISTS;

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
    srand(time(0));
    // Parse the arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            if (i + 1 >= argc || argv[i + 1] == NULL) {
                printf("Number of processors is not specified, falling back to default: %d\n",
                       DEFAULT_N);
            } else {
                if (!(atoi(argv[i + 1]) > 0 && atoi(argv[i + 1]) <= 10)) {
                    printf("Number of processors is not in range [1, 10], falling back to default: "
                           "%d\n",
                           DEFAULT_N);
                } else {
                    number_of_processors = atoi(argv[i + 1]);
                    printf("Number of processors: %d\n", number_of_processors);
                }
            }
        }

        else if (strcmp(argv[i], "-a") == 0) {
            if (i + 1 >= argc || argv[i + 1] == NULL) {
                printf("Scheduling approach is not specified, falling back to default: %c\n",
                       DEFAULT_SAP);
            } else {
                if (strcmp(argv[i + 1], "S") == 0 || strcmp(argv[i + 1], "M") == 0) {
                    scheduling_approach = argv[i + 1][0];
                    printf("Scheduling approach: %c\n", scheduling_approach);
                } else {
                    printf("Invalid scheduling approach: %s, falling back to default: %c\n",
                           argv[i + 1], DEFAULT_SAP);
                }
            }

            if (scheduling_approach == 'S') {
                if (i + 2 >= argc || argv[i + 2] == NULL) {
                    queue_selection_method = malloc(sizeof(char) * 3);
                    strcpy(queue_selection_method, "NA");
                    queue_selection_method_exists = 1;
                    printf("Queue selection method: %s\n", queue_selection_method);
                } else if (strcmp(argv[i + 2], "NA") == 0) {
                    queue_selection_method = malloc(sizeof(char) * 3);
                    strcpy(queue_selection_method, argv[i + 2]);
                    queue_selection_method_exists = 1;
                    printf("Queue selection method: %s\n", queue_selection_method);
                } else {
                    printf("Queue selection method other than 'NA' (not applicable) is invalid for "
                           "single queue scheduling approach, falling back to default: %s\n",
                           "NA");
                    queue_selection_method = malloc(sizeof(char) * 3);
                    queue_selection_method_exists = 1;
                    strcpy(queue_selection_method, "NA");
                }
            }

            else if (scheduling_approach == 'M') {
                if (i + 2 >= argc || argv[i + 2] == NULL) {
                    printf("Queue selection method is not specified, falling back to default: %s\n",
                           DEFAULT_QS);
                } else {
                    if (strcmp(argv[i + 2], "RM") == 0 || strcmp(argv[i + 2], "LM") == 0) {
                        queue_selection_method = malloc(sizeof(char) * 3);
                        strcpy(queue_selection_method, argv[i + 2]);
                        queue_selection_method_exists = 1;
                        printf("Queue selection method: %s\n", queue_selection_method);
                    } else {
                        printf("Invalid queue selection method: %s, falling back to default: %s\n",
                               argv[i + 2], DEFAULT_QS);
                    }
                }
            }

            else {
                printf("Invalid scheduling approach: %c, falling back to default: %c\n",
                       scheduling_approach, DEFAULT_SAP);
                scheduling_approach = DEFAULT_SAP;
            }
        }

        else if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 >= argc || argv[i + 1] == NULL) {
                printf("Scheduling algorithm not specified, falling back to default: %s\n",
                       DEFAULT_ALG);
            }

            else {
                if (strcmp(argv[i + 1], "FCFS") == 0 || strcmp(argv[i + 1], "SJF") == 0 ||
                    strcmp(argv[i + 1], "RR") == 0) {
                    algorithm = malloc(sizeof(char) * 5);
                    strcpy(algorithm, argv[i + 1]);
                    algorithm_exists = 1;
                    printf("Scheduling algorithm: %s\n", algorithm);
                } else {
                    printf("Invalid scheduling algorithm: %s, falling back to default: %s\n",
                           argv[i + 1], DEFAULT_ALG);
                }
            }

            if (strcmp(algorithm, "RR") == 0) {
                if (i + 2 >= argc || argv[i + 2] == NULL) {
                    printf("Time quantum is not specified, falling back to default: %d\n",
                           DEFAULT_Q);
                } else {
                    if (!(atoi(argv[i + 2]) >= 10 && atoi(argv[i + 2]) <= 100)) {
                        printf("Time quantum is not in range [10, 100], falling back to default: "
                               "%d\n",
                               DEFAULT_Q);
                    } else {
                        time_quantum = atoi(argv[i + 2]);
                        printf("Time quantum: %d\n", time_quantum);
                    }
                }
            }

            else if (strcmp(algorithm, "FCFS") == 0 || strcmp(algorithm, "SJF") == 0) {
                if (i + 2 < argc && argv[i + 2] != NULL && strcmp(argv[i + 2], "0") != 0) {
                    printf("Time quantum other than 0 is not applicable for %s, falling back to default: %d\n",
                           algorithm, DEFAULT_Q);
                }
            }
        }

        else if (strcmp(argv[i], "-i") == 0) {
            if (i + 1 >= argc || argv[i + 1] == NULL) {
                printf("Input file is not specified, since there is no default input file name determined, "
                       "program will exit. Please re-run the program with a valid input file name.\n");
                exit(-1);
            } else {
                input_file = malloc(sizeof(char) * (strlen(argv[i + 1]) + 1));
                strcpy(input_file, argv[i + 1]);
                printf("Input file: %s\n", input_file);
                input_file_exists = 1;
            }
        }

        else if (strcmp(argv[i], "-m") == 0) {
            if (i + 1 >= argc || argv[i + 1] == NULL) {
                printf("Outmode is not specified, falling back to default outmode: %c\n", outmode);
            } else {
                char mode = argv[i + 1][0];

                if (mode == '1' || mode == '2' || mode == '3') {
                    printf("Outmode: %c\n", mode);
                    outmode = mode;
                } else {
                    printf("Invalid outmode: %c, falling back to default outmode: %c\n", mode,
                           outmode);
                }
            }
        }

        else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc || argv[i + 1] == NULL) {
                printf("Output file is not specified, and there is no default for it, hence all "
                       "output will go to screen\n");
            } else {
                outfile = malloc(sizeof(char) * (strlen(argv[i + 1]) + 1));
                strcpy(outfile, argv[i + 1]);
                printf("Output file: %s\n", outfile);
            }
        }

        else if (strcmp(argv[i], "-r") == 0) {
            if (i + 1 >= argc || argv[i + 1] == NULL) {
                printf("T value is not specified, falling back to default value: %d\n", DEFAULT_T);
            }
            else {
                if (atoi(argv[i + 1]) <= 0) {
                    printf("Invalid T value: %d, falling back to default value: %d\n", atoi(argv[i + 1]),
                           DEFAULT_T);
                }
                else {
                    t = atoi(argv[i + 1]);
                }
            }

            if (i + 2 >= argc || argv[i + 2] == NULL) {
                printf("T1 value is not specified, falling back to default value: %d\n", DEFAULT_T1);
            }
            else {
                if (atoi(argv[i + 2]) < 10) {
                    printf("Invalid T1 value: %d, T1 should be bigger than or equal to minimum interarrival time 10 ms, falling back to default value: %d\n", atoi(argv[i + 2]),
                           DEFAULT_T1);
                }
                else {
                    t1 = atoi(argv[i + 2]);
                }
            }

            if (i + 3 >= argc || argv[i + 3] == NULL) {
                printf("T2 value is not specified, falling back to default value: %d\n", DEFAULT_T2);
            }
            else {
                if (atoi(argv[i + 3]) < t1) {
                    printf("Invalid T2 value: %d, T2 >= T1 should hold, falling back to default value: %d\n", atoi(argv[i + 3]),
                           DEFAULT_T2);
                }
                else {
                    t2 = atoi(argv[i + 3]);
                }
            }

            if (i + 4 >= argc || argv[i + 4] == NULL) {
                printf("L value is not specified, falling back to default value: %d\n", DEFAULT_L);
            }
            else {
                if (atoi(argv[i + 4]) <= 0) {
                    printf("Invalid L value: %d, falling back to default value: %d\n", atoi(argv[i + 4]),
                           DEFAULT_L);
                }
                else {
                    l = atoi(argv[i + 4]);
                }
            }

            if (i + 5 >= argc || argv[i + 5] == NULL) {
                printf("L1 value is not specified, falling back to default value: %d\n", DEFAULT_L1);
            }
            else {
                if (atoi(argv[i + 5]) < 10) {
                    printf("Invalid L1 value: %d, L1 should be bigger than or equal to minimum burst length 10 ms, falling back to default value: %d\n", atoi(argv[i + 5]),
                           DEFAULT_L1);
                }
                else {
                    l1 = atoi(argv[i + 5]);
                }
            }

            if (i + 6 >= argc || argv[i + 6] == NULL) {
                printf("L2 value is not specified, falling back to default value: %d\n", DEFAULT_L2);
            }
            else {
                if (atoi(argv[i + 6]) < l1) {
                    printf("Invalid L2 value: %d, L2 >= L1 should hold, falling back to default value: %d\n", atoi(argv[i + 6]),
                           DEFAULT_L2);
                }
                else {
                    l2 = atoi(argv[i + 6]);
                }
            }

            if (i + 7 >= argc || argv[i + 7] == NULL) {
                printf("PC value is not specified, falling back to default value: %d\n", DEFAULT_PC);
            }
            else {
                if (atoi(argv[i + 7]) < 0) {
                    printf("Invalid PC value: %d, falling back to default value: %d\n", atoi(argv[i + 7]),
                           DEFAULT_PC);
                }
                else {
                    pc = atoi(argv[i + 7]);
                }
            }

            random_generate = 1;
            printf("The value of t: %d, t1: %d, t2: %d, l: %d, l1: %d, l2: %d, pc: %d\n", t, t1, t2,
                   l, l1, l2, pc);
        }
    }

    if (scheduling_approach == 'S') {
        // Common queue for single-queue scheduling algorithm
        queue = queue_create();
    } else if (scheduling_approach == 'M') {
        // Create n number of queues for multiple-queue scheduling algorithm
        processor_queues = malloc(sizeof(queue_t*) * number_of_processors);
        processor_queue_locks = malloc(sizeof(pthread_mutex_t) * number_of_processors);
        processor_queue_conds = malloc(sizeof(pthread_cond_t) * number_of_processors);

        for (int i = 0; i < number_of_processors; i++) {
            processor_queues[i] = queue_create();
            pthread_mutex_init(&processor_queue_locks[i], NULL);
            pthread_cond_init(&processor_queue_conds[i], NULL);
        }
    }

    history_queue = queue_create();
    pthread_t threads[number_of_processors];
    scheduler_args_t args[number_of_processors];

    if (outfile != NULL) {
        outfp = fopen(outfile, "w");
        if (outfp == NULL) {
            printf("Error opening output file: %s\n", outfile);
            exit(-1);
        }
    }

    // Create n number of threads to simulate processors
    for (int i = 0; i < number_of_processors; i++) {
        if (strcmp(algorithm, "FCFS") == 0) {
            args[i] = (scheduler_args_t){
                .source_queue = (scheduling_approach == 'S') ? queue : processor_queues[i],
                .time_quantum = -1,
                .history_queue = history_queue,
                .id_of_processor = i + 1,
                .outfile = outfp,
                .outmode = outmode,
                .queue_generator_cond = (scheduling_approach == 'S') ? &queue_generator_cond
                                                                     : &processor_queue_conds[i],
                .queue_generator_lock = (scheduling_approach == 'S') ? &queue_generator_lock
                                                                     : &processor_queue_locks[i],
                .history_queue_lock = &history_queue_lock};

            if (pthread_create(&threads[i], NULL, fcfs, (void*)&args[i]) != 0) {
                printf("Error: Thread creation failed.\n");
                exit(-1);
            }
        }

        else if (strcmp(algorithm, "SJF") == 0) {
            args[i] = (scheduler_args_t){
                .source_queue = (scheduling_approach == 'S') ? queue : processor_queues[i],
                .time_quantum = -1,
                .history_queue = history_queue,
                .id_of_processor = i + 1,
                .outfile = outfp,
                .outmode = outmode,
                .queue_generator_cond = (scheduling_approach == 'S') ? &queue_generator_cond
                                                                     : &processor_queue_conds[i],
                .queue_generator_lock = (scheduling_approach == 'S') ? &queue_generator_lock
                                                                     : &processor_queue_locks[i],
                .history_queue_lock = &history_queue_lock};

            if (pthread_create(&threads[i], NULL, sjf, (void*)&args[i]) != 0) {
                printf("Error: Thread creation failed.\n");
                exit(-1);
            }
        }

        else if (strcmp(algorithm, "RR") == 0) {
            args[i] = (scheduler_args_t){
                .source_queue = (scheduling_approach == 'S') ? queue : processor_queues[i],
                .time_quantum = time_quantum,
                .history_queue = history_queue,
                .id_of_processor = i + 1,
                .outfile = outfp,
                .outmode = outmode,
                .queue_generator_cond = (scheduling_approach == 'S') ? &queue_generator_cond
                                                                     : &processor_queue_conds[i],
                .queue_generator_lock = (scheduling_approach == 'S') ? &queue_generator_lock
                                                                     : &processor_queue_locks[i],
                .history_queue_lock = &history_queue_lock};

            if (pthread_create(&threads[i], NULL, rr, (void*)&args[i]) != 0) {
                printf("Error: Thread creation failed.\n");
                exit(-1);
            }
        }
    }

    if (input_file_exists && !random_generate) {
        // Update the queue in the main thread by reading from the input file
        if (scheduling_approach == 'S') {
            update_queue_s(input_file);
        } else if (scheduling_approach == 'M') {
            update_queue_m(input_file);
        }
    } else {
        if (scheduling_approach == 'S') {
            update_queue_s_random();
        } else if (scheduling_approach == 'M') {
            update_queue_m_random();
        }
    }

    // Wait for all the threads to finish
    for (int i = 0; i < number_of_processors; i++) {
        pthread_join(threads[i], NULL);
    }

    // Print the history queue
    pthread_mutex_lock(&history_queue_lock);

    print_history_queue(history_queue, outfp);

    pthread_mutex_unlock(&history_queue_lock);

    // TODO: Free memory allocated for dynamically allocated strings
    if (queue_selection_method != NULL && queue_selection_method_exists == 1) { free(queue_selection_method); }
    if (input_file != NULL && input_file_exists == 1) { free(input_file); }
    if (algorithm != NULL && algorithm_exists == 1) { free(algorithm); }
    if (outfile != NULL) { free(outfile); }

    queue_destroy(history_queue);

    if (scheduling_approach == 'S') {
        queue_destroy(queue);
    } else if (scheduling_approach == 'M') {
        for (int i = 0; i < number_of_processors; i++) {
            queue_destroy(processor_queues[i]);
            pthread_mutex_destroy(&processor_queue_locks[i]);
            pthread_cond_destroy(&processor_queue_conds[i]);
        }

        if (processor_queues != NULL) { free(processor_queues); }
        if (processor_queue_locks != NULL) { free(processor_queue_locks); }
        if (processor_queue_conds != NULL) { free(processor_queue_conds); }
    }

    pthread_mutex_destroy(&queue_generator_lock);
    pthread_cond_destroy(&queue_generator_cond);
    pthread_mutex_destroy(&history_queue_lock);

    if (outfp != NULL) {
        fclose(outfp);
    }

    return 0;
}

void update_queue_s(char* tasks_source) {
    // Open the file
    FILE* fp = fopen(tasks_source, "r");

    if (fp == NULL) {
        printf("Failed to open the input file: %s\n", tasks_source);
        exit(-1);
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
            regfree(&regex);
            exit(-1);
        }

        else {
            if (strncmp(line, "PL", 2) == 0) {
                if (atoi(line + 3) < 0) {
                    printf("Invalid burst length in the input file: %s => Line: %s\nExiting the "
                           "program!",
                           tasks_source, line);
                    regfree(&regex);
                    exit(-1);
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

                print_for_outmode(&pcb, pcb.arrival_time, outmode,
                                  OUTMODE_3_SETTINGS_PCB_ADDED_TO_READY_QUEUE, -999, outfp);

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
                    regfree(&regex);
                    exit(-1);
                }

                current_iat += iat;

                usleep(iat * 1000);

            } else {
                printf("Invalid line encountered in the input file: %s\nLine: %s\n", tasks_source,
                       line);
                regfree(&regex);
                exit(-1);
            }
        }
        regfree(&regex);
    }

    pthread_mutex_lock(&queue_generator_lock);

    print_for_outmode(&dummy_pcb, gettimeofday_ms() - start_time, outmode,
                      OUTMODE_3_SETTINGS_PCB_ADDED_TO_READY_QUEUE, -999, outfp);

    // Add a dummy PCB to the queue to indicate the end of the file
    queue_enqueue(queue, dummy_pcb);

    pthread_mutex_unlock(&queue_generator_lock);

    pthread_cond_signal(&queue_generator_cond);

    if (fp != NULL) {
        fclose(fp);
    }
    if (line != NULL) {
        free(line);
    }
}

void update_queue_m(char* tasks_source) {
    // Open the file
    FILE* fp = fopen(tasks_source, "r");

    if (fp == NULL) {
        printf("Failed to open the input file: %s\n", tasks_source);
        exit(-1);
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
            regfree(&regex);
            exit(-1);
        }

        else {
            if (strncmp(line, "PL", 2) == 0) {
                if (atoi(line + 3) < 0) {
                    printf("Invalid burst length in the input file: %s => Line: %s\nExiting the "
                           "program!",
                           tasks_source, line);
                    regfree(&regex);
                    exit(-1);
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

                if (strcmp(queue_selection_method, "RM") == 0) {

                    int queue_id = last_pid % number_of_processors;

                    if (queue_id == 0)
                        queue_id = number_of_processors;

                    pthread_mutex_lock(&processor_queue_locks[queue_id - 1]);

                    pcb.arrival_time = gettimeofday_ms() - start_time;

                    print_for_outmode(&pcb, pcb.arrival_time, outmode,
                                      OUTMODE_3_SETTINGS_PCB_ADDED_TO_READY_QUEUE_MULTI, queue_id,
                                      outfp);

                    if (strcmp(algorithm, "SJF") == 0) {
                        queue_sorted_enqueue(processor_queues[queue_id - 1], pcb);
                    } else {
                        queue_enqueue(processor_queues[queue_id - 1], pcb);
                    }

                    pthread_mutex_unlock(&processor_queue_locks[queue_id - 1]);
                    pthread_cond_signal(&processor_queue_conds[queue_id - 1]);

                } else {
                    int min_load = get_queue_load(processor_queues[0]);
                    int id_of_min = 0;

                    for (int i = 1; i < number_of_processors; i++) {
                        int load = get_queue_load(processor_queues[i]);

                        if (load < min_load) {
                            min_load = load;
                            id_of_min = i;
                        }

                        else if (load == min_load) {
                            id_of_min = (i < id_of_min) ? i : id_of_min;
                        }
                    }

                    pthread_mutex_lock(&processor_queue_locks[id_of_min]);

                    pcb.arrival_time = gettimeofday_ms() - start_time;

                    print_for_outmode(&pcb, pcb.arrival_time, outmode,
                                      OUTMODE_3_SETTINGS_PCB_ADDED_TO_READY_QUEUE_MULTI, id_of_min + 1,
                                      outfp);

                    if (strcmp(algorithm, "SJF") == 0) {
                        queue_sorted_enqueue(processor_queues[id_of_min], pcb);
                    } else {
                        queue_enqueue(processor_queues[id_of_min], pcb);
                    }

                    pthread_mutex_unlock(&processor_queue_locks[id_of_min]);
                    pthread_cond_signal(&processor_queue_conds[id_of_min]);
                }
                last_pid++;
            } else if (strncmp(line, "IAT", 3) == 0) {
                int iat = atoi(line + 4);

                if (iat < 0) {
                    printf("Invalid IAT in the input file: %s => Line: %s\nExiting the "
                           "program!",
                           tasks_source, line);
                    regfree(&regex);
                    exit(-1);
                }

                current_iat += iat;

                usleep(iat * 1000);

            } else {
                printf("Invalid line encountered in the input file: %s\nLine: %s\n", tasks_source,
                       line);
                regfree(&regex);
                exit(-1);
            }
            regfree(&regex);
        }
    }

    // Add a dummy item to each queue

    for (int i = 0; i < number_of_processors; i++) {
        pthread_mutex_lock(&processor_queue_locks[i]);

        print_for_outmode(&dummy_pcb, gettimeofday_ms() - start_time, outmode,
                          OUTMODE_3_SETTINGS_PCB_ADDED_TO_READY_QUEUE_MULTI, i + 1, outfp);

        queue_enqueue(processor_queues[i], dummy_pcb);

        pthread_mutex_unlock(&processor_queue_locks[i]);
        pthread_cond_signal(&processor_queue_conds[i]);
    }

    if (fp != NULL) {
        fclose(fp);
    }
    if (line != NULL) {
        free(line);
    }
}

void update_queue_s_random() {
    start_time = gettimeofday_ms();
    int count = 0;
    int current_iat = 0;

    while (count < pc) {
        // First, generate random burst length

        double lambda = 1.0 / l;
        double random_burst_length;

        do {
            double random_u = (double)rand() / (double)RAND_MAX; // Betweeen 0 and 1
            random_burst_length = log(1 - random_u) / -lambda;

        } while (random_burst_length < l1 || random_burst_length > l2);

        random_burst_length = (int)round(random_burst_length);
        count++;

        // Create a new PCB
        pcb_t pcb = {.pid = count,
                     .burst_length = random_burst_length,
                     .arrival_time = -1,
                     .remaining_time = random_burst_length,
                     .finish_time = -1,
                     .turnaround_time = -1,
                     .waiting_time = -1,
                     .id_of_processor = -1,
                     .is_dummy = 0};

        pthread_mutex_lock(&queue_generator_lock);
        pcb.arrival_time = gettimeofday_ms() - start_time;

        print_for_outmode(&pcb, pcb.arrival_time, outmode, OUTMODE_3_SETTINGS_PCB_ADDED_TO_READY_QUEUE,
                          -999, outfp);

        if (strcmp(algorithm, "SJF") == 0) {
            queue_sorted_enqueue(queue, pcb);
        } else {
            queue_enqueue(queue, pcb);
        }

        pthread_mutex_unlock(&queue_generator_lock);

        printf("The size of the queue is: %d\n", queue->size);

        printf("%s\n", "a processor is signaled\n");

        pthread_cond_signal(&queue_generator_cond);

        if (count == pc)
            break;

        // Then, generate a random IAT

        double lambda_iat = 1.0 / t;
        double random_iat;

        do {
            double random_u_iat = (double)rand() / (double)RAND_MAX; // Betweeen 0 and 1
            random_iat = log(1 - random_u_iat) / -lambda_iat;

        } while (random_iat < t1 || random_iat > t2);

        random_iat = (int)round(random_iat);
        current_iat += random_iat;

        pthread_mutex_unlock(&queue_generator_lock);

        usleep(random_iat * 1000);
    }

    pthread_mutex_lock(&queue_generator_lock);

    print_for_outmode(&dummy_pcb, gettimeofday_ms() - start_time, outmode,
                      OUTMODE_3_SETTINGS_PCB_ADDED_TO_READY_QUEUE, -999, outfp);

    // Add a dummy PCB to the queue to indicate the end of the file
    queue_enqueue(queue, dummy_pcb);

    pthread_mutex_unlock(&queue_generator_lock);

    pthread_cond_signal(&queue_generator_cond);
}

void update_queue_m_random() {
    start_time = gettimeofday_ms();
    int count = 0;
    int current_iat = 0;

    while (count < pc) {
        // First, generate random burst length

        double lambda = 1.0 / l;
        double random_burst_length;

        do {
            double random_u = (double)rand() / (double)RAND_MAX; // Betweeen 0 and 1
            // printf("Random u is: %f\n", random_u);

            random_burst_length = log(1 - random_u) / -lambda;
            printf("Burst length is created with length: %f\n", random_burst_length);

        } while (random_burst_length < l1 || random_burst_length > l2);

        random_burst_length = (int)(random_burst_length);
        count++;

        // printf("Burst length is created with length: %f \n", random_burst_length);

        // Create a new PCB
        pcb_t pcb = {.pid = count,
                     .burst_length = random_burst_length,
                     .arrival_time = -1,
                     .remaining_time = random_burst_length,
                     .finish_time = -1,
                     .turnaround_time = -1,
                     .waiting_time = -1,
                     .id_of_processor = -1,
                     .is_dummy = 0};

        if (strcmp(queue_selection_method, "RM") == 0) {

            int queue_id = count % number_of_processors;

            if (queue_id == 0)
                queue_id = number_of_processors;

            pthread_mutex_lock(&processor_queue_locks[queue_id - 1]);

            pcb.arrival_time = gettimeofday_ms() - start_time;

            print_for_outmode(&pcb, pcb.arrival_time, outmode,
                            OUTMODE_3_SETTINGS_PCB_ADDED_TO_READY_QUEUE_MULTI, queue_id,
                            outfp);

            if (strcmp(algorithm, "SJF") == 0) {
                queue_sorted_enqueue(processor_queues[queue_id - 1], pcb);
            } else {
                queue_enqueue(processor_queues[queue_id - 1], pcb);
            }

            pthread_mutex_unlock(&processor_queue_locks[queue_id - 1]);
            pthread_cond_signal(&processor_queue_conds[queue_id - 1]);

        } else {
            int min_load = get_queue_load(processor_queues[0]);
            int id_of_min = 0;

            // printf("\n-----------------------------------\n");
            // printf("The load of min is %d is %d \n", 1, min_load);

            for (int i = 1; i < number_of_processors; i++) {
                int load = get_queue_load(processor_queues[i]);

                // printf("\n-----------------------------------\n");
                // printf("The load of %d is %d \n", i + 1, load);

                if (load < min_load) {
                    min_load = load;
                    id_of_min = i;
                }

                else if (load == min_load) {
                    id_of_min = (i < id_of_min) ? i : id_of_min;
                }
            }

            pthread_mutex_lock(&processor_queue_locks[id_of_min]);

            pcb.arrival_time = gettimeofday_ms() - start_time;

            print_for_outmode(&pcb, pcb.arrival_time, outmode,
                              OUTMODE_3_SETTINGS_PCB_ADDED_TO_READY_QUEUE_MULTI, id_of_min + 1, outfp);

            if (strcmp(algorithm, "SJF") == 0) {
                queue_sorted_enqueue(processor_queues[id_of_min], pcb);
            } else {
                queue_enqueue(processor_queues[id_of_min], pcb);
                // printf("The load of %d after enqueue %d \n", id_of_min,
                // get_queue_load(processor_queues[id_of_min]));
            }

            pthread_mutex_unlock(&processor_queue_locks[id_of_min]);

            // printf("Processor %d is signaled after the addition of an item. \n", id_of_min + 1);
            pthread_cond_signal(&processor_queue_conds[id_of_min]);
        }

        if (count == pc)
            break;

        // Then, generate a random IAT

        double lambda_iat = 1.0 / t;
        double random_iat;

        do {
            double random_u_iat = (double)rand() / (double)RAND_MAX; // Betweeen 0 and 1
            random_iat = log(1 - random_u_iat) / -lambda_iat;

        } while (random_iat < t1 || random_iat > t2);

        random_iat = (int)round(random_iat);
        current_iat += random_iat;

        printf("IAT is created with: %f\n", random_iat);

        pthread_mutex_unlock(&queue_generator_lock);

        usleep(random_iat * 1000);
    }

    for (int i = 0; i < number_of_processors; i++) {
        pthread_mutex_lock(&processor_queue_locks[i]);

        print_for_outmode(&dummy_pcb, gettimeofday_ms() - start_time, outmode,
                          OUTMODE_3_SETTINGS_PCB_ADDED_TO_READY_QUEUE_MULTI, i + 1, outfp);

        queue_enqueue(processor_queues[i], dummy_pcb);

        pthread_mutex_unlock(&processor_queue_locks[i]);
        pthread_cond_signal(&processor_queue_conds[i]);
    }
}
