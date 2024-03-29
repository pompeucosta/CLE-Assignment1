#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "shrdArea.h"
#include <pthread.h>
#include "Tdistributor.h"
#include "log/logger.h"
#include "Tworker.h"
#include <stdbool.h>

/**
 * @brief Calculates the elapsed time in seconds since the last call to this function.
 * 
 * Uses static variables to store the last time this function was called and calculates
 * the difference in time between calls. Intended to measure performance or processing
 * durations.
 *
 * @return The elapsed time in seconds as a double.
 */
static double get_delta_time(void)
{
    static struct timespec t0, t1;
    
    t0 = t1;
    if (clock_gettime(CLOCK_MONOTONIC, &t1) != 0)
    { 
        perror("clock_gettime");
        exit(1);
    }
    return (double) (t1.tv_sec - t0.tv_sec) + 
        1.0e-9 * (double) (t1.tv_nsec - t0.tv_nsec);
}

/**
 * @brief Checks if an array of integers is sorted in non-decreasing order.
 *
 * Iterates through the given array to determine if each element is less than
 * or equal to the next element, indicating the array is sorted.
 *
 * @param array Pointer to the first element of the array to be checked.
 * @param size The number of elements in the array.
 * @return true if the array is sorted, false otherwise.
 */
bool isSorted(int *array, int size) {
    for (int i = 0; i < size - 1; i++) {
        if (array[i] > array[i + 1]) {
            return false; 
        }
    }
    return true;
}

/**
 * @brief Main thread
 *
 * @param argc Number of arguments from the command line.
 * @param argv List of arguments from the command line.
 * @return Status of the operation.
 */
int main(int argc, char **argv)
{
    log_message(LOG_INFO, "Application started");
    double time = get_delta_time();
    if (argc < 4) {  
        log_message(LOG_ERROR, "Wrong Usage");
        fprintf(stderr, "Usage: %s <filename> -w <number of worker threads>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Parse command-line arguments for the number of worker threads
    int numThreads = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            numThreads = atoi(argv[i + 1]);
            if (numThreads <= 0) {
                log_message(LOG_ERROR, "Invalid number of threads: %s", argv[i + 1]);
                return EXIT_FAILURE;
            }
            break;
        }
    }

    // Open the file
    FILE *filePtr = fopen(argv[1], "rb");
    log_message(LOG_INFO, "Attempting to open file: %s", argv[1]);
    
    if (filePtr == NULL) {
        log_message(LOG_ERROR, "Error opening file: %s", argv[1]);
        return EXIT_FAILURE;
    }
    log_message(LOG_INFO, "File opened successfully: %s", argv[1]);


    // Read the total number of integers (excluding the first integer)
    int totalInts;
    if (fread(&totalInts, sizeof(int), 1, filePtr) != 1) {
        log_message(LOG_ERROR, "Failed to read the total number of integers from the file: %s", argv[1]);
        fclose(filePtr);
        return EXIT_FAILURE;
    }
    log_message(LOG_INFO, "Total number of integers to read: %d", totalInts);


    log_message(LOG_INFO, "Initializing shared area with %d integers", totalInts);
    // Initialize the shared area
    SharedArea shrdArea;
    shrdArea_init(&shrdArea, totalInts, numThreads);
    shrdArea.fileName = argv[1];

    // Close the file
    fclose(filePtr);

    // Distributor Thread
    pthread_t tdistributorThread;
    log_message(LOG_INFO, "Creating Tdistributor thread");
    if(pthread_create(&tdistributorThread, NULL, Tdistributor_function, (void*)&shrdArea)) {
        log_message(LOG_ERROR, "Failed to create Tdistributor thread");
        perror("Failed to create Tdistributor thread");
        shrdArea_destroy(&shrdArea); 
        return EXIT_FAILURE;
    } else {
        log_message(LOG_INFO, "Tdistributor thread created successfully");
    }

    WorkerArgs* workerArgs = malloc(numThreads * sizeof(WorkerArgs));
    pthread_t* workerThreads = malloc(numThreads * sizeof(pthread_t));

    // Worker Thread
    log_message(LOG_INFO, "Creating %d worker threads", numThreads);
    for (int i = 0; i < numThreads; i++) {
        workerArgs[i].sharedArea = &shrdArea;
        workerArgs[i].workerIndex = i;
        if (pthread_create(&workerThreads[i], NULL, Tworker_function, (void*)&workerArgs[i])) {
            log_message(LOG_ERROR, "Failed to create worker thread %d", i);
        }
    }

    // Wait for all worker threads to complete
    for (int i = 0; i < numThreads; i++) {
        pthread_join(workerThreads[i], NULL);
    }

    log_message(LOG_DEBUG, "Waiting for Tdistributor thread to complete");
    
    // Wait for the thread to finish
    pthread_join(tdistributorThread, NULL);
    log_message(LOG_INFO, "Tdistributor thread has completed");

    if (isSorted(shrdArea.data, shrdArea.size)) {
        printf("The sequence is properly sorted.\n");
    } else {
        printf("The sequence is NOT properly sorted.\n");
    }

    // Clean up resources
    free(workerArgs);
    free(workerThreads);
    shrdArea_destroy(&shrdArea);
    log_message(LOG_INFO, "Application finished successfully");

    time = get_delta_time();
    printf("Time - %f\n", time);
    
    return EXIT_SUCCESS;
}