#include "shrdArea.h"
#include <stdlib.h> // For dynamic memory allocation
#include "log/logger.h"

// Example function to initialize the shared area
void shrdArea_init(SharedArea *sa, int size) {
    log_message(LOG_DEBUG, "Initializing shared area with size: %d integers", size);
    
    sa->data = (int *)malloc(size * sizeof(int));
    if (sa->data == NULL) {
        log_message(LOG_ERROR, "Failed to allocate memory for shared area");
    } else {
        sa->size = size;
        log_message(LOG_INFO, "Shared area initialized successfully");
    }
}

// Example function to clean up resources used by the shared area
void shrdArea_destroy(SharedArea *sa) {
    log_message(LOG_DEBUG, "Destroying shared area and freeing resources");
    
    free(sa->data);
    sa->data = NULL;
    sa->size = 0;

    log_message(LOG_INFO, "Shared area destroyed and resources freed");
}
