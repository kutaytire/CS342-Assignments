#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common_defs.h"
#include "queue.h"
#include "pcb.h"
#include "queue.h"
#include "util.h"
#include <regex.h>
#include <unistd.h>
#include "schedulers.h"

// Function Definitions

void update_queue(char* tasks_source);

// Variables

int number_of_processors = DEFAULT_N;
char scheduling_approach = DEFAULT_SAP;
char* queue_selection_method = DEFAULT_QS;
char* algorithm = DEFAULT_ALG;
int time_quantum = DEFAULT_Q;
char* input_file = DEFAULT_INFILE;
int outmode = DEFAULT_OUTMODE;
char *outfile = DEFAULT_OUTFILE;
int t = DEFAULT_T;
int t1 = DEFAULT_T1;
int t2 = DEFAULT_T2;
int l = DEFAULT_L;
int l1 = DEFAULT_L1;
int l2 = DEFAULT_L2;

queue_t* queue;
pthread_mutex_t lock;


// Main Program

int main(int argc, char* argv[]) {

   // Parse the arguments

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            i++;
            number_of_processors = atoi(argv[i]);
        } else if (strcmp(argv[i], "-a") == 0) {
            i++;
            scheduling_approach = argv[i][0];

            i++;
            strcpy(queue_selection_method, argv[i]);

        } else if (strcmp(argv[i], "-s") == 0) {
            i++;
            strcpy(algorithm, argv[i]);

            i++;
            time_quantum = atoi(argv[i]);

        } else if (strcmp(argv[i], "-i") == 0) {
            i++;
            strcpy(input_file, argv[i]);

        } else if (strcmp(argv[i], "-m") == 0) {
            i++;
            outmode = atoi(argv[i]);

        } else if (strcmp(argv[i], "-o") == 0) {
            i++;
            strcpy(outfile, argv[i]);

        } else if (strcmp(argv[i], "-r") == 0) {
            i++;
            t = atoi(argv[i]);
            i++;
            t1 = atoi(argv[i]);
            i++;
            t2 = atoi(argv[i]);
            i++;
            l = atoi(argv[i]);
            i++;
            l1 = atoi(argv[i]);
            i++;
            l2 = atoi(argv[i]);
        }
    }

    // common queue for single-queue scheduling algorithm
    queue = queue_create();
    pthread_mutex_init(&lock, NULL);
    pthread_t threads[number_of_processors];

    // Create n number of threads to stimulate processors

    for(int i = 0; i < number_of_processors; i++) {

        if (strcmp(algorithm, "FCFS") == 0) {

            if (pthread_create(&threads[i], NULL, fcfs, NULL ) != 0) {
                printf("Error: Thread creation failed.\n");
                return -1;
            }
        }

        else if (strcmp(algorithm, "SJF") == 0) {

            if (pthread_create(&threads[i], NULL, sjf, NULL ) != 0) {
                printf("Error: Thread creation failed.\n");
                return -1;
            }

        }

        else {

            if (pthread_create(&threads[i], NULL, round_robin, NULL ) != 0) {
                printf("Error: Thread creation failed.\n");
                return -1;
            }

        }
    }

    // Update the queue in the main thread by reading from the input file

    // Free memory allocated for dynamically allocated strings


    return 0;

}

void update_queue(char* tasks_source) {

    // Open the file
    FILE* fp = fopen(tasks_source, "r");

    // Check if the file exists
    if (fp == NULL) {
        printf("Failed to open the input file: %s\n", tasks_source);
        exit(-1);
    }

    regex_t regex; // To check if the line is in the correct format
    char* line = NULL;

    int current_iat = 0;
    int last_pid = 1;

    long long start_time = gettimeofday_ms();

    // Read line by line
    while (getline(&line, NULL, fp) != -1) {
        // Each line should follow the format:
        // PL <int> or IAT <int>
        // If the line is not in the correct format, exit the program
        if (regcomp(&regex, "^(PL|IAT) [0-9]+$", REG_EXTENDED) != 0) {
            printf("Invalid line encountered in the input file: %s\nLine: %s\n", tasks_source,
                   line);
            exit(-1);
        }

        else {
            if (strncmp(line, "PL", 2) == 0) {
                long long current_time = gettimeofday_ms();
                // Create a new PCB
                pcb_t pcb = {.pid = last_pid,
                             .burst_length = atoi(line + 3),
                             .arrival_time = current_time,
                             .remaining_time = 0,
                             .finish_time = -1,
                             .turnaround_time = -1,
                             .waiting_time = current_time - start_time,
                             .id_of_processor = NULL};

                // mutex

                queue_enqueue(queue, pcb);

            } else if (strncmp(line, "IAT", 3) == 0) {
                int iat = atoi(line + 4);
                current_iat += iat;

                usleep(iat * 1000);

            } else {
                printf("Invalid line encountered in the input file: %s\nLine: %s\n", tasks_source,
                       line);
                exit(-1);
            }
        }
    }

    // Add a dummy PCB to the queue to indicate the end of the file
    pcb_t pcb = {.pid = -1,
                 .burst_length = -1,
                 .arrival_time = -1,
                 .remaining_time = -1,
                 .finish_time = -1,
                 .turnaround_time = -1,
                 .waiting_time = -1,
                 .id_of_processor = NULL};

    queue_enqueue(queue, pcb);

    return;
}