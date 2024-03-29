#include "Tdistributor.h"
#include "shrdArea.h"
#include "log/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

bool isPowerOfTwo(int n) {
    return (n != 0) && ((n & (n - 1)) == 0);
}

void bitonicMerge(SharedArea *shrdArea, int start, int count, bool ascending) {
    if (count < 2) return;

    int k = count / 2;
    for (int i = start; i < start + k; i++) {
        if ((shrdArea->data[i] > shrdArea->data[i + k]) == ascending) {
            // Swap elements if not in correct order
            int temp = shrdArea->data[i];
            shrdArea->data[i] = shrdArea->data[i + k];
            shrdArea->data[i + k] = temp;
        }
    }
    bitonicMerge(shrdArea, start, k, ascending);
    bitonicMerge(shrdArea, start + k, k, ascending);
}

void* Tdistributor_function(void* arg) {
    SharedArea *shrdArea = (SharedArea*) arg;
    int numberOfWorkers = shrdArea->numWorkers;

    log_message(LOG_INFO, "Tdistributor thread started. Reading file: %s", shrdArea->fileName);

    if (!isPowerOfTwo(shrdArea->size)) {
        log_message(LOG_ERROR, "The number of integers (%d) is not a power of two.", shrdArea->size);
        return NULL;
    }

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

    for (int currSize = 2; currSize <= shrdArea->size; currSize *= 2) {
        for (int i = 0; i < shrdArea->size; i += currSize) {
            int subSequenceSize = currSize / numberOfWorkers;

            // Determine sorting directions for sub-sequences
            bool ascending = (i / currSize) % 2 == 0;

            for (int worker = 0; worker < numberOfWorkers; ++worker) {
                int index = i + worker * subSequenceSize;
                bool sortOrder = (worker % 2 == 0) ? ascending : !ascending;

                pthread_mutex_lock(&shrdArea->mutex);
                shrdArea->startIndices[worker] = index;
                shrdArea->lengths[worker] = subSequenceSize;
                shrdArea->sortOrder[worker] = sortOrder ? 1 : 0;
                shrdArea->ready += 1;
                pthread_cond_broadcast(&shrdArea->condVar); // Signal all worker threads.
                pthread_mutex_unlock(&shrdArea->mutex);
            }

            // Wait for sorting to complete
            pthread_mutex_lock(&shrdArea->mutex);
            while (shrdArea->completedWorkers < numberOfWorkers) {
                pthread_cond_wait(&shrdArea->allDone, &shrdArea->mutex);
            }

            bitonicMerge(shrdArea, i, currSize, ascending);
            shrdArea->completedWorkers = 0; // Reset for the next iteration
            shrdArea->ready = 0; // Reset ready status
            pthread_mutex_unlock(&shrdArea->mutex);
        }
    }

    // Signal workers to exit after completion
    pthread_mutex_lock(&shrdArea->mutex);
    shrdArea->exitFlag = 1;
    pthread_cond_broadcast(&shrdArea->condVar);
    pthread_mutex_unlock(&shrdArea->mutex);
    
    return NULL;
}
