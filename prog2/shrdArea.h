// shrdArea.h
#ifndef SHRDAREA_H
#define SHRDAREA_H

typedef struct {
    int *data;      // Pointer to the array of integers
    int size;       // Number of integers in the array
    
} SharedArea;

void shrdArea_init(SharedArea *sa, int size);
void shrdArea_destroy(SharedArea *sa);

#endif
