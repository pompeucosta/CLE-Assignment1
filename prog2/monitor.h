// monitor.h
#ifndef MONITOR_H
#define MONITOR_H

#include <pthread.h>

typedef struct {
    pthread_mutex_t lock; // Synchronization primitive
    // Add more fields as needed
} Monitor;

void monitor_init(Monitor *m);
void monitor_enter(Monitor *m);
void monitor_exit(Monitor *m);

#endif
