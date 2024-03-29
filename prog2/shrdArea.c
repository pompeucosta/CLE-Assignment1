#include "shrdArea.h"
#include <stdlib.h> 
#include "log/logger.h"

void shrdArea_init(SharedArea *sa, int size, int numWorkers) {
    log_message(LOG_DEBUG, "Initializing shared area with size: %d integers", size);
    
    sa->data = malloc(size * sizeof(int));
    if (sa->data == NULL) {
        log_message(LOG_ERROR, "Failed to allocate memory for shared area");
        return;
    }
    
    sa->size = size;
    sa->numWorkers = numWorkers;
    sa->ready = 0;
    sa->completedWorkers = 0;
    sa->exitFlag = 0;

    pthread_mutex_init(&sa->mutex, NULL);
    pthread_cond_init(&sa->condVar, NULL);
    pthread_cond_init(&sa->allDone, NULL);
    sa->startIndices = malloc(numWorkers * sizeof(int));
    if (sa->startIndices == NULL) {
        log_message(LOG_ERROR, "Failed to allocate memory for start indices");
        free(sa->data);
        return;
    }

    sa->lengths = malloc(numWorkers * sizeof(int));
    if (sa->lengths == NULL) {
        log_message(LOG_ERROR, "Failed to allocate memory for lengths");
        free(sa->data);
        free(sa->startIndices);
        return;
    }

    sa->sortOrder = malloc(numWorkers * sizeof(int));
    if (sa->sortOrder == NULL) {
        log_message(LOG_ERROR, "Failed to allocate memory for sort order");
        // Free previously allocated memory
        if (sa->startIndices != NULL) free(sa->startIndices);
        if (sa->lengths != NULL) free(sa->lengths);
        if (sa->data != NULL) free(sa->data);
        return;
    }

    log_message(LOG_INFO, "Shared area initialized successfully");
}

void shrdArea_destroy(SharedArea *sa) {
    log_message(LOG_DEBUG, "Destroying shared area and freeing resources");
    pthread_mutex_destroy(&sa->mutex);
    pthread_cond_destroy(&sa->condVar);
    pthread_cond_destroy(&sa->allDone);
    
    if (sa->data != NULL) free(sa->data);
    if (sa->startIndices != NULL) free(sa->startIndices);
    if (sa->lengths != NULL) free(sa->lengths);
    if (sa->sortOrder != NULL) free(sa->sortOrder);
    
    log_message(LOG_INFO, "Shared area destroyed and resources freed");
}