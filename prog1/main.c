#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "wordProcessing.h"
#include "wordProcessingSharedArea.h"

#define MAX_BUFFER_SIZE 8000

typedef struct {
    uint32_t numWords;
    uint32_t numWordsWithConsonants;
} fileResults;

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

int main(int argc,char* argv[]) {
    uint16_t count = 5;
    char* files[count];
    files[0] = "dataSet1/text0.txt";
    files[1] = "dataSet1/text1.txt";
    files[2] = "dataSet1/text2.txt";
    files[3] = "dataSet1/text3.txt";
    files[4] = "dataSet1/text4.txt";
    setFiles(files,count);

    fileResults results[count];
    for(int i = 0; i < count; i++) {
        results[i].numWords = 0;
        results[i].numWordsWithConsonants = 0;
    }

    const int bufferSize = MAX_BUFFER_SIZE;
    uint8_t* buffer;
    uint32_t tempNWords,tempNWordsConsonants;
    int bufferLen;
    uint16_t fileIndex;

    buffer = (uint8_t*)malloc(sizeof(uint8_t) * bufferSize);
    while((bufferLen = fillBuffer(buffer,bufferSize,&fileIndex)) != EOF) {
        processBuffer(buffer,bufferLen,&tempNWords,&tempNWordsConsonants);
        results[fileIndex].numWords += tempNWords;
        results[fileIndex].numWordsWithConsonants += tempNWordsConsonants;
    }
    
    free(buffer);
    for(int i = 0; i < count; i++) {
        printf("File: %s\n",files[i]);
        printf("#words: %d\n",results[i].numWords);
        printf("#words with consonants: %d\n\n",results[i].numWordsWithConsonants);
    }
}