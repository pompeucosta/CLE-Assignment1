#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <libgen.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "wordProcessing.h"
#include "wordProcessingSharedArea.h"

#define MAX_BUFFER_SIZE 8000

uint32_t fileCount;
uint32_t* statusWorkers;

/**
 * @brief Processes a buffer to calculate the amount of words and the amount of words with at least two equal consonants.
 *
 * @param pBuffer The buffer to read from. This should point to the start of the buffer containing the text to be processed.
 * @param bufferSize The size of the buffer. This indicates the number of bytes available in the buffer for reading.
 * @param pNumWords Pointer to a variable where the total number of words found in the buffer will be stored.
 * @param pNumWordsWithTwoOrMoreConsonants Pointer to a variable where the number of words with at least two equal consonants will be stored.
 */
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

/**
 * @brief Worker function to process the files.
 *
 * @param params Pointer to application-defined worker identification. This is used
 *               to identify the specific worker instance and its data.
 */
void* processFiles(void* params) {
    uint32_t id = *((uint32_t*) params);

    const int bufferSize = MAX_BUFFER_SIZE;
    uint8_t* buffer;
    uint32_t nWords,nWordsConsonants;
    uint32_t bufferLen;
    int code;
    uint16_t fileID;
    buffer = (uint8_t*)malloc(sizeof(uint8_t) * bufferSize);
    while((code = fillBuffer(buffer,bufferSize,&fileID,&bufferLen)) == 0) {
        processBuffer(buffer,bufferLen,&nWords,&nWordsConsonants);
        if(savePartialResults(fileID,nWords,nWordsConsonants) != 0) {
            perror("failed to save partial results");
            statusWorkers[id] = EXIT_FAILURE;
            pthread_exit(&statusWorkers[id]);
        }
    }

    if(code < -1) {
        statusWorkers[id] = EXIT_FAILURE;
    }
    else {
        statusWorkers[id] = EXIT_SUCCESS;
    }

    free(buffer);
    pthread_exit(&statusWorkers[id]);
}


/**
 * @brief Print command usage
 *
 * @param progName A string containing the name of the program.
 */
void printUsage(char* progName) {
    printf ("\nSynopsis: %s [FILE PATHS] [OPTIONS]\n"
           "  OPTIONS:\n"
           "  -t nThreads  --- set the number of threads to be created (default: 1)\n"
           "  -h           --- print this help\n", progName);
}

/**
 *  @brief Get the process time that has elapsed since last call of this time.
 *
 *  @return process elapsed time
 */
double get_delta_time(void)
{
    static struct timespec t0, t1;

    t0 = t1;
    if(clock_gettime (CLOCK_MONOTONIC, &t1) != 0) {
        perror ("clock_gettime");
        exit(1);
    }
    return (double) (t1.tv_sec - t0.tv_sec) + 1.0e-9 * (double) (t1.tv_nsec - t0.tv_nsec);
}

/**
 * @brief Main thread
 *
 * @param argc Number of arguments from the command line.
 * @param argv List of arguments from the command line.
 * @return Status of the operation.
 */
int main(int argc,char* argv[]) {
    int option;
    uint32_t numWorkers = 1;

    while((option = getopt(argc,argv,"t:h")) != -1) {
        switch (option)
        {
            case 't':
                if(atoi(optarg) <= 0) {
                    fprintf(stderr,"Number of workers must be positive\n");
                    printUsage(basename(argv[0]));
                    return EXIT_FAILURE;
                }
                numWorkers = atoi(optarg);
                break;
            case 'h':
                printUsage(basename(argv[0]));
                return EXIT_SUCCESS;
            case '?':
                fprintf(stderr,"Invalid option\n");
                printUsage(basename(argv[0]));
                return EXIT_FAILURE;
        }
    }

    uint16_t fileCount = argc - optind;
    if(fileCount == 0) {
        fprintf(stderr,"Please provide files\n");
        printUsage(basename(argv[0]));
    }

    char* files[fileCount];
    int j = 0;
    for(int i = optind; i < argc; i++,j++) {
        files[j] = argv[i];
    }

    if(setFiles(files,fileCount) < 0) {
        perror("Failed to set files");
        return EXIT_FAILURE;
    }

    pthread_t* workers;

    if((workers = malloc(numWorkers * sizeof(pthread_t))) == NULL ||
        (statusWorkers = malloc(numWorkers * sizeof(uint32_t))) == NULL) {
            perror("error allocation memory for threads");
            exit(EXIT_FAILURE);
        }


    for(uint32_t i = 0; i < numWorkers; i++) {
        statusWorkers[i] = i;
    }

    (void) get_delta_time();

    for(int i = 0; i < numWorkers; i++) {
        if((pthread_create(&workers[i],NULL,processFiles,&statusWorkers[i])) != 0) {
            perror("error creating thread");
            exit(EXIT_FAILURE);
        }
    }

    void* status;
    for(int i = 0; i < numWorkers; i++) {
        if((pthread_join(workers[i],(void*) &status)) != 0) {
            perror("failed to join thread");
            exit(EXIT_FAILURE);
        }

        uint32_t code = *(uint32_t*)status;
        if(code == EXIT_FAILURE) {
            fprintf(stderr,"thread exited in failure\n");
            exit(EXIT_FAILURE);
        }
    }

    if(results == NULL) {
        fprintf(stderr,"A problem ocurred when trying to access the results\n");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < fileCount; i++) {
        printf("File: %s\n",files[i]);
        printf("#words: %d\n",results[i].numWords);
        printf("#words with consonants: %d\n\n",results[i].numWordsWithConsonants);
    }

    printf("Elapsed time = %.6fs \n",get_delta_time());

    exit(EXIT_SUCCESS);
}