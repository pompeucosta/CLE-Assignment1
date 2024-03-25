#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "wordProcessing.h"
#include "wordProcessingSharedArea.h"

#define MAX_BUFFER_SIZE 8000

typedef struct {
    uint32_t numWords;
    uint32_t numWordsWithConsonants;
} fileResults;

typedef struct {
    uint32_t workerId;
    fileResults* results;
    uint32_t returnCode;
} workerParams;

workerParams* statusWorkers;
uint32_t fileCount;

void processBuffer(uint8_t* pBuffer,uint32_t bufferSize,uint32_t* pNumWords,uint32_t* pNumWordsWithTwoOrMoreConsonants) {
    uint32_t character,consonantsChecked = 0;
    uint8_t bytesRead = 0;
    bool insideWord = false,wordCheckedFlag = false;
    (*pNumWords) = 0;
    (*pNumWordsWithTwoOrMoreConsonants) = 0;

    for(uint32_t totalBytesRead = 0; totalBytesRead < bufferSize; totalBytesRead += bytesRead) {
        character = getUTF8Character(pBuffer,bufferSize,totalBytesRead,&bytesRead);
        removeUTF8Accent(&character);
        lowercase(&character);

        if(!insideWord) {
            if(isAlphanumericOrUnderscore(character)) {
                (*pNumWords)++;
                insideWord = true;
                consonantsChecked = 0;
                wordCheckedFlag = false;
            }
        }
        else {
            if(isSeparationCharacter(character)) {
                insideWord = false;
            }
        }

        if(insideWord && !wordCheckedFlag) {
            if(character >= 'b' && character <= 'z' && character != 'e' && character != 'i' && character != 'o' && character != 'u') {
                uint32_t pos = 0x01 << (character - 'b');
                if(consonantsChecked & pos) {
                    (*pNumWordsWithTwoOrMoreConsonants)++;
                    wordCheckedFlag = true;
                }
                else {
                    consonantsChecked |= pos;
                }
            }
        }
    }
}

void* processFiles(void* params) {
    uint32_t id = *((uint32_t*) params);
    for(int i = 0; i < fileCount; i++) {
        statusWorkers[id].results[i].numWords = 0;
        statusWorkers[id].results[i].numWordsWithConsonants = 0;
    }

    const int bufferSize = MAX_BUFFER_SIZE;
    uint8_t* buffer;
    uint32_t tempNWords,tempNWordsConsonants;
    uint32_t bufferLen;
    int code;
    uint16_t fileIndex;
    buffer = (uint8_t*)malloc(sizeof(uint8_t) * bufferSize);
    while((code = fillBuffer(buffer,bufferSize,&fileIndex,&bufferLen)) == 0) {
        processBuffer(buffer,bufferLen,&tempNWords,&tempNWordsConsonants);
        statusWorkers[id].results[fileIndex].numWords += tempNWords;
        statusWorkers[id].results[fileIndex].numWordsWithConsonants += tempNWordsConsonants;
    }

    if(code < -1) {
        statusWorkers[id].returnCode = EXIT_FAILURE;
    }
    else {
        statusWorkers[id].returnCode = EXIT_SUCCESS;
    }

    free(buffer);
    pthread_exit(&statusWorkers[id]);
}

int main(int argc,char* argv[]) {
    uint16_t count = 5;
    fileCount = 5;
    char* files[count];
    files[0] = "dataSet1/text0.txt";
    files[1] = "dataSet1/text1.txt";
    files[2] = "dataSet1/text2.txt";
    files[3] = "dataSet1/text3.txt";
    files[4] = "dataSet1/text4.txt";
    int code = setFiles(files,count);
    if(code < 0) {
        perror("Failed to set files");
        return EXIT_FAILURE;
    }

    const uint32_t numWorkers = 2;
    pthread_t* workers;

    if((workers = malloc(numWorkers * sizeof(pthread_t))) == NULL ||
        (statusWorkers = malloc(numWorkers * sizeof(workerParams))) == NULL) {
            perror("error allocation memory for threads");
            exit(EXIT_FAILURE);
        }

    for(int i = 0; i < numWorkers; i++) {
        statusWorkers[i].workerId = i;
        statusWorkers[i].results = malloc(count * sizeof(fileResults));
    }


    for(int i = 0; i < numWorkers; i++) {
        if((pthread_create(&workers[i],NULL,processFiles,&statusWorkers[i])) != 0) {
            perror("error creating thread");
            exit(EXIT_FAILURE);
        }
    }

    fileResults results[count];
    for(int i = 0; i < count; i++) {
        results[i].numWords = 0;
        results[i].numWordsWithConsonants = 0;
    }

    void* status;
    for(int i = 0; i < numWorkers; i++) {
        if((pthread_join(workers[i],(void*) &status)) != 0) {
            perror("failed to join thread");
            exit(EXIT_FAILURE);
        }

        workerParams w = *(workerParams*)status;
        if(w.returnCode == EXIT_FAILURE) {
            fprintf(stderr,"thread returned in failure\n");
            exit(EXIT_FAILURE);
        }

        for(int j = 0; j < count; j++) {
            results[j].numWords += w.results[j].numWords;
            results[j].numWordsWithConsonants += w.results[j].numWordsWithConsonants;
        }
    }

    for(int i = 0; i < count; i++) {
        printf("File: %s\n",files[i]);
        printf("#words: %d\n",results[i].numWords);
        printf("#words with consonants: %d\n\n",results[i].numWordsWithConsonants);
    }

    exit(EXIT_SUCCESS);
}