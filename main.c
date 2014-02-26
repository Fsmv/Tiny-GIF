#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Dictionary.h"

/**
 * Appends the element to the array extending it if necessary
 *
 * @param arr array to append to
 * @param size size of the array currently
 * @param index position to add to
 * @param code element to add
 */
void setCode(uint16_t **arr, size_t *size, size_t index, const uint16_t code) {
    if(index >= *size) {
        uint16_t *temp = malloc(sizeof(uint16_t) * 2 * index);
        memcpy(temp, *arr, *size * sizeof(uint16_t));
        free(*arr);
        *size = 2 * index;
        *arr = temp;
    }

    (*arr)[index] = code;
}

/**
 * Appends the element to the array extending it if necessary
 *
 * @param arr array to append to
 * @param size size of the array currently
 * @param index position to add to
 * @param ch element to add
 */
void setChar(char **arr, size_t *size, size_t index, const char ch) {
    if(index >= *size) {
        char *temp = malloc(sizeof(char) * 2 * index);
        memcpy(temp, *arr, *size * sizeof(char));
        free(*arr);
        *size = 2 * index;
        *arr = temp;
    }

    (*arr)[index] = ch;
}

/**
 * Compresses a string with the LZW algorithm
 *
 * The code array will be allocated on the heap
 *
 * @param string the text to compress
 * @param code pointer to the array to store the result in
 * @param codec pointer to the size of the array allocated
 */
void LZW_Compress(const char *string, uint16_t **code, size_t *codec) {
    Node head;
    dict_init(&head);

    size_t resultLen = 64;
    uint16_t *result = malloc(sizeof(uint16_t) * resultLen);
    size_t resultIndex = 0;

    size_t symLen = 16;
    uint8_t *currSym = malloc(sizeof(uint8_t) * symLen);
    size_t symIndex = 0;
    size_t charIndex = 0;
    while(string[charIndex] != '\0') {
        setChar((char**)&currSym, &symLen, symIndex, string[charIndex]);
        symIndex++;

        if(!dict_contains(&head, currSym, symIndex)) {
            setCode(&result, &resultLen, resultIndex, dict_add(&head, currSym, symIndex));

            currSym[0] = string[charIndex];
            symIndex = 1;
            resultIndex++;
        }

        charIndex++;
    }

    setChar((char**)&currSym, &symLen, symIndex, '\0');
    setCode(&result, &resultLen, resultIndex, dict_add(&head, currSym, symIndex+1));
    resultIndex++;

    *code = result;
    *codec = resultIndex;

    free(currSym);
    dict_free(&head);
}

/**
 * Decompresses an LZW Code
 *
 * The string array will be allocated on the heap
 *
 * @param code the code to decompress
 * @param codec the size of the code array
 * @param string pointer to store a new string in
 */
void LZW_Decompress(const uint16_t *code, const size_t codec, char **string) {
    Node head;
    dict_init(&head);

    int i;
    for(i = 0; i < codec; i++) {
        
    }

    dict_free(&head);
}

/**
 * Compresses a file and outputs the result in outfile
 *
 * @param infile name of the file to compress
 * @param outfile name of the file to write to
 */
void compressFile(const char *infile, const char *outfile) {
}

/**
 * Decompresses a file with the lzw algorithm and outputs in outfile
 *
 * @param infile name of the file to decompress
 * @param outfile name of the file to write to
 */
void decompressFile(const char *infile, const char *outfile) {
}

int main(int argc, char *argv[]) {
    clock_t last = clock();

    uint16_t *code;
    size_t length;
    LZW_Compress("abcdefafaaacaa", &code, &length);

    int i;
    for(i = 0; i < length; i++) {
        printf("%x ", code[i]);
    }

    free(code);

    printf("\nElapsed: %fs\n", (float)(clock() - last) / CLOCKS_PER_SEC);
}
