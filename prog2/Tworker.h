// Tworker.h
#ifndef TWORKER_H
#define TWORKER_H

#include "shrdArea.h"
#include <stdbool.h>

typedef struct {
    SharedArea *sharedArea;
    int workerIndex;
    bool sortOrder; // true for ascending, false for descending
} WorkerArgs;

void* Tworker_function(void* arg);

#endif