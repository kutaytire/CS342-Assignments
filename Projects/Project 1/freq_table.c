#include "freq_table.h"
#include <stdlib.h>

FreqRecord* new_freq_record(char* word) {
    FreqRecord* fr = malloc(sizeof(FreqRecord));
    *fr = (FreqRecord) {0};
    copy_word_to_word(fr->word, word, strlen(word));
    fr->frequency = 1;
    return fr;
}

FreqTable* new_freq_table(int capacity) {
    FreqTable* ft = malloc(sizeof(FreqTable));
    ft->records = malloc(sizeof(FreqRecord) * capacity);
    ft->capacity = capacity;
    ft->size = 0;
    return ft;
}

FreqTable* new_freq_table_from_freq_records(FreqRecord* records, int size) {
    FreqTable* ft = new_freq_table(1);
    for (int i = 0; i < size; i++) {
        if (records[i].frequency <= 0 || strlen(records[i].word) == 0) {
            continue;
        }

        else {
            add_freq_record(ft, &records[i]);
        }
    }
    return ft;
}

void merge_duplicate_freq_records(FreqTable* ft)
{
    for (int i = 0; i < ft->size; i++) {
        for (int j = i + 1; j < ft->size; j++) {
            if (strcmp(ft->records[i].word, ft->records[j].word) == 0) {
                ft->records[i].frequency += ft->records[j].frequency;
                delete_freq_record(ft, j);
                j--;
            }
        }
    }
}

void free_freq_table(FreqTable* ft) {
    free(ft->records);
    free(ft);
}

FreqRecord* get_freq_record(FreqTable* ft, int index) { return &ft->records[index]; }

void set_capacity(FreqTable* ft, int capacity) {
    ft->records = realloc(ft->records, sizeof(FreqRecord) * capacity);
    ft->capacity = capacity;
}

void set_size(FreqTable* ft, int size) {
    ft->size = size;
    // If the given size is greater than the capacity, set the capacity to the new size
    if (ft->size > ft->capacity) {
        set_capacity(ft, ft->size);
    }
}

void add_freq_record(FreqTable* ft, FreqRecord* fr) {
    // First check if the word is already in the table
    int index = check_word(ft, fr->word);
    if (index != -1) {
        ft->records[index].frequency += fr->frequency;
        return;
    }

    // Add a new record to the table

    // If the table is full, double the capacity
    if (ft->size + 1 == ft->capacity) {
        set_capacity(ft, ft->capacity * 2);
    }

    copy_word_to_word(ft->records[ft->size].word, fr->word, strlen(fr->word));
    ft->records[ft->size].frequency = fr->frequency;
    ft->size++;
}

void delete_freq_record(FreqTable* ft, int index)
{
    for (int i = index; i < ft->size - 1; i++) {
        ft->records[i] = ft->records[i + 1];
    }
    ft->size--;
}

int check_word(FreqTable* ft, char* word) {
    for (int i = 0; i < ft->size; i++) {
        if (strcmp(ft->records[i].word, word) == 0) {
            return i;
        }
    }
    return -1;
}

void heapify(FreqTable* ft, int size, int index) {
    int min = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;

    if (left < size) {
        if (ft->records[left].frequency < ft->records[min].frequency) {
            min = left;
        }

        else if (ft->records[left].frequency == ft->records[min].frequency) {
            if (strcmp(ft->records[left].word, ft->records[min].word) > 0) {
                min = left;
            }
        }
    }

    if (right < size) {
        if (ft->records[right].frequency < ft->records[min].frequency) {
            min = right;
        }

        else if (ft->records[right].frequency == ft->records[min].frequency) {
            if (strcmp(ft->records[right].word, ft->records[min].word) > 0) {
                min = right;
            }
        }
    }

    if (min != index) {
        swap_freq_record(ft, index, min);

        heapify(ft, size, min);
    }
}

void heap_sort(FreqTable* ft, int size) {
    int index = size / 2 - 1;
    while (index >= 0) {
        heapify(ft, size, index);
        index--;
    }

    index = size - 1;
    while (index >= 0) {
        swap_freq_record(ft, 0, index);
        heapify(ft, index, 0);
        index--;
    }
}

void swap_freq_record(FreqTable* ft, int index1, int index2) {
    FreqRecord temp = {0};
    temp.frequency = ft->records[index1].frequency;
    copy_word_to_word(temp.word, ft->records[index1].word, strlen(ft->records[index1].word));

    ft->records[index1].frequency = ft->records[index2].frequency;
    copy_word_to_word(ft->records[index1].word, ft->records[index2].word,
                      strlen(ft->records[index2].word));

    ft->records[index2].frequency = temp.frequency;
    copy_word_to_word(ft->records[index2].word, temp.word, strlen(temp.word));
}

void copy_word_to_word(char* word1, char* word2, int word2_size) {
    for (int i = 0; i < word2_size; i++) {
        word1[i] = word2[i];
    }
    word1[word2_size] = '\0';
}

void print_freq_table(FreqTable* ft) {
    for (int i = 0; i < ft->size; i++) {
        FreqRecord* fr = get_freq_record(ft, i);
        printf("%s %d\n", fr->word, fr->frequency);
    }
}
