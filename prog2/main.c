#include <stdio.h>
#include <stdlib.h>
#include "shrdArea.h"
#include "monitor.h"
#include <pthread.h>
#include "Tdistributor.h"
#include "log/logger.h"

/**
* \brief Get the process time that has elapsed since last call of this time.
*
* \return process elapsed time
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

int main(int argc, char **argv)
{
    log_message(LOG_INFO, "Application started");
    double time = get_delta_time();
    if (argc < 2)
    {
        log_message(LOG_ERROR, "Wrong Usage");
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Open the file
    FILE *filePtr = fopen(argv[1], "rb");
    log_message(LOG_INFO, "Attempting to open file: %s", argv[1]);
    
    if (filePtr == NULL) {
        log_message(LOG_ERROR, "Error opening file: %s", argv[1]);
        return EXIT_FAILURE;
    } else {
        log_message(LOG_INFO, "File opened successfully: %s", argv[1]);
    }


    // Find out the size of the file
    fseek(filePtr, 0, SEEK_END);
    long fileSize = ftell(filePtr);
    fseek(filePtr, 0, SEEK_SET);

    //TODO: Change this, number of its is all - the first (first is the total number of ints)
    // After determining fileSize
    log_message(LOG_DEBUG, "File size determined: %ld bytes", fileSize);
    int numberOfInts = fileSize / sizeof(int);

    log_message(LOG_INFO, "Initializing shared area with %d integers", numberOfInts);
    // Initialize the shared area
    SharedArea shrdArea;
    shrdArea_init(&shrdArea, numberOfInts);

    // Read the integers from the file into the shared area data
    if (fread(shrdArea.data, sizeof(int), numberOfInts, filePtr) != numberOfInts) {
        perror("Failed to read the entire file");
        shrdArea_destroy(&shrdArea);
        fclose(filePtr);
        return EXIT_FAILURE;
    }

    // Close the file
    fclose(filePtr);

    Monitor monitor;
    monitor_init(&monitor);

    //TODO: Add Creating of worker threads (worker[n] threads) n = 0, ... N-1
    // Create thread
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

    log_message(LOG_DEBUG, "Waiting for Tdistributor thread to complete");
    // Wait for the thread to finish
    pthread_join(tdistributorThread, NULL);
    log_message(LOG_INFO, "Tdistributor thread has completed");

    // Clean up resources
    shrdArea_destroy(&shrdArea);
    log_message(LOG_INFO, "Application finished successfully");

    time = get_delta_time();
    printf("Time - %f\n", time);
    
    return EXIT_SUCCESS;
}

