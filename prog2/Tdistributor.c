// Tdistributor.c
#include "Tdistributor.h"
#include "shrdArea.h"
#include "monitor.h"
#include "log/logger.h"
#include "stdio.h"

void* Tdistributor_function(void* arg) {
    printf("Thread distributor is running\n");
    log_message(LOG_INFO, "Tdistributor thread started");
    
    SharedArea *shrdArea = (SharedArea*) arg;
    // Use the shared area and monitor to synchronize access
    
    log_message(LOG_INFO, "Tdistributor thread is exiting");
    return NULL;
}
