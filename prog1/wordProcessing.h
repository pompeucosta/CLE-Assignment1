#ifndef WordProcessing
#define WordProcessing

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Reduces the size of the buffer so it ends with a full character.
 *
 * @param buffer The buffer of bytes to be adjusted.
 * @param bufferLen The initial size of the buffer.
 *
 * @return The new size of the buffer after removing the trailing bytes that do not form a full character.
 */
uint32_t reduceToLastPosOfFullCharacter(uint8_t* buffer,uint32_t bufferLen);

/**
 * @brief Gets a UTF-8 character from the buffer.
 *
 * @param buffer The buffer of bytes from which to read the character.
 * @param bufferLen The size of the buffer.
 * @param startPos The position in the buffer to start reading from.
 * @param bytesRead The number of bytes that were read to form the character.
 *
 * @return The UTF-8 character read from the buffer, or 0 if the start position is out of bounds.
 */
uint32_t getUTF8Character(uint8_t* buffer,uint32_t bufferLen,uint32_t startPos,uint8_t* bytesRead);

/**
 * @brief Removes the accentuation of a UTF-8 Portuguese character.
 *
 * @param pCharacter The character from which to remove the accentuation.
 */
void removeUTF8Accent(uint32_t* pCharacter);

/**
 * @brief Lowercases an uppercase ASCII character.
 *
 * @param character The character to be lowercased.
 */
void lowercase(uint32_t* character);

/**
 * @brief Determines if a character is a separation character.
 *
 * @param character The character to check.
 *
 * @return true if the character is a separation character, false otherwise.
 */
bool isSeparationCharacter(uint32_t character);

/**
 * @brief Determines if a character is alphanumeric or an underscore.
 *
 * @param character The character to check.
 *
 * @return true if the character is alphanumeric or an underscore, false otherwise.
 */
bool isAlphanumericOrUnderscore(uint32_t character);

/**
 * @brief Reduces the size of the buffer so it contains only full words.
 *
 * @param pBuffer The buffer to be adjusted.
 * @param bufferSize The initial size of the buffer.
 *
 * @return The new size of the buffer after removing the trailing characters that do not form a full word.
 */
uint32_t reduceToLastFullWord(uint8_t* pBuffer,uint32_t bufferSize);
#endif