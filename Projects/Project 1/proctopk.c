#include "common_defs.h"
#include "file_processor.h"
#include "freq_table.h"
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

const char* SM_NAME = "proctopk_shared_memory"; // A global variable for the shared memory name,
                                                //  as stated in the project description
char** file_names = NULL; // Global "filenames table" as stated in the project description
                          // It will be initialized in the main function, but for now it is
                          // declared as NULL. It will store a pointer to an array of strings
                          // located in the heap. Each string will be a corresponding file name.

// Program Start

int main(int argc, char* argv[]) {
    /**
        Example input: proctopk <K> <outfile> <N> <infile1> .... <infileN>
        For example: proctopk 100 outfile.txt 3 in1.txt in2.txt in3.txt
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
        int k_most_frequent_words = atoi(argv[1]);

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

        // Create a shared memory segment
        int sm_item_count = number_of_files * k_most_frequent_words;
        const int SM_SIZE = sm_item_count * sizeof(FreqRecord);
        int shm_fd;    // Shared memory file descriptor
        void* shm_ptr; // Shared memory pointer
        shm_fd = shm_open(SM_NAME, O_CREAT | O_RDWR, 0666);
        ftruncate(shm_fd, SM_SIZE);
        shm_ptr = mmap(0, SM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

        if (shm_ptr == MAP_FAILED) {
            fprintf(stderr, "Shared memory mapping failed!");
            exit(-1);
        }

        // Create N child processes
        for (int i = 0; i < number_of_files; i++) {
            pid_t pid = fork();
            if (pid < 0) {
                fprintf(stderr, "Fork failed!");
                exit(-1);
            }

            else if (pid == 0) {
                // Child logic
                // Find the K-most occuring words
                FreqRecord* most_frequent_words =
                    find_most_k_freq_words_from_file(file_names[i], k_most_frequent_words, NULL, 1);

                // Copy the K-most occuring words to the shared memory segment
                memcpy(shm_ptr + i * k_most_frequent_words * sizeof(FreqRecord),
                       most_frequent_words, k_most_frequent_words * sizeof(FreqRecord));

                free(most_frequent_words);

                // NOTE: Normally we already free these in the parent process (see end of the main),
                // but we are doing it here as well to avoid memory leaks as Valgrind labels them
                // as "still reachable" inside the child processes.
                for (int i = 0; i < number_of_files; i++) {
                    free(file_names[i]);
                }
                free(file_names);
                close(shm_fd);
                // END NOTE

                exit(0);
            }

            else { // pid > 0
                //  Parent logic
            }
        }

        // Wait for all child processes to finish
        for (int i = 0; i < number_of_files; i++) {
            wait(NULL);
        }

        // Create a frequency table from the shared memory segment
        FreqTable* shm_ptr_freq_table =
            new_freq_table_from_freq_records((FreqRecord*)shm_ptr, sm_item_count);

        int* length = malloc(sizeof(int));

        FreqRecord* final_freq_records =
            find_most_k_freq_words_from_freq_table(shm_ptr_freq_table, k_most_frequent_words, length);

        print_to_file(argv[2], final_freq_records, *length);

        free_freq_table(shm_ptr_freq_table);

        for (int i = 0; i < number_of_files; i++) {
            free(file_names[i]);
        }
        free(file_names);

        free(length);
        free(final_freq_records);

        // Close the shared memory segment
        munmap(shm_ptr, SM_SIZE);
        close(shm_fd);
        shm_unlink(SM_NAME);

        exit(0);
    }
}