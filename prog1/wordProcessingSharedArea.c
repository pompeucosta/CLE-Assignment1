#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include "wordProcessingSharedArea.h"
#include "wordProcessing.h"

typedef struct {
    uint16_t current;
    char** files;
    uint16_t fileCount;
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

int fillBuffer(uint8_t* buffer,uint32_t bufferSize,uint16_t* fileIndex,uint32_t* newBufferSize) {
    int result = 0;
    if(pthread_mutex_lock(&accessCR) != 0) {
        result = -3;
    }

    pthread_once(&init,initialization);

    if(result == 0) {
        while(isReading) {
            if(pthread_cond_wait(&notReading,&accessCR) != 0) {
                result = -4;
            }
        }

        isReading = true;

        if(currentFile == NULL && result == 0) {
            result = -2;
        }

        if(data.current == data.fileCount && result == 0) {
            result = EOF;
        }

        if(result == 0) {
            int bytesRead = fread(buffer,sizeof(uint8_t),bufferSize,currentFile);
            *fileIndex = data.current;
            if(feof(currentFile)) {
                fclose(currentFile);
                data.current++;
                if(data.current != data.fileCount) {
                    if((currentFile = fopen(data.files[data.current],"rb")) == NULL) {
                        result = -2;
                    }
                }

                *newBufferSize = bytesRead;
            }
            else {
                uint32_t newLen = reduceToLastPosOfFullCharacter(buffer,bytesRead);
                newLen = reduceToLastFullWord(buffer,newLen);

                long moveBackAmount = bytesRead - newLen;
                if(fseek(currentFile,-moveBackAmount,SEEK_CUR) != 0) {
                    result = -7;
                }

                *newBufferSize = newLen;
            }
        }

        isReading = false;
    }

    if(pthread_cond_signal(&notReading) != 0) {
        result = -5;
    }

    if(pthread_mutex_unlock(&accessCR) != 0) {
        result = -6;
    }
    
    return result;
}