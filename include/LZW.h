#ifndef LZW_H
#define LZW_H

#include <stdint.h>
#include <stddef.h>
#include "Dictionary.h"

//the maximum index for gifs
#define MAX_INDEX 0xFFF

typedef struct {
    Dictionary dict;
    uint8_t *currSym;
    uint8_t alphabetSize;
} LZW;

/**
 * Compresses a string with the LZW algorithm
 *
 * The code array will be allocated on the heap
 *
 * @param string the text to compress
 * @param code pointer to the array to store the result in
 * @param codec pointer to the size of the array allocated
 * @param alphabetSize the number of possible initial codes
 */
extern void LZW_Compress(const char *string, size_t stringc, uint16_t **code, size_t *codec, uint8_t alphabetSize);

/**
 * Decompresses an LZW Code
 *
 * The string array will be allocated on the heap
 *
 * @param code the code to decompress
 * @param codec the size of the code array
 * @param string pointer to store a new string in
 * @param alphabetSize the number of possible initial codes
 */
extern void LZW_Decompress(const uint16_t *code, const size_t codec, char **string, uint8_t alphabetSize);

/**
 * Compresses data one byte at a time
 *
 * @param data byte to compress
 * @param state the state, alphabetSize must be initialized, dict and currSym
 * must be NULL for the first byte
 * @return the compressed data value (or 0xFFFF if nothing was output)
 */
extern uint16_t LZW_CompressOne(const char data, LZW *state);

/**
 * Initializes LZW state
 *
 * dict = currSym = NULL;
 * alphabetSize = alphabetSize;
 */
inline void LZW_Init(uint8_t alphabetSize, LZW *state) {
    state->dict = state->currSym = NULL;
    state->alphabetSize = alphabetSize;
}

#endif
