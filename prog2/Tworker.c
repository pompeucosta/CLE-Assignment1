#include "Tworker.h"
#include "shrdArea.h"
#include "log/logger.h"
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>


void compareAndSwap(int *array, int i, int j, int dir) {
    if (dir == (array[i] > array[j])) {
        // Swap elements
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

void bitonic_merge(int *array, int low, int count, int dir) {
    if (count > 1) {
        int k = count / 2;
        for (int i = low; i < low + k; i++) {
            compareAndSwap(array, i, i + k, dir);
        }
        bitonic_merge(array, low, k, dir);
        bitonic_merge(array, low + k, k, dir);
    }
}

void bitonic_sort_bottom_up(int *array, int low, int count, int dir) {
    for (int k = 2; k <= count; k = 2 * k) {
        for (int j = k >> 1; j > 0; j = j >> 1) {
            for (int i = low; i + j < low + count; i++) {
                if ((i & k) == 0) {
                    if (array[i] > array[i + j]) {
                        // Swap elements
                        int temp = array[i];
                        array[i] = array[i + j];
                        array[i + j] = temp;
                    }
                } else {
                    if (array[i] < array[i + j]) {
                        // Swap elements
                        int temp = array[i];
                        array[i] = array[i + j];
                        array[i + j] = temp;
                    }
                }
            }
        }
    }
}

void* Tworker_function(void* arg) {
    WorkerArgs *workerArgs = (WorkerArgs*) arg;
    SharedArea *shrdArea = workerArgs->sharedArea;
    int workerIndex = workerArgs->workerIndex;

    do {
        log_message(LOG_DEBUG, "Worker thread %d: Checking for new work or exit signal.", workerIndex);
        pthread_mutex_lock(&shrdArea->mutex);

        while (!shrdArea->ready && !shrdArea->exitFlag) {
            pthread_cond_wait(&shrdArea->condVar, &shrdArea->mutex);
        }

        if (shrdArea->exitFlag) {
            pthread_mutex_unlock(&shrdArea->mutex);
            log_message(LOG_INFO, "Worker thread %d: Exit flag set, exiting.", workerIndex);
            break; // Exit loop and terminate thread
        }

        pthread_mutex_unlock(&shrdArea->mutex);
        
        int startIndex = shrdArea->startIndices[workerIndex];
        int length = shrdArea->lengths[workerIndex];

        // Log the sub-sequence before sorting
        log_message(LOG_DEBUG, "Worker thread %d: Sub-sequence before sorting:", workerIndex);
        for (int i = startIndex; i < startIndex + length; i++) {
            log_message(LOG_DEBUG, "Data[%d] = %d", i, shrdArea->data[i]);
        }

        log_message(LOG_INFO, "Worker thread %d starting bitonic sort on sub-sequence from index %d, length %d", workerIndex, startIndex, length);
        bitonic_sort_bottom_up(shrdArea->data, startIndex, length, 1); // 1 for ascending sort

        // Log the sub-sequence after sorting
        log_message(LOG_DEBUG, "Worker thread %d: Sub-sequence after sorting:", workerIndex);
        for (int i = startIndex; i < startIndex + length; i++) {
            log_message(LOG_DEBUG, "Data[%d] = %d", i, shrdArea->data[i]);
        }
        pthread_mutex_lock(&shrdArea->mutex);
        shrdArea->completedWorkers++;
        if (shrdArea->completedWorkers == shrdArea->numWorkers) {
            pthread_cond_broadcast(&shrdArea->allDone); // Notify Tdistributor that all workers are done
        }
        pthread_mutex_unlock(&shrdArea->mutex);

        log_message(LOG_INFO, "Worker thread %d completed bitonic sort on sub-sequence", workerIndex);
    } while (true);

    return NULL;
}