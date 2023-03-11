#ifndef __FILE_PROCESSOR_H__
#define __FILE_PROCESSOR_H__

#include "common_defs.h"
#include "freq_table.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

FreqTable* get_file_word_freq_table(char* file_name);
FreqRecord* find_most_k_freq_words_from_file(char* file_name, int k, int* new_k, int force_fill);
FreqRecord* find_most_k_freq_words_from_freq_table(FreqTable* ft, int k, int* new_k);
void print_to_file(char* file_name, FreqRecord* fr, int size);

#endif // __FILE_PROCESSOR_H__
