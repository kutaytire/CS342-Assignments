#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define MAX_LINE_LENGTH 1024
#define PAGE_SIZE 4096 //given in prompt

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define NUM_FLAGS 26

// These flags are from: https://www.kernel.org/doc/Documentation/vm/pagemap.txt
const char* flags[NUM_FLAGS] = {
        "LOCKED", "ERROR", "REFERENCED", "UPTODATE", "DIRTY", "LRU",
        "ACTIVE", "SLAB", "WRITEBACK", "RECLAIM", "BUDDY", "MMAP",
        "ANON", "SWAPCACHE", "SWAPBACKED", "COMPOUND_HEAD",
        "COMPOUND_TAIL", "HUGE", "UNEVICTABLE", "HWPOISON",
        "NOPAGE", "KSM", "THP", "BALLOON", "ZERO_PAGE", "IDLE"
};

// QUESTION 2
void print_frameinfo(unsigned long long frame_number) {
    unsigned long setOfFlags;
    off_t index = frame_number * sizeof(unsigned long); // same as previous method's

    FILE* kpageflags_fp = fopen("/proc/kpageflags", "r");
    if (kpageflags_fp == NULL) {
        fprintf(stderr, "Failed to open /proc/kpageflags\n");
        return;
    }
    // Seek to that index, entry
    if (fseek(kpageflags_fp, index, SEEK_SET) == -1) {// = 0 on success, -1 on failure
        printf("Failed to seek kpageflags entry:%jd for frame number:%llx\n", (intmax_t)index, frame_number);
        if (kpageflags_fp != NULL) {
            fclose(kpageflags_fp);
        }
        return;
    }

    // Read from that entry
    if (fread(&setOfFlags, sizeof(unsigned long), 1, kpageflags_fp) != 1) {
        if(feof(kpageflags_fp)){//END OF FILE reached
            //continue
        }
        else{
            printf("Failed to seek kpageflags entry:%jd for frame number:%llx\n", (intmax_t)index, frame_number);
            if (kpageflags_fp != NULL) {
                fclose(kpageflags_fp);
            }
            return;
        }
    }

    // /kpagecount part
    // Read from kpagecount
    unsigned long mapping_count;
    FILE* kpagecount_fp = fopen("/proc/kpagecount", "r");
    if (kpagecount_fp == NULL) {
        fprintf(stderr, "Failed to open /proc/kpagecount\n");
        return;
    }

    // Seek to the mapping count entry
    if (fseek(kpagecount_fp, index, SEEK_SET) == -1) {
        printf("Failed to seek kpagecount entry:%jd for frame number:%llx\n", (intmax_t)index, frame_number);
        if (kpagecount_fp != NULL) {
            fclose(kpagecount_fp);
        }
        return;
    }

    // Read the mapping count
    if (fread(&mapping_count, sizeof(unsigned long), 1, kpagecount_fp) != 1) {
        if (feof(kpagecount_fp)) {//END OF FILE reached
            //continue
        }
        else {
            printf("Failed to read kpagecount entry:%jd for frame number:%llx\n", (intmax_t)index, frame_number);
            if (kpageflags_fp != NULL) {
                fclose(kpageflags_fp);
            }
            return;
        }
    }

    if (kpagecount_fp != NULL) {
        fclose(kpagecount_fp);
    }
    printf("Frame Number (PFN): 0x%09llx\n", frame_number);
    printf("Flags in Hex 0x%lx\n", setOfFlags);
    for (int i = 0; i < NUM_FLAGS; i++) {
        printf("Flag %2d %s: %d\n", i, flags[i], (setOfFlags & (1UL << i)) != 0);
    }

    printf("Mapping Count: %lu\n", mapping_count);

    if (kpageflags_fp != NULL) {
        fclose(kpageflags_fp);
    }
}

// QUESTION 3
void calculate_memory_usage(int pid) {
    FILE* maps_fp;
    FILE* kpagecount_fp;
    FILE* pagemap_fp;
    unsigned long start = 0, end = 0;
    unsigned long pagemap_entry = 0;
    char maps_path[MAX_LINE_LENGTH];
    char pagemap_path[MAX_LINE_LENGTH];
    char kpagecount_path[MAX_LINE_LENGTH];
    unsigned long virtual_memory = 0;
    unsigned long exclusive_memory = 0;
    unsigned long all_memory = 0;

    // Open files
    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);
    snprintf(pagemap_path, sizeof(pagemap_path), "/proc/%d/pagemap", pid);
    snprintf(kpagecount_path, sizeof(kpagecount_path), "/proc/kpagecount");

    maps_fp = fopen(maps_path, "r");
    if (maps_fp == NULL) {
        fprintf(stderr, "Failed to open %s\n", maps_path);
        return;
    }

    pagemap_fp = fopen(pagemap_path, "rb");
    if (pagemap_fp == NULL) {
        fprintf(stderr, "Failed to open %s\n", pagemap_path);
        if (maps_fp != NULL) {
            fclose(maps_fp);
        }
        return;
    }

    kpagecount_fp = fopen(kpagecount_path, "r");
    if (kpagecount_fp == NULL) {
        fprintf(stderr, "Failed to open %s\n", kpagecount_path);
        if (maps_fp != NULL) {
            fclose(maps_fp);
        }
        if (pagemap_fp != NULL) {
            fclose(pagemap_fp);
        }
        return;
    }

    char line[MAX_LINE_LENGTH];
    // Read the virtual memory areas from the maps file
    while (fgets(line,sizeof(line),maps_fp) != NULL) {
        if (sscanf(line, "%lx-%lx", &start, &end) == 2) {

            unsigned long num_pages = (end - start) / PAGE_SIZE; // end is not included )
            virtual_memory += (end - start) / 1024; // convert here to KB


            // Iterate over each page within the range
            for (unsigned long i = 0; i < num_pages; i++) {
                // Calculate the address of the current page
                unsigned long page_address = start + (i * PAGE_SIZE);

                // Calculate the index using the page address
                unsigned long pagemap_index = (page_address / PAGE_SIZE) * sizeof(unsigned long);

                // Seek to the corresponding entry in the pagemap file
                if (fseek(pagemap_fp, pagemap_index, SEEK_SET) != 0) {
                    printf("Failed to seek pagemap entry for address %lx\n", page_address);
                    continue;
                }


                // Read the pagemap entry for the page
                if (fread(&pagemap_entry, sizeof(unsigned long), 1, pagemap_fp) != 1) {
                    if(feof(pagemap_fp)){
                        // if we read the last line, still we have same value and we need to process it
                    }
                    else{
                        printf("Failed to read pagemap entry for address %lx\n", page_address);
                        continue;
                    }
                }

                // Check if the page is mapped and valid
                if ((pagemap_entry & (1UL << 63)) != 0) {
                    // Page is mapped and valid
                    unsigned long frame_number = pagemap_entry & ((1UL << 55) - 1); // Bits 0-54  page frame number (PFN) if present

                    // Calculate the index using the frame number
                    off_t kpagecount_index = frame_number * sizeof(unsigned long);

                    // Seek to the corresponding entry in the kpagecount file
                    if (fseek(kpagecount_fp, kpagecount_index, SEEK_SET) == -1) {
                        printf("Failed to seek kpagecount entry for frame number\n");
                        continue;
                    }

                    // Read the mapping count from kpagecount
                    unsigned long mapping_count;
                    if (fread(&mapping_count, sizeof(unsigned long), 1, kpagecount_fp) != 1) {
                       if(feof(kpagecount_fp)){
//                        printf("END OF FILE");
//                            continue;//TODO may be better without cont.
                        }
                        printf("Failed to read kpagecount entry for frame number\n");
                        continue;
                    }
//                printf("%d:%d  2:%d\n",i,pagemap_index,kpagecount_index);
                    if (mapping_count == 0) { // just incase
                        // Handle unmapped pages (if necessary)
                    } else if (mapping_count == 1) {
                        // Exclusively mapped page
                        exclusive_memory++;
                    }
                    // All pages with mapping count >= 1
                    all_memory++;
                }
            }
        }


    }

    // Close the files
    if (maps_fp != NULL) {
        fclose(maps_fp);
    }
    if (pagemap_fp != NULL) {
        fclose(pagemap_fp);
    }
    if (kpagecount_fp != NULL) {
        fclose(kpagecount_fp);
    }

    // Print the memory usage in kilobytes
    printf("(pid=%d) memused:\n", pid);
    printf("virtual=%lu KB\n", virtual_memory);
    printf("pmemAll=%lu KB\n", all_memory * 4);
    printf("pmem_alone (Exclusive)=%lu KB\n", exclusive_memory * 4);
    printf("mapped_once=%lu KB\n", exclusive_memory * 4);
}

void create_vmalists(int pid, unsigned long** starts, unsigned long** ends, int* size) {
    FILE* maps_fp;
    char maps_path[MAX_LINE_LENGTH];
    unsigned long start = 0, end = 0;

    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);

    maps_fp = fopen(maps_path, "r");
    if (maps_fp == NULL) {
        fprintf(stderr, "Failed to open %s\n", maps_path);
        return;
    }


    char line[MAX_LINE_LENGTH];
    *size = 0;
    while (fgets(line, sizeof(line), maps_fp) != NULL) {
        if (sscanf(line, "%lx-%lx", &start, &end) == 2) {
            // Reallocate memory for the start and end addresses array
            *(starts) = realloc(*(starts), (*size + 1) * sizeof(unsigned long));
            *(ends) = realloc(*(ends), (*size + 1) * sizeof(unsigned long));
            if (*(starts) == NULL || *(ends) == NULL) {
                fprintf(stderr, "Failed to allocate memory\n");
                if (maps_fp) {
                    fclose(maps_fp);
                }
                return;
            }

            // Store the start address in the array
            unsigned long* starts_array = *(starts);
            unsigned long* ends_array = *(ends);

            starts_array[*size] = start;
            ends_array[*size] = end;
            (*size)++;
        }
    }

    if (maps_fp) {
        fclose(maps_fp);
    }
}

uint64_t get_framenum_for_page(uint64_t va1, int fd_pagemap) {
    off_t offset = va1 * 8;
    uint64_t frame_num;

    if (lseek(fd_pagemap, offset, SEEK_SET) == -1) {
        // Handle lseek error
        return 0;
    }

    if (read(fd_pagemap, &frame_num, sizeof(frame_num)) != sizeof(frame_num)) {
        // Handle read error
        return 0;
    }

    frame_num = frame_num & 0x7FFFFFFFFFFFFFULL;
    return frame_num;
}

//Question 4
void get_physical_address(pid_t pid, uintptr_t virtual_address) {
    // Calculate the virtual page number and page offset
    uintptr_t virtual_page_number = virtual_address / PAGE_SIZE;
    uintptr_t page_offset = virtual_address % PAGE_SIZE;

    unsigned long* starts = NULL;
    unsigned long* ends = NULL;
    int size = 0;

    create_vmalists(pid, &starts, &ends, &size);

    // Open the /proc/[PID]/pagemap file
    char pagemap_path[64];
    sprintf(pagemap_path, "/proc/%d/pagemap", pid);
    int pagemap_fd = open(pagemap_path, O_RDONLY);
    if (pagemap_fd == -1) {
        perror("Failed to open pagemap file");
        exit(EXIT_FAILURE);
    }

    // Seek to the entry corresponding to the virtual page
    off_t pagemap_offset = virtual_page_number * sizeof(uint64_t);
    if (lseek(pagemap_fd, pagemap_offset, SEEK_SET) == -1) {
        perror("Failed to seek in pagemap file");
        if (pagemap_fd != -1) {
            close(pagemap_fd);
        }
        exit(EXIT_FAILURE);
    }

    // Read the entry from pagemap file
    uint64_t pagemap_entry;
    if (read(pagemap_fd, &pagemap_entry, sizeof(uint64_t)) != sizeof(uint64_t)) {
        perror("Failed to read from pagemap file");
        if (pagemap_fd != -1) {
            close(pagemap_fd);
        }
        exit(EXIT_FAILURE);
    }

    if (pagemap_fd != -1) {
        close(pagemap_fd);
    }

    // Extract the frame number from the pagemap entry
    uint64_t frame_number = pagemap_entry & ((1ULL << 54) - 1);
    if (!(pagemap_entry & (1ULL << 63))) {
        // Page not present in memory
        printf("Virtual Address: 0x%012lx\n", (unsigned long)virtual_address);
        printf("Physical Frame Number: 0x%09lx\n", (unsigned long)frame_number);


        int flag = 0;
        // For VA, check if it is between start_adresses and end_address
        for (int i = 0; i < size; i++) {
            if (virtual_address >= starts[i] && virtual_address < ends[i]) {
                flag = 1;

                printf("Virtual Page Number=0x%012lx not-in-memory\n", (unsigned long)virtual_page_number);
            }
        }

        if (!flag) {
            printf("Virtual Page Number=0x%012lx unused\n", (unsigned long)virtual_page_number);
        }

        exit(EXIT_FAILURE);
    }

    // Calculate the physical address
    uintptr_t physical_address = (frame_number * PAGE_SIZE) | page_offset;

    printf("Virtual Address: 0x%016lx\n", (unsigned long)virtual_address);
    printf("Physical Address: 0x%016lx\n", (unsigned long)physical_address);

    if (starts != NULL) {
        free(starts);
    }
    if (ends != NULL) {
        free(ends);
    }
}

//Question 5
void print_page_info(pid_t pid, uintptr_t virtual_address) {
    // Calculate the virtual page number
    uintptr_t virtual_page_number = virtual_address / PAGE_SIZE;

    unsigned long* starts = NULL;
    unsigned long* ends = NULL;
    int size = 0;

    create_vmalists(pid, &starts, &ends, &size);

    // Open the /proc/[PID]/pagemap file
    char pagemap_path[64];
    sprintf(pagemap_path, "/proc/%d/pagemap", pid);
    int pagemap_fd = open(pagemap_path, O_RDONLY);
    if (pagemap_fd == -1) {
        perror("Failed to open pagemap file");
        exit(EXIT_FAILURE);
    }

    // Seek to the entry corresponding to the virtual page
    off_t pagemap_offset = virtual_page_number * sizeof(uint64_t);
    if (lseek(pagemap_fd, pagemap_offset, SEEK_SET) == -1) {
        perror("Failed to seek in pagemap file");
        if (pagemap_fd != -1) {
            close(pagemap_fd);
        }
        exit(EXIT_FAILURE);
    }

    // Read the entry from pagemap file
    uint64_t pagemap_entry;
    if (read(pagemap_fd, &pagemap_entry, sizeof(uint64_t)) != sizeof(uint64_t)) {
        perror("Failed to read from pagemap file");
        if (pagemap_fd != -1) {
            close(pagemap_fd);
        }
        exit(EXIT_FAILURE);
    }

    if (pagemap_fd != -1) {
        close(pagemap_fd);
    }

    // Extract the information from the pagemap entry
    uint64_t pfn = pagemap_entry & ((1ULL << 55) - 1);
    uint64_t swap_type = (pagemap_entry >> 5) & ((1ULL << 5) - 1);
    uint64_t swap_offset = (pagemap_entry >> 5) & ((1ULL << 50) - 1);
    uint64_t soft_dirty = (pagemap_entry >> 55) & 1;
    uint64_t exclusively_mapped = (pagemap_entry >> 56) & 1;
    uint64_t file_page_shared_anon = (pagemap_entry >> 61) & 1;
    uint64_t swapped = (pagemap_entry >> 62) & 1;
    uint64_t present = (pagemap_entry >> 63) & 1;

    // Print the information
    printf("Virtual Address: 0x%12lx\n", virtual_address);
    printf("Virtual Page Number: 0x%09lx\n", virtual_page_number);
    printf("Physical Frame Number: 0x%09lx\n", pfn);
    if( swapped == 1){
        printf("Swap Type: 0x%lx\n", swap_type);
        printf("Swap Offset: 0x%lx\n", swap_offset);
    }

    printf("Present: %lu\n", present);
    printf("Swapped: %lu\n", swapped);
    printf("File-Anon: %lu\n", file_page_shared_anon);
    printf("Exclusive: %lu\n", exclusively_mapped);
    printf("Soft Dirty: %lu\n", soft_dirty);

    if (!(pagemap_entry & (1ULL << 63))) {

        int flag = 0;
        // For VA, check if it is between start_adresses and end_address
        for (int i = 0; i < size; i++) {
            if (virtual_address >= starts[i] && virtual_address < ends[i]) {
                flag = 1;

                printf("Virtual Page Number not-in-memory\n");
            }
        }

        if (!flag) {
            printf("Virtual Page Number unused\n");
        }
    }

    if (starts != NULL) {
        free(starts);
    }
    if (ends != NULL) {
        free(ends);
    }
}

void calculate_all_table_size(int pid) {
    int num_of_fourth_level = 1;
    int num_of_third_level = 1;
    int num_of_second_level = 1;
    int NUM_OF_TOP_LEVEL = 1;

    unsigned long block_size_one = 0x200000; // 2MB
    unsigned long block_size_two = 0x40000000; // 1GB
    unsigned long block_size_three = 0x8000000000; // 512GB

    unsigned long *start_addresses_hex = NULL;
    unsigned long *end_addresses_hex = NULL;
    int size = 0;

    create_vmalists(pid, &start_addresses_hex, &end_addresses_hex, &size);

    // Find the number of 2nd,3rd,4th level page table entries
    for(int i = 0; i <= size - 1; i++) {

        num_of_fourth_level += ((end_addresses_hex[i] - 1) / block_size_one) - (start_addresses_hex[i] / block_size_one); // Finds how many blocks are passed for one segment

        // Find whether to allocate a new table for the new segment
        if(i < size - 1) {
            if((start_addresses_hex[i + 1] / block_size_one) - ((end_addresses_hex[i] - 1) / block_size_one) != 0) {
                num_of_fourth_level++;
            }
        }

        num_of_third_level += ((end_addresses_hex[i] - 1) / block_size_two) - (start_addresses_hex[i] / block_size_two); // Finds how many blocks are passed for one segment

        // Find whether to allocate a new table for the new segment
        if(i < size - 1) {
            if((start_addresses_hex[i + 1] / block_size_two) - ((end_addresses_hex[i] - 1) / block_size_two) != 0) {
                num_of_third_level++;
            }
        }

        num_of_second_level += ((end_addresses_hex[i] - 1) / block_size_three) - (start_addresses_hex[i] / block_size_three); // Finds how many blocks are passed for one segment

        // Find whether to allocate a new table for the new segment
        if(i < size - 1) {
            if((start_addresses_hex[i + 1] / block_size_three) - ((end_addresses_hex[i] - 1) / block_size_three) != 0) {
                num_of_second_level++;
            }
        }
    }

    if (size == 0) {
        num_of_fourth_level = 0;
        num_of_third_level = 0;
        num_of_second_level = 0;
    }

    printf("(pid=%d) total memory occupied by 4-level page table: %d KB (%d frames)\n", pid, (num_of_fourth_level + num_of_third_level + num_of_second_level + NUM_OF_TOP_LEVEL) * 4, num_of_fourth_level + num_of_third_level + num_of_second_level + NUM_OF_TOP_LEVEL);

    printf("(pid=%d) number of page tables used: level1=%d, level2=%d, level3=%d, level4=%d\n", pid, NUM_OF_TOP_LEVEL, num_of_second_level, num_of_third_level, num_of_fourth_level);

    if (start_addresses_hex != NULL) {
        free(start_addresses_hex);
    }
    if (end_addresses_hex != NULL) {
        free(end_addresses_hex);
    }
}

uint64_t get_entry_for_page(uint64_t a1, int fd_pagemap) {
    uint64_t a = 0;
    lseek(fd_pagemap, 8 * a1, 0);
    read(fd_pagemap, &a, 8uLL);
    return a;
}

// QUESTION 6
int map_range(int pid, unsigned long va1, unsigned long va2){
    // Call print_page_info for each va in the range
    uintptr_t va = 0;

    unsigned long *start_addresses_hex = NULL;
    unsigned long *end_addresses_hex = NULL;
    int size = 0;

    create_vmalists(pid, &start_addresses_hex, &end_addresses_hex, &size);

    // shift 12 every element of start_addresses and end_addresses
    for (int i = 0; i < size; i++) {
        start_addresses_hex[i] = start_addresses_hex[i] >> 12;
        end_addresses_hex[i] = end_addresses_hex[i] >> 12;
    }

    // Open the /proc/[PID]/pagemap file
    char pagemap_path[MAX_LINE_LENGTH];
    sprintf(pagemap_path, "/proc/%d/pagemap", pid);
    int pagemap_fd = open(pagemap_path, O_RDONLY);
    if (pagemap_fd == -1) {
        perror("Failed to open pagemap file");
        exit(EXIT_FAILURE);
    }

    unsigned long shifted_end_va = va2 >> 12;

    for (va = va1 >> 12; va < shifted_end_va; va += 1) {
        // Calculate the virtual page number and page offset
        uintptr_t virtual_page_number = va / PAGE_SIZE;

        // Seek to the entry corresponding to the virtual page
        off_t pagemap_offset = virtual_page_number * sizeof(uint64_t);
        if (lseek(pagemap_fd, pagemap_offset, SEEK_SET) == -1) {
            perror("Failed to seek in pagemap file");
            if (pagemap_fd != -1) {
                close(pagemap_fd);
            }
            exit(EXIT_FAILURE);
        }

        // Read the entry from pagemap file
        uint64_t pagemap_entry;
        if (read(pagemap_fd, &pagemap_entry, sizeof(uint64_t)) != sizeof(uint64_t)) {
            perror("Failed to read from pagemap file");
            if (pagemap_fd != -1) {
                close(pagemap_fd);
            }
            exit(EXIT_FAILURE);
        }

        int flag = 0;
        // For VA, check if it is between start_adresses and end_address
        for (int i = 0; i < size; i++) {
            if (va >= start_addresses_hex[i] && va < end_addresses_hex[i]) {
                flag = 1;

                uint64_t frame_number = get_framenum_for_page(va, pagemap_fd);
                if (frame_number) {
                    // Page present in memory
                    printf("mapping: vpn=0x%012lx pfn=0x%09lx\n", va, frame_number);
                } else {
                    printf("mapping: vpn=0x%012lx not-in-memory\n", va);
                }

            }
        }

        if (!flag) {
            printf("mapping: vpn=0x%012lx unused\n", va);
        }
    }

    if (start_addresses_hex != NULL) {
        free(start_addresses_hex);
    }
    if (end_addresses_hex != NULL) {
        free(end_addresses_hex);
    }

    if (pagemap_fd != -1) {
        close(pagemap_fd);
    }

    return 0;
}

void create_vmalist_file_ext(int pid, char*** file_names) {
    FILE* maps_fp;
    char maps_path[MAX_LINE_LENGTH];

    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);

    maps_fp = fopen(maps_path, "r");
    if (maps_fp == NULL) {
        fprintf(stderr, "Failed to open %s\n", maps_path);
        return;
    }

    char line[MAX_LINE_LENGTH];
    uint64_t size = 0;

    char* file_name_pointer = NULL;
    unsigned long a,b;
    char c,d,e,f;

    while (fgets(line, sizeof(line), maps_fp) != NULL) {
        file_name_pointer = malloc(MAX_LINE_LENGTH);
        if (sscanf(line, "%lx-%lx %s %s %s %s %s", &a, &b, &c, &d, &e, &f, file_name_pointer) == 7) {
            // Reallocate memory for the start and end addresses array
            *(file_names) = realloc(*(file_names), (size + 1) * sizeof(char* ));
            if (*(file_names) == NULL) {
                fprintf(stderr, "Failed to allocate memory\n");
                if (maps_fp != NULL) {
                    fclose(maps_fp);
                }
                return;
            }

            // Store the file name pointer in the array
            (*file_names)[size] = file_name_pointer;
            size = size + 1;
        }

        else {
            *(file_names) = realloc(*(file_names), (size + 1) * sizeof(char* ));

            // Put empty string
            (*file_names)[size] = "";
            size = size + 1;

            if (file_name_pointer != NULL) {
                free(file_name_pointer);
            }
        }
    }
    if (maps_fp != NULL) {
        fclose(maps_fp);
    }
}

// QUESTION 7
int map_all(int pid) {
    unsigned long *start_addresses_hex = NULL;
    unsigned long *end_addresses_hex = NULL;
    char** file_names = NULL;
    int size = 0;

    // Open the /proc/[PID]/pagemap file
    char pagemap_path[MAX_LINE_LENGTH];
    sprintf(pagemap_path, "/proc/%d/pagemap", pid);
    int pagemap_fd = open(pagemap_path, O_RDONLY);
    if (pagemap_fd == -1) {
        perror("Failed to open pagemap file");
        exit(EXIT_FAILURE);
    }

    create_vmalists(pid, &start_addresses_hex, &end_addresses_hex, &size);
    create_vmalist_file_ext(pid, &file_names);

    // shift 12 every element of start_addresses and end_addresses
    for (int i = 0; i < size; i++) {
        start_addresses_hex[i] = start_addresses_hex[i] >> 12;
        end_addresses_hex[i] = end_addresses_hex[i] >> 12;
    }

    for (int i = 0; i < size; i++) {
        unsigned long va1 = start_addresses_hex[i];
        unsigned long va2 = end_addresses_hex[i];

        for (unsigned long j = va1; j < va2; j++) {
            uint64_t frame_number = get_framenum_for_page(j, pagemap_fd);

            if (frame_number) {
                // Page present in memory
                printf("mapping: vpn=0x%09lx pfn=0x%09lx, fname=%s\n", j, frame_number, file_names[i]);
            } else {
                uint64_t entry_of_page = get_entry_for_page(j, pagemap_fd);
                printf("mapping: vpn=0x%09lx not-in-memory, swpd=%d,   fname=%s\n", j, (entry_of_page & 0x4000000000000000LL) != 0, file_names[i]);
            }
        }
    }

    // free the file names array
    for (int i = 0; i < size; i++) {
        if (strcmp(file_names[i], "") == 0) {
            // skip
        }
        else {
            free(file_names[i]);
        }
    }
    if (file_names != NULL) {
        free(file_names);
    }

    if (start_addresses_hex != NULL) {
        free(start_addresses_hex);
    }
    if (end_addresses_hex != NULL) {
        free(end_addresses_hex);
    }

    return 0;
}

// QUESTION 8
int map_all_in(int pid) {
    unsigned long *start_addresses_hex = NULL;
    unsigned long *end_addresses_hex = NULL;
    int size = 0;

    // Open the /proc/[PID]/pagemap file
    char pagemap_path[MAX_LINE_LENGTH];
    sprintf(pagemap_path, "/proc/%d/pagemap", pid);
    int pagemap_fd = open(pagemap_path, O_RDONLY);
    if (pagemap_fd == -1) {
        perror("Failed to open pagemap file");
        exit(EXIT_FAILURE);
    }

    create_vmalists(pid, &start_addresses_hex, &end_addresses_hex, &size);

    // shift 12 every element of start_addresses and end_addresses
    for (int i = 0; i < size; i++) {
        start_addresses_hex[i] = start_addresses_hex[i] >> 12;
        end_addresses_hex[i] = end_addresses_hex[i] >> 12;
    }

    for (int i = 0; i < size; i++) {
        unsigned long va1 = start_addresses_hex[i];
        unsigned long va2 = end_addresses_hex[i];

        for (unsigned long j = va1; j < va2; j++) {
            uint64_t frame_number = get_framenum_for_page(j, pagemap_fd);

            if (frame_number) {
                // Page present in memory
                printf("mapping: vpn=0x%09lx pfn=0x%09lx\n", j, frame_number);
            }
        }
    }

    if (start_addresses_hex != NULL) {
        free(start_addresses_hex);
    }
    if (end_addresses_hex != NULL) {
        free(end_addresses_hex);
    }

    return 0;
}

int main(int argc, char *argv[]) {

    if (argc < 3) {
        printf("Usage: %s <command> <parameters>\n", argv[0]);
        return 1;
    }

    pid_t pid;
    uintptr_t va, va1, va2;
    uint64_t pfn;

    // Parse the command and execute the corresponding function
    if (strcmp(argv[1], "-frameinfo") == 0) {
        if (argc != 3) {
            printf("Invalid number of parameters for command -frameinfo\n");
            return 1;
        }
        if (strncmp(argv[2], "0x", 2) == 0) {
            // Argument is already in hexadecimal
            pfn = strtoull(argv[2], NULL, 16);
        } else {
            // Argument is in decimal
            pfn = strtoull(argv[2], NULL, 10);
        }
        print_frameinfo(pfn);
    } else if (strcmp(argv[1], "-memused") == 0) {
        if (argc != 3) {
            printf("Invalid number of parameters for command -memused\n");
            return 1;
        }
        pid = atoi(argv[2]);
        calculate_memory_usage(pid);
    } else if (strcmp(argv[1], "-mapva") == 0) {
        if (argc != 4) {
            printf("Invalid number of parameters for command -mapva\n");
            return 1;
        }
        pid = atoi(argv[2]);
        va = strtoul(argv[3], NULL, 16);
        get_physical_address(pid, va);
    } else if (strcmp(argv[1], "-pte") == 0) {
        if (argc != 4) {
            printf("Invalid number of parameters for command -pte\n");
            return 1;
        }
        pid = atoi(argv[2]);
        va = strtoul(argv[3], NULL, 16);
        print_page_info(pid, va);
    } else if (strcmp(argv[1], "-maprange") == 0) {
        if (argc != 5) {
            printf("Invalid number of parameters for command -maprange\n");
            return 1;
        }
        pid = atoi(argv[2]);
        va1 = strtoul(argv[3], NULL, 16);
        va2 = strtoul(argv[4], NULL, 16);
        map_range(pid, va1, va2);
    } else if (strcmp(argv[1], "-mapall") == 0) {
        if (argc != 3) {
            printf("Invalid number of parameters for command -mapall\n");
            return 1;
        }
        pid = atoi(argv[2]);
        map_all(pid);
    } else if (strcmp(argv[1], "-mapallin") == 0) {
        if (argc != 3) {
            printf("Invalid number of parameters for command -mapallin\n");
            return 1;
        }
        pid = atoi(argv[2]);
        //TODO
        map_all_in(pid);
    } else if (strcmp(argv[1], "-alltablesize") == 0) {
        if (argc != 3) {
            printf("Invalid number of parameters for command -alltablesize\n");
            return 1;
        }
        pid = atoi(argv[2]);
        //TODO
        calculate_all_table_size(pid);
    } else {
        printf("Invalid command: %s\n", argv[1]);
        return 1;
    }
    return 0;
}