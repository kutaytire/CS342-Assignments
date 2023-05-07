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
int available_res[MAXR];
thread_info threads[MAXP];

//..... other definitions/variables .....
pthread_mutex_t lock;

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

    // Initialize max_demand matrix
    for (int i = 0; i < num_threads; i++) {
        for (int j = 0; j < num_resources; j++) {
            max_demand[i][j] = 0;
        }
    }

    // Initialize mutex lock
    if (pthread_mutex_init(&lock, NULL) != 0) {
        return -1; // Error: Mutex lock initialization failed
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
