#include "Tworker.h"
#include "shrdArea.h"
#include "log/logger.h"
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>


void sort(int *array, int start, int length, bool ascending) {
    int end = start + length;
    for (int i = start; i < end - 1; ++i) {
        for (int j = start; j < end - i - 1; ++j) {
            if (ascending ? array[j] > array[j + 1] : array[j] < array[j + 1]) {
                // Swap elements
                int temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
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

        bool ascending = shrdArea->sortOrder[workerIndex];

        shrdArea->ready--;
        
        int startIndex = shrdArea->startIndices[workerIndex];
        int length = shrdArea->lengths[workerIndex];

        pthread_mutex_unlock(&shrdArea->mutex);

        log_message(LOG_INFO, "Worker thread %d starting bitonic sort on sub-sequence from index %d, length %d", workerIndex, startIndex, length);
        sort(shrdArea->data, startIndex, length, ascending);

        pthread_mutex_lock(&shrdArea->mutex);
        shrdArea->completedWorkers++;
        if (shrdArea->completedWorkers == shrdArea->numWorkers) {
            pthread_cond_broadcast(&shrdArea->allDone); // Notify Tdistributor that all workers are done
        }
        pthread_mutex_unlock(&shrdArea->mutex);

        log_message(LOG_INFO, "Worker thread %d completed sort on sub-sequence", workerIndex);
    } while (true);

    return NULL;
}