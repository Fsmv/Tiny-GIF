#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "Dictionary.h"

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

    uint16_t *result = malloc(sizeof(uint16_t) * 50);
    size_t resultIndex = 0;

    uint8_t currSym[10];
    size_t symLen = 0;
    size_t charIndex = 0;
    while(string[charIndex] != '\0') {
        currSym[symLen] = string[charIndex];
        symLen++;

        if(!dict_contains(&head, currSym, symLen)) {
            result[resultIndex] = dict_add(&head, currSym, symLen);

            currSym[0] = string[charIndex];
            symLen = 1;
            resultIndex++;
        }

        charIndex++;
    }

    *code = result;
    *codec = resultIndex;
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
