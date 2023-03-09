#ifndef __FREQ_TABLE_H__
#define __FREQ_TABLE_H__

#include "common_defs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char word[MAX_WORD_LENGTH];
    int frequency;
} FreqRecord;

typedef struct {
    FreqRecord* records;
    int capacity;
    int size;
} FreqTable;

FreqRecord* new_freq_record(char* word);
FreqTable* new_freq_table(int capacity);
FreqTable* new_freq_table_from_freq_records(FreqRecord* records, int size);
void merge_duplicate_freq_records(FreqTable* ft);
void free_freq_table(FreqTable* ft);
FreqRecord* get_freq_record(FreqTable* ft, int index);
void set_capacity(FreqTable* ft, int capacity);
void set_size(FreqTable* ft, int size);
void add_freq_record(FreqTable* ft, FreqRecord* fr);
void delete_freq_record(FreqTable* ft, int index);
int check_word(FreqTable* ft, char* word);
void print_freq_table(FreqTable* ft);

// Heap functions
void heapify(FreqTable* ft, int size, int index);
void heap_sort(FreqTable* ft, int size);
void swap_freq_record(FreqTable* ft, int index1, int index2);
void copy_word_to_word(char* word1, char* word2, int word2_size);

#endif // __FREQ_TABLE_H__
