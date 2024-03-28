// shrdArea.h
#ifndef SHRDAREA_H
#define SHRDAREA_H

#include <pthread.h>

typedef struct {
    int *data;                  // Pointer to the array of integers
    int size;                   // Number of integers in the array
    char *fileName;             // Name of the file to read data from
    int numWorkers;             // Number of worker threads
    int *startIndices;          // Array of start indices for each sub-sequence
    int *lengths;               // Array of lengths for each sub-sequence
    int completedWorkers;       // Tracks the number of workers that have completed their work
    pthread_mutex_t mutex;      // Mutex for synchronization
    pthread_cond_t condVar;     // Condition variable for synchronization
    pthread_cond_t allDone;     // Distributor waits on this until all workers are done
    int ready;                  // Flag to indicate readiness for workers
    int exitFlag;
} SharedArea;

void shrdArea_init(SharedArea *sa, int size, int numWorkers);
void shrdArea_destroy(SharedArea *sa);

#endif
