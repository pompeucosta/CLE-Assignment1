// Tdistributor.h
#ifndef TDISTRIBUTOR_H
#define TDISTRIBUTOR_H

#include "shrdArea.h"
#include <stdbool.h>

/**
 * @brief Entry point for the distributor thread in a multithreaded sorting application.
 *
 * This function coordinates the sorting of an array of integers distributed across
 * multiple worker threads. It reads data from a file into a shared area, divides
 * the work among worker threads, and manages the merging process to ensure the data
 * is sorted correctly. It also handles synchronization and signaling with worker
 * threads to manage the sorting process efficiently.
 *
 * @param arg A pointer to a SharedArea structure containing data and synchronization primitives.
 * @return A NULL pointer, indicating successful completion of the thread's task.
 */
void* Tdistributor_function(void* arg);

/**
 * @brief Determines if a given number is a power of two.
 *
 * A number is a power of two if it's not zero and its binary representation
 * has only one '1' bit.
 *
 * @param n The number to check.
 * @return true if n is a power of two, false otherwise.
 */
bool isPowerOfTwo(int n);

/**
 * @brief Merges sub-sequences using the bitonic algorithm into a sorted sequence.
 *
 * This recursive function divides a bitonic sequence into halves, and sorts each
 * half in the given order (ascending or descending), ensuring the entire sequence
 * becomes sorted.
 *
 * @param shrdArea Pointer to the shared area structure containing the data.
 * @param start The starting index of the sub-sequence to be merged.
 * @param count The number of elements in the sub-sequence.
 * @param ascending A boolean indicating the desired order of sorting (true for ascending).
 */
void bitonicMerge(SharedArea *shrdArea, int start, int count, bool ascending);
#endif