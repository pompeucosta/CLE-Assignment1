#include "Tdistributor.h"
#include "shrdArea.h"
#include "log/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

bool isGloballySorted(int *data, int size) {
    for (int i = 0; i < size - 1; i++) {
        if (data[i] > data[i + 1]) {
            return false; // Sequence is not fully sorted
        }
    }
    return true; // Sequence is fully sorted
}

void redistributeWork(SharedArea *shrdArea) {
    int subSequenceSize = shrdArea->size / shrdArea->numWorkers;
    for (int i = 0; i < shrdArea->numWorkers; i++) {
        shrdArea->startIndices[i] = i * subSequenceSize;
        shrdArea->lengths[i] = (i == shrdArea->numWorkers - 1) ? 
                               (shrdArea->size - shrdArea->startIndices[i]) :
                               subSequenceSize;
        log_message(LOG_DEBUG, "Redistributed: Worker %d, StartIndex: %d, Length: %d", i, shrdArea->startIndices[i], shrdArea->lengths[i]);
    }
}

void* Tdistributor_function(void* arg) {
    SharedArea *shrdArea = (SharedArea*) arg;
    log_message(LOG_INFO, "Tdistributor thread started. Reading file: %s", shrdArea->fileName);

    FILE *filePtr = fopen(shrdArea->fileName, "rb");
    if (filePtr == NULL) {
        log_message(LOG_ERROR, "Error opening file: %s", shrdArea->fileName);
        return NULL; // Exit the thread if the file cannot be opened
    }

    if (fread(shrdArea->data, sizeof(int), shrdArea->size, filePtr) != shrdArea->size) {
        log_message(LOG_ERROR, "Failed to read the specified number of integers (%d) from the file: %s", shrdArea->size, shrdArea->fileName);
        fclose(filePtr);
        return NULL;
    }
    
    fclose(filePtr);
    log_message(LOG_INFO, "Successfully read %d integers from the file %s into the shared area", shrdArea->size, shrdArea->fileName);

    // Initial distribution of work
    redistributeWork(shrdArea);

    // Signal worker threads that they can start processing
    pthread_mutex_lock(&shrdArea->mutex);
    shrdArea->ready = 1;
    pthread_cond_broadcast(&shrdArea->condVar);
    pthread_mutex_unlock(&shrdArea->mutex);
    log_message(LOG_INFO, "Initial work distributed, and worker threads signaled to start.");

    // Wait for initial sorting to complete
    pthread_mutex_lock(&shrdArea->mutex);
    while (shrdArea->completedWorkers < shrdArea->numWorkers) {
        pthread_cond_wait(&shrdArea->allDone, &shrdArea->mutex);
    }
    pthread_mutex_unlock(&shrdArea->mutex);

    // Redistribution loop
    while (!isGloballySorted(shrdArea->data, shrdArea->size)) {
        log_message(LOG_DEBUG, "Sequence not globally sorted, preparing to redistribute work.");

        // Reset completion flag and ready flag
        pthread_mutex_lock(&shrdArea->mutex);
        shrdArea->completedWorkers = 0;
        shrdArea->ready = 0;
        pthread_mutex_unlock(&shrdArea->mutex);

        // Recalculate work distribution
        redistributeWork(shrdArea);

        // Signal workers for next round of sorting
        pthread_mutex_lock(&shrdArea->mutex);
        shrdArea->ready = 1;
        pthread_cond_broadcast(&shrdArea->condVar);
        pthread_mutex_unlock(&shrdArea->mutex);

        // Wait for this round of sorting to complete
        pthread_mutex_lock(&shrdArea->mutex);
        while (shrdArea->completedWorkers < shrdArea->numWorkers) {
            pthread_cond_wait(&shrdArea->allDone, &shrdArea->mutex);
        }
        pthread_mutex_unlock(&shrdArea->mutex);
    }

    // All data sorted, signal workers to exit
    pthread_mutex_lock(&shrdArea->mutex);
    shrdArea->exitFlag = 1;
    pthread_cond_broadcast(&shrdArea->condVar);
    pthread_mutex_unlock(&shrdArea->mutex);
    log_message(LOG_INFO, "Sequence fully sorted, signaling worker threads to exit.");

    return NULL;
}
