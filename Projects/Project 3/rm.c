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
int available_res[MAXR];
thread_info threads[MAXP];

//..... other definitions/variables .....
pthread_mutex_t lock;
pthread_cond_t* conds;

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
        }
    }

    // Initialize mutex lock
    if (pthread_mutex_init(&lock, NULL) != 0) {
        return -1; // Error: Mutex lock initialization failed
    }

    conds = malloc(sizeof(pthread_cond_t) * num_threads);

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
    if (tid < 0 || tid >= num_threads || threads[tid].state == NOT_STARTED) {
        return -1; // invalid tid or thread not started
    }

    // Free all the resources held by the thread
    int i;
    for (i = 0; i < num_resources; i++) {
        available_res[i] += max_demand[tid][i];
    }

    // Mark the thread as terminated
    threads[tid].state = TERMINATED;
    return 0;
}


int rm_claim (int claim[])
{
    pthread_t tid = pthread_self();

    for (int i = 0; i < num_resources; i++) {
        // Check if claim is valid
        if (claim[i] > existing_res[i]) {
            return -1;
        }
        // Update max_demand matrix with claim information
        max_demand[(long) tid][i] = claim[i];
    }
    return 0;
}

int rm_request (int request[])
{

    int allocate = 1;
    pthread_t tid = pthread_self();

    // First check whether the request can be granted in the first place
    pthread_mutex_lock(&lock);
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
        int need_matrix[MAXP][MAXR];

        // Allocation
        for(int i = 0; i < num_threads; i++) {
            for (int j = 0; j < num_resources; j++) {

                allocation_copy[i][j] = allocation[i][j];
            }
        }

        //Initialize the copy available matrix and grant the request
        for(int k = 0; k < num_resources; k++) {
            available_res_copy[k] = available_res[k];

            allocation_copy[(long) tid][k] += request[k];
            available_res_copy[k] -= request[k];
        }

        // Create the need matrix
        for(int i = 0; i < num_threads; i++) {
            for(int j = 0; j < num_resources; j++) {

                need_matrix[i][j] = max_demand[i][j] - allocation_copy[i][j];
            }
        }

        int count = 0;
        int already_checked = 0; // Used if there is a deadlock and no process can process
        while(1) {
        // Check if the new state is safe
            for(int i = 0; i < num_threads; i++) {

                int can_process = 1;
                for(int j = 0; j < num_resources; j++) {

                    // If already processed or need cannot be met, skip the thread
                    if(available_res_copy[j] < need_matrix[i][j] || need_matrix[i][j] < 0) {
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
                        need_matrix[i][j] = -1;
                        // Update available resources (Assume the process is finished)
                        available_res_cop[j] += allocation_copy[i][j];
                    }
                }
            }

            // If all threads have processed or there is a deadlock, leave the loop
            if(count == num_threads || already_checked >= num_threads)
                break;

        }

        // Check if the available resources are equal to existing resources
        for (int i = 0; i < num_resources; i++) {

            if(existing_res[i] != available_res_copy[i])
                allocate = 0;   
        }
    }

    while (!allocate) {
        pthread_cond_wait(&conds[tid], &lock);
    }

    for (int i = 0; i < num_resources; i++) {

        allocation[(long) tid][i] += request[i];
        available_res[i] -= request[i]; 
    }
    pthread_mutex_unlock(&lock);
   
    int ret = 0;

    return(ret);
}


int rm_release (int release[])
{
    int ret = 0;

    return (ret);
}


int rm_detection()
{
    int ret = 0;

    return (ret);
}


void rm_print_state (char hmsg[])
{
    return;
}
