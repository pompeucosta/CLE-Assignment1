#ifndef wordProcessingSharedArea
#define wordProcessingSharedArea

#include <stdint.h>

/**
 * @brief Sets the file's names into the internal data structure for reading.
 *
 * @param files The paths of the files to be read.
 * @param count The number of files to be read.
 *
 * @return 0 if the operation was successful, -1 otherwise.
 */

int setFiles(char** files,uint16_t count);

/**
 * @brief Fills the provided buffer with a maximum of bufferSize bytes.
 *
 * @param buffer The buffer to fill with data.
 * @param bufferSize The maximum number of bytes to fill in the buffer.
 * @param fileIndex The index of the file from which to read data.
 * @param newBufferSize The number of bytes that were actually inserted into the buffer.
 *
 * @return 0 if the operation was successful, -1 if there are no more bytes to read, any value less than -1 if an error occurred.
 */
int fillBuffer(uint8_t* buffer,uint32_t bufferSize,uint16_t* fileIndex,uint32_t* newBufferSize);

#endif