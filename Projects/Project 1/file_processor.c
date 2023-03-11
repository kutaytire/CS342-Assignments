#include "file_processor.h"

FreqTable* get_file_word_freq_table(char* file_name) {
    // Open the file
    FILE* file = fopen(file_name, "r");
    if (file == NULL) {
        fprintf(stderr, "File could not be opened!");
        exit(-1);
    }

    // Create a frequency table
    FreqTable* freq_table = new_freq_table(1);

    char current_word[MAX_WORD_LENGTH];
    char c;
    int i = 0;

    while (i < sizeof(current_word)) {
        c = fgetc(file);
        if (c == EOF) {
            if (i != 0) {
                current_word[i++] = '\0';
                FreqRecord* fr = new_freq_record(current_word);
                add_freq_record(freq_table, fr);
                free(fr);
            }

            break;
        }

        if (c == '\t' || c == ' ' || c == '\n') {
            if (i == 0) {
                continue;
            }

            else {
                current_word[i] = '\0';
                FreqRecord* fr = new_freq_record(current_word);
                add_freq_record(freq_table, fr);
                free(fr);
                // Reset the current word
                i = 0;
                strcpy(current_word, "");
            }
        }

        else {
            current_word[i++] = toupper(c);
        }
    }

    fclose(file);
    return freq_table;
}

FreqRecord* find_most_k_freq_words_from_file(char* file_name, int k, int* new_k, int force_fill) {
    FreqTable* freq_table = get_file_word_freq_table(file_name);
    heap_sort(freq_table, freq_table->size); // Sort the frequency table

    // + Can we assume an input file contains at least K words?
    // - We can not assume that. The result may be less than K words.
    int s;
    if (k > freq_table->size) {
        s = freq_table->size;
    }

    else {
        s = k;
    }

    // If force_fill is true, we will return an array of size K
    // We do this in order to maintain memory alignment when we use POSIX shared memory
    FreqRecord* most_k_freq_words = malloc(sizeof(FreqRecord) * (force_fill ? k : s));
    for (int i = 0; i < s; i++) {
        most_k_freq_words[i].frequency = freq_table->records[i].frequency;
        copy_word_to_word(most_k_freq_words[i].word, freq_table->records[i].word,
                          strlen(freq_table->records[i].word));
    }

    // If force_fill is true, we will fill the rest of the array with invalid values
    if (force_fill) {
        for (int i = s; i < k; i++) {
            most_k_freq_words[i].frequency = -1;
            copy_word_to_word(most_k_freq_words[i].word, "", 0);
        }
    }

    free_freq_table(freq_table);

    if (new_k != NULL) {
        *new_k = s;
    }

    return most_k_freq_words;
}

FreqRecord* find_most_k_freq_words_from_freq_table(FreqTable* ft, int k, int* new_k) {
    heap_sort(ft, ft->size); // Sort the frequency table

    // + Can we assume an input file contains at least K words?
    // - We can not assume that. The result may be less than K words.
    int s;
    if (k > ft->size) {
        s = ft->size;
    }

    else {
        s = k;
    }

    FreqRecord* most_k_freq_words = malloc(sizeof(FreqRecord) * s);
    for (int i = 0; i < s; i++) {
        most_k_freq_words[i].frequency = ft->records[i].frequency;
        copy_word_to_word(most_k_freq_words[i].word, ft->records[i].word,
                          strlen(ft->records[i].word));
    }

    if (new_k != NULL) {
        *new_k = s;
    }

    return most_k_freq_words;
}

void print_to_file(char* file_name, FreqRecord* fr, int size) {
    FILE* file = fopen(file_name, "w+");

    if (file == NULL) {
        fprintf(stderr, "%s\n", "File could not be opened!");
        exit(-1);
    }

    for (int i = 0; i < size; i++) {
        if (i == size - 1) {
            fprintf(file, "%s %d", fr[i].word, fr[i].frequency);
        } else {
            fprintf(file, "%s %d\n", fr[i].word, fr[i].frequency);
        }
    }

    fclose(file);
}
