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
pthread_cond_t reading;
pthread_cond_t notReading;
pthread_once_t init = PTHREAD_ONCE_INIT;
FILE* currentFile = NULL;

void initialization() {
    data.current = 0;
    data.fileCount = 0;
    pthread_cond_init(&reading,NULL);
    pthread_cond_init(&notReading,NULL);
}

void setFiles(char** files,uint16_t count) {
    pthread_once(&init,initialization);
    data.files = files;
    data.fileCount = count;
    if(count == 0)
        currentFile = NULL;
    else {
        currentFile = fopen(files[0],"rb");
        data.current = 0;
    }
}

int fillBuffer(uint8_t* buffer,uint32_t bufferSize,uint16_t* fileIndex) {
    pthread_once(&init,initialization);
    if(data.current == data.fileCount)
        return EOF;

    int bytesRead = fread(buffer,sizeof(uint8_t),bufferSize,currentFile);
    *fileIndex = data.current;
    if(feof(currentFile)) {
        fclose(currentFile);
        data.current++;
        if(data.current != data.fileCount) {
            if((currentFile = fopen(data.files[data.current],"rb")) == NULL) {
                perror("");
            }
        }

        return bytesRead;
    }
    else {
        uint32_t newLen = reduceToLastPosOfFullCharacter(buffer,bytesRead);
        newLen = reduceToLastFullWord(buffer,newLen);

        long moveBackAmount = bytesRead - newLen;
        fseek(currentFile,-moveBackAmount,SEEK_CUR);

        return newLen;
    }

}