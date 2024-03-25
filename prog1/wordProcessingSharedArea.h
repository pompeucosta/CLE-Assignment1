#ifndef wordProcessingSharedArea
#define wordProcessingSharedArea

#include <stdint.h>

/// @brief Sets the file's names into the internal data structure for reading
/// @param files The path's of the files
/// @param count The quantity of files
/// @return 0 if it was sucessful and -1 otherwise
int setFiles(char** files,uint16_t count);

/// @brief Fills the provided buffer with a maximum of bufferSize bytes
/// @param buffer The buffer to fill
/// @param bufferSize The size of the buffer
/// @param fileIndex The index of the file the function read from
/// @param newBufferSize The amount of bytes that were inserted into the buffer
/// @return 0 if it was sucessfull, -1 if there are no more bytes to read, any value less than -1 if an error ocurred
int fillBuffer(uint8_t* buffer,uint32_t bufferSize,uint16_t* fileIndex,uint32_t* newBufferSize);

#endif