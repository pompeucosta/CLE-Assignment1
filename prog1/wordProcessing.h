#ifndef WordProcessing
#define WordProcessing

#include <stdbool.h>
#include <stdint.h>

uint32_t reduceToLastPosOfFullCharacter(uint8_t* buffer,uint32_t bufferLen);

uint32_t getUTF8Character(uint8_t* buffer,uint32_t bufferLen,uint32_t startPos,uint8_t* bytesRead);

void removeLowercaseUTF8Accent(uint32_t utfCharacter,uint32_t* pConvertedCharacter);

void removeUTF8Accent(uint32_t* pCharacter);

void lowercase(uint32_t* character);

bool isSeparationCharacter(uint32_t character);

bool isAlphanumericOrUnderscore(uint32_t character);

uint32_t reduceToLastFullWord(uint8_t* pBuffer,uint32_t bufferSize);
#endif