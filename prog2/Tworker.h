#ifndef TWORKER_H
#define TWORKER_H

#include "shrdArea.h"
#include <stdbool.h>

/**
 * @brief Structure to pass arguments to worker threads.
 *
 * Contains all necessary information for a worker thread to perform its task,
 * including a pointer to the shared area (which contains the data to be sorted
 * and synchronization primitives), the index of the worker thread, and the
 * sort order for the data segment assigned to this worker.
 */
typedef struct {
    SharedArea *sharedArea; ///< Pointer to the shared area structure containing data and synchronization mechanisms.
    int workerIndex;        ///< Index of the worker thread, used for logging and accessing thread-specific data.
    bool sortOrder;         ///< Sort order for this worker; true for ascending, false for descending.
} WorkerArgs;

/**
 * @brief The function to be executed by each worker thread.
 *
 * This function is responsible for sorting a segment of data within the shared
 * area. It waits for work to be assigned, performs the sorting based on the
 * specified order, and then signals completion. It utilizes synchronization
 * primitives to safely access shared data and coordinate with other threads.
 *
 * @param arg A void pointer to a WorkerArgs structure containing the worker's
 * specific data and instructions.
 * @return Always returns NULL, indicating the thread has completed its task.
 */
void* Tworker_function(void* arg);

/**
 * @brief Performs a sort on a subsection of an array, in either ascending or descending order.
 *
 * This function iterates over a specified portion of an array, repeatedly swapping adjacent elements
 * if they are in the wrong order, according to the specified sorting direction. This is a simple,
 * yet inefficient, sorting algorithm best suited for small datasets or partially sorted arrays.
 *
 * @param array Pointer to the array to be sorted.
 * @param start The starting index within the array from which sorting begins.
 * @param length The number of elements to be sorted starting from the index.
 * @param ascending A boolean flag indicating the sort direction (true for ascending, false for descending).
 */
void sort(int *array, int start, int length, bool ascending);

#endif