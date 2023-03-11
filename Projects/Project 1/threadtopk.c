#include "common_defs.h"
#include "file_processor.h"
#include "freq_table.h"
#include "pthread.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>



typedef struct {
    char* file_name;
    int status; // 0 -> not processed, 1 -> processed, or being processed
} FileInfo;

pthread_mutex_t lock;
FreqTable* freq_table;
int k_most_frequent_words;

FileInfo* file_names = NULL; // Global "filenames table" as stated in the project description
                             // It will be initialized in the main function, but for now it is
                             // declared as NULL. It will store a pointer to an array of strings
                             // located in the heap. Each string will be a corresponding file name.

// Function

void* update_freq_table(void* file_void);

// Program Start

int main(int argc, char* argv[]) {
    /**
        Example input: threadtopk <K> <outfile> <N> <infile1> .... <infileN>
        For example: threadtopk 100 outfile.txt 3 in1.txt in2.txt in3.txt
        K -> K most frequently occurring words
        outfile -> output file
        N -> number of input files
    */ 
    if (argc < 5) {
        printf("%s\n", "Not enough arguments");
        printf("%s\n", "Example input: proctopk <K> <outfile> <N> <infile1> .... <infileN>");
        printf(
            "%s",
            "Where K is the number of most frequently occurring words, outfile is the output file, "
            "N is the number of input files, and infile1 ... infileN are the input files.");
        exit(-1);
    }

    else {
        int number_of_files = atoi(argv[3]);
        k_most_frequent_words = atoi(argv[1]);

        if (number_of_files != argc - 4) {
            printf("%s", "Number of input files does not match the number of input files "
                         "specified in the command line arguments.");
            exit(-1);
        }

        if (number_of_files < MIN_N || number_of_files > MAX_N) {
            printf("%s", "Number of input files can be minimum 1 and maximum 10.");
            exit(-1);
        }

        if (k_most_frequent_words < MIN_K || k_most_frequent_words > MAX_K) {
            printf("%s", "K can be minimum 1 and maximum 1000.");
            exit(-1);
        }

        // Global "filenames table" as stated in the project description
        file_names = malloc(sizeof(FileInfo) * number_of_files);
        memset(file_names, 0, sizeof(FileInfo) * number_of_files);
        for (int i = 0; i < number_of_files; i++) {
            file_names[i].file_name = malloc(sizeof(char) * (strlen(argv[i + 4]) + 1));
            strcpy(file_names[i].file_name, argv[i + 4]);
            file_names[i].status = 0;
        }

        pthread_mutex_init(&lock, NULL);

        // Create an array of threads
        pthread_t threads[number_of_files];
        freq_table = new_freq_table(1);

        for(int i = 0; i < number_of_files; i++) {
            if (pthread_create(&threads[i], NULL, update_freq_table, file_names[i].file_name) != 0){
                printf("Error: Thread creation failed.\n");
                return -1;
            }
        }
        // Wait for all threads to finish
        for (int i = 0; i < number_of_files; i++) {
            pthread_join(threads[i], NULL);
        }

        FreqRecord* final_freq_records =
            find_most_k_freq_words_from_freq_table(freq_table, k_most_frequent_words);

        print_to_file(argv[2], final_freq_records, k_most_frequent_words);

        free_freq_table(freq_table);

        for (int i = 0; i < number_of_files; i++) {
            free(file_names[i].file_name);
        }
        free(file_names);
        free(final_freq_records);
        
        pthread_mutex_destroy(&lock);
    }

    return 0;
        
}

void* update_freq_table(void* file_void) {

    char* file_name = (char*) file_void;
    pthread_mutex_lock(&lock);

    FreqRecord* most_frequent_words = find_most_k_freq_words_from_file(file_name, k_most_frequent_words);

    // Does not give the exact size if file contains less words than k.
    //int length = sizeof(most_frequent_words)/sizeof(FreqRecord);

    for (int i = 0; i < k_most_frequent_words; i++) {

        add_freq_record(freq_table, &(most_frequent_words[i]));
    }
    
    /*  Test
    printf("Thread initialized to process %s.\n", file_name);
    for (int i = 0; i < k_most_frequent_words; i++) {

        printf("Word: %s Freq: %d\n", most_frequent_words[i].word, most_frequent_words[i].frequency);
    }
    print_freq_table(freq_table);
    */

    pthread_mutex_unlock(&lock);
    return NULL;

}