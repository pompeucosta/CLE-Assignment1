// shrdArea.h
#ifndef SHRDAREA_H
#define SHRDAREA_H

#include <pthread.h>

/**
 * @brief Structure representing a shared area for distributing work among threads.
 *
 * Contains all necessary information for managing a shared data space used in
 * multithreaded processing, including synchronization mechanisms, worker
 * management, and data partitioning details.
 */
typedef struct {
    int *data;                  ///< Pointer to the array of integers.
    int size;                   ///< Total number of integers in the array.
    char *fileName;             ///< Name of the file from which data is read.
    int numWorkers;             ///< Number of worker threads processing the data.
    int *startIndices;          ///< Array of start indices for each worker's sub-sequence.
    int *lengths;               ///< Array of lengths for each worker's sub-sequence.
    int completedWorkers;       ///< Counter for the number of workers that have completed their tasks.
    pthread_mutex_t mutex;      ///< Mutex for protecting shared data and coordinating worker threads.
    pthread_cond_t condVar;     ///< Condition variable for signaling between threads.
    pthread_cond_t allDone;     ///< Condition variable used to signal distributor when all workers are done.
    int ready;                  ///< Flag indicating readiness for workers to begin processing.
    int exitFlag;               ///< Flag for signaling all threads to exit.
    int *sortOrder;             ///< Array indicating the order in which workers should process data.
} SharedArea;

/**
 * @brief Initializes the shared area used for distributing work among threads.
 *
 * This function allocates memory for the shared data array, start indices, lengths,
 * and sort order for each worker. It also initializes synchronization primitives
 * (mutex and condition variables) used to coordinate access to the shared data.
 *
 * @param sa Pointer to the SharedArea structure to be initialized.
 * @param size The total number of integers that will be stored in the shared area.
 * @param numWorkers The number of worker threads that will process the data.
 */
void shrdArea_init(SharedArea *sa, int size, int numWorkers);


/**
 * @brief Destroys the shared area and frees all allocated resources.
 *
 * This function cleans up the shared area by destroying the synchronization primitives
 * and freeing all dynamically allocated memory. It ensures that all resources allocated
 * during the initialization are properly released to prevent memory leaks.
 *
 * @param sa Pointer to the SharedArea structure to be destroyed.
 */
void shrdArea_destroy(SharedArea *sa);

#endif
