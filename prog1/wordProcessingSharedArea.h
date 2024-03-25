#ifndef wordProcessingSharedArea
#define wordProcessingSharedArea

#include <stdint.h>

int setFiles(char** files,uint16_t count);

int fillBuffer(uint8_t* buffer,uint32_t bufferSize,uint16_t* fileIndex,uint32_t* newBufferSize);

#endif