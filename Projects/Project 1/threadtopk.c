#include "common_defs.h"
#include "file_processor.h"
#include "freq_table.h"
#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

pthread_mutex_t lock;
FreqTable* freq_table;
int k_most_frequent_words;

char** file_names = NULL; // Global "filenames table" as stated in the project description
                          // It will be initialized in the main function, but for now it is
                          // declared as NULL. It will store a pointer to an array of strings
                          // located in the heap. Each string will be a corresponding file name.

// Function declaration

void* update_freq_table(void* file_name);

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
        file_names = malloc(sizeof(char*) * number_of_files);
        for (int i = 0; i < number_of_files; i++) {
            file_names[i] = malloc(sizeof(char) * (strlen(argv[i + 4]) + 1));
            strcpy(file_names[i], argv[i + 4]);
        }

        pthread_mutex_init(&lock, NULL);

        // Create an array of threads
        pthread_t threads[number_of_files];
        freq_table = new_freq_table(1);

        for (int i = 0; i < number_of_files; i++) {
            if (pthread_create(&threads[i], NULL, update_freq_table, file_names[i]) != 0) {
                fprintf(stderr, "Thread creation failed!");
                exit(-1);
            }
        }

        // Wait for all threads to finish
        for (int i = 0; i < number_of_files; i++) {
            pthread_join(threads[i], NULL);
        }

        // To give the exact size if the final frequency table contains less than K words
        int* length = malloc(sizeof(int));

        FreqRecord* final_freq_records =
            find_most_k_freq_words_from_freq_table(freq_table, k_most_frequent_words, length);

        print_to_file(argv[2], final_freq_records, *length);

        free_freq_table(freq_table);

        for (int i = 0; i < number_of_files; i++) {
            free(file_names[i]);
        }
        free(file_names);

        free(final_freq_records);
        free(length);

        pthread_mutex_destroy(&lock);
    }

    return 0;
}

void* update_freq_table(void* file_name) {
    pthread_mutex_lock(&lock);

    // To give the exact size if file's frequency table contains less than K words
    int* length = malloc(sizeof(int));

    FreqRecord* most_frequent_words =
        find_most_k_freq_words_from_file((char*)file_name, k_most_frequent_words, length, 0);


    for (int i = 0; i < *length; i++) {
        add_freq_record(freq_table, &(most_frequent_words[i]));
    }

    /*  Test
    printf("Thread initialized to process %s.\n", (char*)file_name);
    for (int i = 0; i < *length; i++) {

        printf("Word: %s Freq: %d\n", most_frequent_words[i].word,
    most_frequent_words[i].frequency);
    }
    print_freq_table(freq_table);
    */

    pthread_mutex_unlock(&lock);

    free(length);
    free(most_frequent_words);

    pthread_exit(0);
}
