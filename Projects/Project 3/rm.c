#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <pthread.h>
#include "rm.h"

typedef enum {
    NOT_STARTED,
    RUNNING,
    WAITING,
    TERMINATED
} state;

typedef struct {
    pthread_t tid;
    state state;
} thread_info;

// global variables

int deadlock_avoidance;  // indicates if deadlocks will be avoided or not
int num_threads;   // number of processes
int num_resources;   // number of resource types
int existing_res[MAXR]; // Existing resources vector
int max_demand[MAXP][MAXR];
int allocation[MAXP][MAXR];
int need_matrix[MAXP][MAXR];
int request_matrix[MAXP][MAXR];
int available_res[MAXR];
thread_info threads[MAXP];

//..... other definitions/variables .....
pthread_mutex_t lock;
pthread_cond_t conds[MAXP];

// end of global variables

int rm_init(int p_count, int r_count, int r_exist[],  int avoid)
{
    // Validate input parameters
    if (p_count <= 0 || r_count <= 0 || p_count > MAXP || r_count > MAXR) {
        return -1; // Error: Invalid parameters
    }

    num_threads = p_count;
    num_resources = r_count;
    deadlock_avoidance = avoid;

    // initialize (create) resources
    for (int i = 0; i < num_resources; i++) {
        existing_res[i] = r_exist[i];
        available_res[i] = r_exist[i];
    }
    // resources initialized (created)

    // Initialize threads array
    for (int i = 0; i < num_threads; i++) {
        threads[i].tid = 0;
        threads[i].state = NOT_STARTED;
    }

    // Initialize max_demand and allocation matrices
    for (int i = 0; i < num_threads; i++) {
        for (int j = 0; j < num_resources; j++) {
            max_demand[i][j] = 0;
            allocation[i][j] = 0;
            need_matrix[i][j] = 0;
            request_matrix[i][j] = 0;
        }
    }

    // Initialize mutex lock
    if (pthread_mutex_init(&lock, NULL) != 0) {
        return -1; // Error: Mutex lock initialization failed
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_cond_init(&conds[i], NULL);
    }

    return 0;
}


int rm_thread_started(int tid)
{
    // Check if the tid is within the range [0, N-1]
    if (tid < 0 || tid >= num_threads) {
        return -1;
    }

    // Associate the tid value with the internal thread ID
    pthread_t thread_id = pthread_self();
    threads[tid].tid = thread_id;

    // Set the initial state for this thread
    threads[tid].state = RUNNING;

    return 0;
}


int rm_thread_ended()
{
    long tid = (long) pthread_self();
    int real_tid = -1;
    for (int i = 0; i < num_threads; i++) {
        if ((long) threads[i].tid == tid) {
            real_tid = i;
            break;
        }
    }

    pthread_mutex_lock(&lock);

    // Free all the resources held by the thread
    for (int i = 0; i < num_resources; i++) {
        available_res[i] += allocation[real_tid][i];
        need_matrix[real_tid][i] += allocation[real_tid][i];
        allocation[real_tid][i] = 0;
    }

    // Wake up all the threads that are waiting for resources
    for (int i = 0; i < num_threads; i++) {
        pthread_cond_signal(&conds[i]);
    }

    pthread_mutex_unlock(&lock);

    // Mark the thread as terminated
    threads[real_tid].state = TERMINATED;
    printf("\n\nThread %d terminated\n", real_tid);
    return 0;
}


int rm_claim (int claim[])
{
    long tid = (long) pthread_self();

    int real_tid = -1;
    for (int i = 0; i < num_threads; i++) {
        if ((long) threads[i].tid == tid) {
            real_tid = i;
            break;
        }
    }

    if(deadlock_avoidance) {

        for (int i = 0; i < num_resources; i++) {
            // Check if claim is valid
            if (claim[i] > existing_res[i]) {
                return -1;
            }
            // Update max_demand matrix with claim information
            need_matrix[real_tid][i] = claim[i];
            max_demand[real_tid][i] = claim[i];
        }
    }
    return 0;
}

int rm_request (int request[])
{

    for(int i = 0; i < num_resources; i++) {

        if(request[i] > existing_res[i]) {
            return -1;
        }
    }

    int allocate = 1;
    long tid = (long) pthread_self();

    int real_tid = -1;
    for (int i = 0; i < num_threads; i++) {
        if ((long) threads[i].tid == tid) {
            real_tid = i;
            break;
        }
    }

    // First check whether the request can be granted in the first place
    pthread_mutex_lock(&lock);

    allocate_calculation:
    allocate = 1;

    // Initialize the request matrix

    for(int i = 0; i < num_resources; i++) {

        request_matrix[real_tid][i] = request[i];
    }

    for(int i = 0; i < num_resources; i++){

        if(request[i] > available_res[i]) {
            allocate = 0;
        }
    }


    // If the request can be granted and avoidance is used, check if the new state is safe
    if(deadlock_avoidance && allocate) {

        // Create a copy of allocation and available matrices

        int allocation_copy[MAXP][MAXR];
        int available_res_copy[MAXR];
        int need_matrix_copy[MAXP][MAXR];

        // Allocation
        for(int i = 0; i < num_threads; i++) {
            for (int j = 0; j < num_resources; j++) {

                allocation_copy[i][j] = allocation[i][j];
                need_matrix_copy[i][j] = need_matrix[i][j];
            }
        }

        //Initialize the copy available matrix and grant the request
        for(int k = 0; k < num_resources; k++) {
            available_res_copy[k] = available_res[k];

            allocation_copy[real_tid][k] += request[k];
            available_res_copy[k] -= request[k];
            need_matrix_copy[real_tid][k] -= request[k];
        }


        printf("\n###########################\n");
        printf("Hypothetic state suppose that we granted\n");
        printf("###########################\n");
        printf("Exist:\n");
        for (int i = 0; i < num_resources; i++) {
            printf("\tR%d ", i);
        }
        printf("\n");
        for (int i = 0; i < num_resources; i++) {
            printf("\t%d ", existing_res[i]);
        }
        printf("\n");
        printf("\nAvailable:\n");
        for (int i = 0; i < num_resources; i++) {
            printf("\tR%d ", i);
        }
        printf("\n");
        for (int i = 0; i < num_resources; i++) {
            printf("\t%d ", available_res_copy[i]);
        }
        printf("\n");
        printf("\nAllocation:\n");
        for (int i = 0; i < num_resources; i++) {
            printf("\tR%d ", i);
        }
        printf("\n");
        for (int i = 0; i < num_threads; i++) {
            printf("T%d: ", i);
            for (int j = 0; j < num_resources; j++) {
                printf("\t%d ", allocation_copy[i][j]);
            }
            printf("\n");
        }
        printf("\nRequest:\n");
        for (int i = 0; i < num_resources; i++) {
            printf("\tR%d ", i);
        }
        printf("\n");
        for (int i = 0; i < num_threads; i++) {
            printf("T%d: ", i);
            for (int j = 0; j < num_resources; j++) {
                printf("\t%d ", request_matrix[i][j]);
            }
            printf("\n");
        }
        printf("\nMaxDemand:\n");
        for (int i = 0; i < num_resources; i++) {
            printf("\tR%d ", i);
        }
        printf("\n");
        for (int i = 0; i < num_threads; i++) {
            printf("T%d: ", i);
            for (int j = 0; j < num_resources; j++) {
                printf("\t%d ", max_demand[i][j]);
            }
            printf("\n");
        }
        printf("\n");
        printf("Need:\n");
        for (int i = 0; i < num_resources; i++) {
            printf("\tR%d ", i);
        }
        printf("\n");
        for (int i = 0; i < num_threads; i++) {
            printf("T%d: ", i);
            for (int j = 0; j < num_resources; j++) {
                printf("\t%d ", need_matrix_copy[i][j]);
            }
            printf("\n");
        }
        printf("\n");




        int count = 0;
        int already_checked = 0; // Used if there is a deadlock and no process can process
        while(1) {
        // Check if the new state is safe
            for(int i = 0; i < num_threads; i++) {

                int can_process = 1;
                for(int j = 0; j < num_resources; j++) {

                    // If already processed or need cannot be met, skip the thread
                    if(available_res_copy[j] < need_matrix_copy[i][j] || need_matrix_copy[i][j] < 0) {
                        can_process = 0;
                        already_checked += 1;
                        break;
                    }
                }

                // If can process, update the new matrices
                if(can_process) {
                    count += 1;
                    already_checked = 0;
                    for(int j = 0; j < num_resources; j++) {

                        // Give -1 for the need matrix entries to prevent processing again
                        need_matrix_copy[i][j] = -1;
                        // Update available resources (Assume the process is finished)
                        available_res_copy[j] += allocation_copy[i][j];
                        allocation_copy[i][j] = 0;
                    }

                    printf("\n###########################\n");
                    printf("Hypothetic state suppose that we granted to thread %d\n", i);
                    printf("###########################\n");
                    printf("Exist:\n");
                    for (int i = 0; i < num_resources; i++) {
                        printf("\tR%d ", i);
                    }
                    printf("\n");
                    for (int i = 0; i < num_resources; i++) {
                        printf("\t%d ", existing_res[i]);
                    }
                    printf("\n");
                    printf("\nAvailable:\n");
                    for (int i = 0; i < num_resources; i++) {
                        printf("\tR%d ", i);
                    }
                    printf("\n");
                    for (int i = 0; i < num_resources; i++) {
                        printf("\t%d ", available_res_copy[i]);
                    }
                    printf("\n");
                    printf("\nAllocation:\n");
                    for (int i = 0; i < num_resources; i++) {
                        printf("\tR%d ", i);
                    }
                    printf("\n");
                    for (int i = 0; i < num_threads; i++) {
                        printf("T%d: ", i);
                        for (int j = 0; j < num_resources; j++) {
                            printf("\t%d ", allocation_copy[i][j]);
                        }
                        printf("\n");
                    }
                    printf("\nRequest:\n");
                    for (int i = 0; i < num_resources; i++) {
                        printf("\tR%d ", i);
                    }
                    printf("\n");
                    for (int i = 0; i < num_threads; i++) {
                        printf("T%d: ", i);
                        for (int j = 0; j < num_resources; j++) {
                            printf("\t%d ", request_matrix[i][j]);
                        }
                        printf("\n");
                    }
                    printf("\nMaxDemand:\n");
                    for (int i = 0; i < num_resources; i++) {
                        printf("\tR%d ", i);
                    }
                    printf("\n");
                    for (int i = 0; i < num_threads; i++) {
                        printf("T%d: ", i);
                        for (int j = 0; j < num_resources; j++) {
                            printf("\t%d ", max_demand[i][j]);
                        }
                        printf("\n");
                    }
                    printf("\n");
                    printf("Need:\n");
                    for (int i = 0; i < num_resources; i++) {
                        printf("\tR%d ", i);
                    }
                    printf("\n");
                    for (int i = 0; i < num_threads; i++) {
                        printf("T%d: ", i);
                        for (int j = 0; j < num_resources; j++) {
                            printf("\t%d ", need_matrix_copy[i][j]);
                        }
                        printf("\n");
                    }
                    printf("\n");
                }
            }



            // If all threads have processed or there is a deadlock, leave the loop
            if(count == num_threads || already_checked >= num_threads)
                break;

        }



        printf("count is %d", count);
        printf("num of threads is %d", num_threads);

        if(count != num_threads) {
            allocate = 0;
            printf("Possible deadlock detected\n");
        }
    }

    while (!allocate) {
        printf("Waiting\n\n");
        threads[real_tid].state = WAITING;
        pthread_cond_wait(&conds[real_tid], &lock);
        printf("\n\nCheck go to\n\n");
        goto allocate_calculation;
    }

    rm_print_state("Debug1");

    for (int i = 0; i < num_resources; i++) {

        allocation[real_tid][i] += request[i];
        available_res[i] -= request[i];

        if(deadlock_avoidance) {
            need_matrix[real_tid][i] -= request[i];
        }
        request_matrix[real_tid][i] = 0;
    }

    rm_print_state("After allocation");

    pthread_mutex_unlock(&lock);

    return 0;
}


int rm_release (int release[])
{
    if (pthread_mutex_lock(&lock) != 0) {
        fprintf(stderr, "Error: failed to lock mutex\n");
        return -1;
    }

    long tid = (long) pthread_self();
    int real_tid = -1;
    for (int i = 0; i < num_threads; i++) {
        if ((long) threads[i].tid == tid) {
            real_tid = i;
            break;
        }
    }

    for (int i = 0; i < num_resources; i++) {
        if (release[i] < 0) {
            fprintf(stderr, "Error: attempting to release negative instances of resource type %d\n", i);
            pthread_mutex_unlock(&lock);
            return -1;
        }

        available_res[i] += release[i];
        if(deadlock_avoidance) {
            need_matrix[real_tid][i] += release[i];
        }
        allocation[real_tid][i] -= release[i];
    }

    for (int i = 0; i < num_threads; i++) {

        pthread_cond_signal(&conds[i]);
    }

    rm_print_state("After release");

    pthread_mutex_unlock(&lock);

    return 0;
}


int rm_detection()
{
    pthread_mutex_lock(&lock);

    // Create a copy of allocation and available matrices
    int allocation_copy[MAXP][MAXR];
    int available_res_copy[MAXR];
    int request_matrix_copy[MAXP][MAXR];

    // Allocation
    for(int i = 0; i < num_threads; i++) {
        for (int j = 0; j < num_resources; j++) {
            allocation_copy[i][j] = allocation[i][j];
            request_matrix_copy[i][j] = request_matrix[i][j];
        }
    }

    // Copy available matrix
    for(int k = 0; k < num_resources; k++) {
        available_res_copy[k] = available_res[k];
    }

    int count = 0;
    int already_checked = 0; // Used if there is a deadlock and no process can process
    while(1) {
    // Check if the new state is safe
        for(int i = 0; i < num_threads; i++) {

            int can_process = 1;
            for(int j = 0; j < num_resources; j++) {

                // If already processed or request cannot be met, skip the thread
                if(available_res_copy[j] < request_matrix_copy[i][j] || request_matrix_copy[i][j] < 0) {
                    can_process = 0;
                    already_checked += 1;
                    break;
                }
            }

            // If can process, update the new matrices
            if(can_process) {
                count += 1;
                already_checked = 0;
                for(int j = 0; j < num_resources; j++) {

                    // Give -1 for the need matrix entries to prevent processing again
                    request_matrix_copy[i][j] = -1;
                    // Update available resources (Assume the process is finished)
                    available_res_copy[j] += allocation_copy[i][j];
                    }
                }
            }

        // If all threads have processed or there is a deadlock, leave the loop
        if(count == num_threads || already_checked >= num_threads)
            break;
    }

    pthread_mutex_unlock(&lock);

    return num_threads - count;

}


void rm_print_state (char hmsg[])
{
    printf("\n###########################\n");
    printf("%s\n", hmsg);
    printf("###########################\n");
    printf("Exist:\n");
    for (int i = 0; i < num_resources; i++) {
        printf("\tR%d ", i);
    }
    printf("\n");
    for (int i = 0; i < num_resources; i++) {
        printf("\t%d ", existing_res[i]);
    }
    printf("\n");
    printf("\nAvailable:\n");
    for (int i = 0; i < num_resources; i++) {
        printf("\tR%d ", i);
    }
    printf("\n");
    for (int i = 0; i < num_resources; i++) {
        printf("\t%d ", available_res[i]);
    }
    printf("\n");
    printf("\nAllocation:\n");
    for (int i = 0; i < num_resources; i++) {
        printf("\tR%d ", i);
    }
    printf("\n");
    for (int i = 0; i < num_threads; i++) {
        printf("T%d: ", i);
        for (int j = 0; j < num_resources; j++) {
            printf("\t%d ", allocation[i][j]);
        }
        printf("\n");
    }
    printf("\nRequest:\n");
    for (int i = 0; i < num_resources; i++) {
        printf("\tR%d ", i);
    }
    printf("\n");
    for (int i = 0; i < num_threads; i++) {
        printf("T%d: ", i);
        for (int j = 0; j < num_resources; j++) {
            printf("\t%d ", request_matrix[i][j]);
        }
        printf("\n");
    }
    printf("\nMaxDemand:\n");
    for (int i = 0; i < num_resources; i++) {
        printf("\tR%d ", i);
    }
    printf("\n");
    for (int i = 0; i < num_threads; i++) {
        printf("T%d: ", i);
        for (int j = 0; j < num_resources; j++) {
            printf("\t%d ", max_demand[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    printf("Need:\n");
    for (int i = 0; i < num_resources; i++) {
        printf("\tR%d ", i);
    }
    printf("\n");
    for (int i = 0; i < num_threads; i++) {
        printf("T%d: ", i);
        for (int j = 0; j < num_resources; j++) {
            printf("\t%d ", need_matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}