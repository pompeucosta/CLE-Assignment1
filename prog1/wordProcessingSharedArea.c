#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include "wordProcessingSharedArea.h"
#include "wordProcessing.h"

/**
 * @struct fileData
 * @brief Structure to hold file data, including paths, count, and the current file index.
 *
 * This structure is used to manage a collection of file paths, keep track of the total number of files,
 * and indicate the current file being processed by an algorithm.
 */
typedef struct {
    uint16_t current; ///< Index of the current file being read by the algorithm.
    char** files; ///< Array of file paths.
    uint16_t fileCount; ///< Total number of files.
} fileData;

fileData data;
bool isReading = false;
pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t notReading;
pthread_once_t init = PTHREAD_ONCE_INIT;
FILE* currentFile = NULL;

void initialization() {
    data.current = 0;
    data.fileCount = 0;
    pthread_cond_init(&notReading,NULL);
}

int setFiles(char** files,uint16_t count) {
    pthread_once(&init,initialization);
    data.files = files;
    data.fileCount = count;
    if(count == 0)
        currentFile = NULL;
    else {
        if((currentFile = fopen(files[0],"rb")) == NULL) {
            return -1;
        }
        data.current = 0;
    }

    return 0;
}

int signalAndUnlock() {
    isReading = false;
    int result = 0;
    if(pthread_cond_signal(&notReading) != 0)
        result = -1;

    if(pthread_mutex_unlock(&accessCR) != 0)
        result = -2;

    return result;
}

int fillBuffer(uint8_t* buffer,uint32_t bufferSize,uint16_t* fileIndex,uint32_t* newBufferSize) {
    if(pthread_mutex_lock(&accessCR) != 0) {
        return -3;
    }

    pthread_once(&init,initialization);

    while(isReading) {
        if(pthread_cond_wait(&notReading,&accessCR) != 0) {
            if(pthread_mutex_unlock(&accessCR) != 0) {
                return -6;
            }

            return -4;
        }
    }

    isReading = true;

    if(currentFile == NULL) {
        signalAndUnlock();
        return -2;
    }

    if(data.current == data.fileCount) {
        signalAndUnlock();
        return EOF;
    }

    int bytesRead = fread(buffer,sizeof(uint8_t),bufferSize,currentFile);
    *fileIndex = data.current;
    if(feof(currentFile)) {
        fclose(currentFile);
        data.current++;
        if(data.current != data.fileCount) {
            if((currentFile = fopen(data.files[data.current],"rb")) == NULL) {
                signalAndUnlock();
                return -2;
            }
        }

        *newBufferSize = bytesRead;
    }
    else {
        uint32_t newLen = reduceToLastPosOfFullCharacter(buffer,bytesRead);
        newLen = reduceToLastFullWord(buffer,newLen);

        long moveBackAmount = bytesRead - newLen;
        if(fseek(currentFile,-moveBackAmount,SEEK_CUR) != 0) {
            signalAndUnlock();
            return -7;
        }

        *newBufferSize = newLen;
    }

   signalAndUnlock();
    
    return 0;
}