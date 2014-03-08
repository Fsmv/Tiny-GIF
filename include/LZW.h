#ifndef LZW_H
#define LZW_H

#include <stdint.h>
#include <stddef.h>

/**
 * Compresses a string with the LZW algorithm
 *
 * The code array will be allocated on the heap
 *
 * @param string the text to compress
 * @param code pointer to the array to store the result in
 * @param codec pointer to the size of the array allocated
 */
extern void LZW_Compress(const char *string, uint16_t **code, size_t *codec, uint8_t alphabetSize);

/**
 * Decompresses an LZW Code
 *
 * The string array will be allocated on the heap
 *
 * @param code the code to decompress
 * @param codec the size of the code array
 * @param string pointer to store a new string in
 */
extern void LZW_Decompress(const uint16_t *code, const size_t codec, char **string, uint8_t alphabetSize);

#endif
