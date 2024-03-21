#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_BUFFER_SIZE 8000

#define BYTE_0_0_EXTRA 0x00
#define BYTE_0_1_EXTRA 0xC0
#define BYTE_0_2_EXTRA 0xE0
#define BYTE_0_3_EXTRA 0xF0
#define BYTES_1_2_3_EXTRA 0x80

#define BYTE_0_0_EXTRA_MASK 0x80
#define BYTE_0_1_EXTRA_MASK 0xE0
#define BYTE_0_2_EXTRA_MASK 0xF0
#define BYTE_0_3_EXTRA_MASK 0xF8
#define BYTES_1_2_3_EXTRA_MASK 0xC0

uint32_t reduceToLastPosOfFullCharacter(uint8_t* buffer,uint32_t bufferLen) {
    uint8_t bytesRead = 0;
    uint8_t expectedNumberOfExtraBytes = 0;
    for(uint8_t pointer = bufferLen - 1; pointer > 0; pointer--) {    
        bytesRead++;
        if((buffer[pointer] & BYTE_0_0_EXTRA_MASK) == BYTE_0_0_EXTRA) {
            expectedNumberOfExtraBytes = 0;
            break;
        }
        else if((buffer[pointer] & BYTE_0_1_EXTRA_MASK) == BYTE_0_1_EXTRA) {
            expectedNumberOfExtraBytes = 1;
            break;
        }
        else if((buffer[pointer] & BYTE_0_2_EXTRA_MASK) == BYTE_0_2_EXTRA) {
            expectedNumberOfExtraBytes = 2;
            break;
        }
        else if((buffer[pointer] & BYTE_0_3_EXTRA_MASK) == BYTE_0_3_EXTRA) {
            expectedNumberOfExtraBytes = 3;
            break;
        }
    }


    if((bytesRead - 1) != expectedNumberOfExtraBytes) {
        bufferLen -= bytesRead;
    }

    return bufferLen;
}

uint32_t getUTF8Character(uint8_t* buffer,uint32_t bufferLen,uint32_t startPos,uint8_t* bytesRead) {
    if(startPos >= bufferLen)
        return 0;
    
    uint8_t byte = buffer[startPos];
    uint32_t character = byte;

    int numBytesExtra = -1;

    if ((byte & BYTE_0_0_EXTRA_MASK) == BYTE_0_0_EXTRA) {
        numBytesExtra = 0;
    } else if ((byte & BYTE_0_1_EXTRA_MASK) == BYTE_0_1_EXTRA) {
        numBytesExtra = 1;
    } else if ((byte & BYTE_0_2_EXTRA_MASK) == BYTE_0_2_EXTRA) {
        numBytesExtra = 2;
    } else if ((byte & BYTE_0_3_EXTRA_MASK) == BYTE_0_3_EXTRA) {
        numBytesExtra = 3;
    }

    for(int i = 0; i < numBytesExtra; i++) {
        character <<= 8;
        character |= buffer[startPos + i + 1];
    }

    *bytesRead = numBytesExtra + 1;
    return character;
}

void removeLowercaseUTF8Accent(uint32_t utfCharacter,uint32_t* pConvertedCharacter) {
    #define a_lowerbound 0xC3A0
    #define a_upperbound 0xC3A3

    #define e_lowerbound 0xC3A8
    #define e_upperbound 0xC3AA

    #define i_lowerbound 0xC3AC
    #define i_upperbound 0xC3AD

    #define o_lowerbound 0xC3B2
    #define o_upperbound 0xC3B5

    #define u_lowerbound 0xC3B9
    #define u_upperbound 0xC3BA

    #define c_cedilha 0xC3A7

    if(utfCharacter >= a_lowerbound && utfCharacter <= a_upperbound) {
        *pConvertedCharacter = 'a';
        return;
    }
    
    if(utfCharacter >= e_lowerbound && utfCharacter <= e_upperbound) {
        *pConvertedCharacter = 'e';
        return;
    }
    
    if(utfCharacter >= i_lowerbound && utfCharacter <= i_upperbound) {
        *pConvertedCharacter = 'i';
        return;
    }
    
    if(utfCharacter >= o_lowerbound && utfCharacter <= o_upperbound) {
        *pConvertedCharacter = 'o';
        return;
    }
    
    if(utfCharacter >= u_lowerbound && utfCharacter <= u_upperbound) {
        *pConvertedCharacter = 'u';
        return;
    }
    
    if(utfCharacter == c_cedilha) {
        *pConvertedCharacter = 'c';
        return;
    }
}

void removeUTF8Accent(uint32_t* pCharacter) {
    uint32_t character = *pCharacter;

    removeLowercaseUTF8Accent(*pCharacter,&character);
    if(character != (*pCharacter)) {
        *pCharacter = character;
        return;
    }

    removeLowercaseUTF8Accent((*pCharacter) + 0x0020,&character);
    if(character != (*pCharacter)) {
        *pCharacter = character;
        return;
    }
}

void lowercase(uint32_t* character) {
    if((*character) >= 65 && (*character) <= 90) {
        *character += 32;
    }
}

bool isSeparationCharacter(uint32_t character) {
    if(character == ' ' || character == 0x9 || character == '\n' || character == '\r') {
        return true;
    }

    if(character == '-' || character == '"' || character == 0xE2809C || character == 0xe2809D || character == '[' || character == ']' || character == '(' || character == ')') {
        return true;
    }

    if(character == '.' || character == ',' || character == ':' || character == ';' || character == '?' || character == '!' || character == 0xE28093 || character == 0xE280A6) {
        return true;
    }

    return false;
}

bool isAlphanumericOrUnderscore(uint32_t character) {
    if(character >= 97 && character <= 122)
        return true;

    if(character >= 48 && character <= 57)
        return true;

    if(character == '_')
        return true;

    return false;
}

uint32_t reduceToLastFullWord(uint8_t* pBuffer,uint32_t bufferSize) {
    for(uint32_t i = bufferSize - 1;i >= 0; i--) {
        uint8_t byte = pBuffer[i];
        if((byte & BYTES_1_2_3_EXTRA_MASK) == BYTES_1_2_3_EXTRA)
            continue;

        int numBytesExtra = -1;

        if ((byte & BYTE_0_0_EXTRA_MASK) == BYTE_0_0_EXTRA) {
            numBytesExtra = 0;
        } else if ((byte & BYTE_0_1_EXTRA_MASK) == BYTE_0_1_EXTRA) {
            numBytesExtra = 1;
        } else if ((byte & BYTE_0_2_EXTRA_MASK) == BYTE_0_2_EXTRA) {
            numBytesExtra = 2;
        } else if ((byte & BYTE_0_3_EXTRA_MASK) == BYTE_0_3_EXTRA) {
            numBytesExtra = 3;
        }

        uint32_t character = byte;
        for(int j = 0; j < numBytesExtra; j++) {
            character <<= 8;
            character |= pBuffer[i + j + 1];
        }

        if(isSeparationCharacter(character)) {
            return i;
        }
    }
}

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
    FILE* file = fopen("dataSet1/text4.txt", "rb");

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