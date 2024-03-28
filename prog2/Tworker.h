// Tworker.h
#ifndef TWORKER_H
#define TWORKER_H

#include "shrdArea.h" 

typedef struct {
    SharedArea *sharedArea;
    int workerIndex;
} WorkerArgs;

void* Tworker_function(void* arg);

#endif