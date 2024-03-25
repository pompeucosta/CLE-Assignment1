#ifndef WordProcessing
#define WordProcessing

#include <stdbool.h>
#include <stdint.h>

/// @brief Reduces the size of the buffer so it ends with a full character 
/// @param buffer The buffer of bytes
/// @param bufferLen The size of the buffer
/// @return The size of the buffer without the bytes that are not of a full character
uint32_t reduceToLastPosOfFullCharacter(uint8_t* buffer,uint32_t bufferLen);

/// @brief Gets a UTF-8 character
/// @param buffer The buffer of bytes
/// @param bufferLen The size of the buffer
/// @param startPos The position to start reading from
/// @param bytesRead The amount of bytes that the character read has
/// @return The character read if startPos is within the buffer's bounds, otherwise returns 0
uint32_t getUTF8Character(uint8_t* buffer,uint32_t bufferLen,uint32_t startPos,uint8_t* bytesRead);

/// @brief Remove the accentuation of an UTF-8 portuguese character
/// @param pCharacter The character to remove the accentuation
void removeUTF8Accent(uint32_t* pCharacter);

/// @brief Lowercases an uppercase ASCII character
/// @param character The character to be lowercased
void lowercase(uint32_t* character);

/// @brief Determines if a character is a separation character
/// @param character The character to check
/// @return true if it is a separation character, false otherwise
bool isSeparationCharacter(uint32_t character);

/// @brief Determines if a character is alphanumeric or an underscore
/// @param character The character to check
/// @return true if the character is alphanumeric or underscore, false otherwise
bool isAlphanumericOrUnderscore(uint32_t character);

/// @brief Reduces the size of the buffer so it contains only full words
/// @param pBuffer The buffer
/// @param bufferSize The size of the buffer
/// @return The size of the buffer with only full words
uint32_t reduceToLastFullWord(uint8_t* pBuffer,uint32_t bufferSize);
#endif