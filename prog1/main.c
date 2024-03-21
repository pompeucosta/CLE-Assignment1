#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "wordProcessing.h"

#define MAX_BUFFER_SIZE 8000

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
    FILE* file = fopen("dataSet1/text0.txt", "rb");

    if (file == NULL) {
        printf("Error when trying to open file: %s\n","t");
        perror("");
        return -1;
    }

    const int bufferSize = MAX_BUFFER_SIZE;
    uint8_t* buffer;
    uint32_t bufferLen = 0,numWords = 0,numWordsConsonants = 0,newLen = 0,tempNWords,tempNWordsConsonants;
    long moveBackAmount = 0;

    buffer = (uint8_t*)malloc(sizeof(uint8_t) * bufferSize);
    while(!feof(file)) {
        bufferLen = fread(buffer,1,bufferSize,file);
        if(!feof(file)) {
            newLen = reduceToLastPosOfFullCharacter(buffer,bufferLen);
            newLen = reduceToLastFullWord(buffer,newLen);
        }
        else {
            newLen = bufferLen;
        }

        processBuffer(buffer,newLen,&tempNWords,&tempNWordsConsonants);
        numWords += tempNWords;
        numWordsConsonants += tempNWordsConsonants;
        moveBackAmount = bufferLen - newLen;
        if(!feof(file) && (fseek(file,-moveBackAmount,SEEK_CUR) != 0)) {
            perror("");
            break;
        }
    }
    
    fclose(file);
    printf("#words: %d\n",numWords);
    printf("#words with consonants: %d\n",numWordsConsonants);
}