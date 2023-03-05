#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/fcntl.h>


// Definitions
#define SNAME "shmname"

// Function Declaration
void findKMostWords(char fileName[]);

// Program Start
int main(int argc, char *argv[])
{
    int numberOfFiles = 0;
    int kMostWords = 0;
   

    if (argc > 1) {
        kMostWords = atoi(argv[1]);
        numberOfFiles = atoi(argv[3]);
    }

    // File name table
    int fileNames[numberOfFiles];
    for (int i = 0; i < numberOfFiles; i++){
            fileNames[i] = argv[i + 4];
    }


    // Shared memory is created by the parent process
    const int SIZE = 4096;
    int shm_fd;
    void *ptr;
    shm_fd = shm_open(SNAME, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd,SIZE);
    ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if(ptr == MAP_FAILED) {
        printf("Map failed\n");
        return -1;
    }

    // Create N child processes

    for(int i = 0; i < numberOfFiles; i++) {
        if(fork() == 0) {
            // Find the k-most occuring words
            exit(0);
        }
    }

    

    return 0;
}


void findKMostWords(char fileName[]) {

    
}