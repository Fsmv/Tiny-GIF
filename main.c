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
    Dictionary dict;
    dict_init(&dict);

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

        if(!dict_contains(&dict, currSym, symIndex)) {
            //If the dictionary does not cantain the symbol, add it
            setCode(&result, &resultLen, resultIndex, dict_add(&dict, currSym, symIndex));

            //if we go over 16 bits reset the dictionary
            if(dict.currIndex == 0xFFFF) {
                dict_free(&dict);
                dict_init(&dict);
                setCode(&result, &resultLen, resultIndex, CLEAR_CODE);
            }

            //set the current symbol to last char read
            currSym[0] = string[charIndex];
            symIndex = 1;
            resultIndex++;
        }

        charIndex++;
    }

    setChar((char**)&currSym, &symLen, symIndex, '\0');
    setCode(&result, &resultLen, resultIndex, dict_add(&dict, currSym, symIndex+1));
    resultIndex++;

    *code = result;
    *codec = resultIndex;

    free(currSym);
    dict_free(&dict);
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
    Dictionary dict;
    dict_init(&dict);

    size_t symLen = 16;
    uint8_t *currSym = malloc(sizeof(uint8_t) * symLen);
    size_t symIndex = 0;

    size_t resultLen = 32;
    char *result = malloc(sizeof(char) * resultLen);
    size_t resultIndex = 0;

    int i;
    for(i = 0; i < codec; i++) {
        uint8_t *phrase = NULL;
        size_t phraseLen;

        //If the clear code shows up reset the dictionary
        if(code[i] == CLEAR_CODE) {
            dict_free(&dict);
            dict_init(&dict);
            continue;
        }

        if(dict_search(&dict, code[i], &phrase, &phraseLen)) {
            //output phrase
            resultIndex += phraseLen;
            setChar(&result, &resultLen, resultIndex, '\0');
            memcpy(result + resultIndex - phraseLen, phrase, phraseLen);

            //Add currSym + phrase[0] to dictionary
            setChar((char **) &currSym, &symLen, symIndex, phrase[0]);
            if(symIndex > 0) { //if == 0 its already in the dictionary
                dict_add(&dict, currSym, symIndex+1);
            }

            //currSym = phrase
            free(currSym);
            currSym = phrase;
            symLen = symIndex = phraseLen;
        }else{
            //currSym += currSym[0]
            setChar((char **)&currSym, &symLen, symIndex, currSym[0]);
            symIndex++;

            //output currSym
            resultIndex += symIndex;
            setChar(&result, &resultLen, resultIndex, '\0');
            memcpy(result + resultIndex - symIndex, currSym, symIndex);

            //add currSym to dictionary
            dict_add(&dict, currSym, symIndex);
            free(phrase); //allocated in dict_search
        }
    }

    *string = result;

    free(currSym);
    dict_free(&dict);
}

/**
 * Compresses a file and outputs the result in outfile
 *
 * @param infile name of the file to compress
 * @param outfile name of the file to write to
 */
void processFile(FILE *infile, FILE *outfile, char shouldCompress) {
    if(infile == NULL || outfile == NULL) {
        fprintf(stderr, "Error: could not read files");
        exit(1);
    }

    //get file size of infile
    fseek(infile, 0L, SEEK_END);
    long inSize = ftell(infile);
    fseek(infile, 0L, SEEK_SET);

    char *inData = malloc(inSize * sizeof(char));
    fread(inData, sizeof(char), inSize, infile);

    if(shouldCompress) {
        uint16_t *compressedData = NULL;
        size_t dataSize;

        LZW_Compress(inData, &compressedData, &dataSize);

        fwrite(compressedData, sizeof(uint16_t), dataSize, outfile);
        free(compressedData);
    }else{
        char *outString = NULL;
        LZW_Decompress((uint16_t *) inData, inSize / (sizeof(uint16_t)/sizeof(char)), &outString);

        fputs(outString, outfile);
        free(outString);
    }

    free(inData);
    fclose(infile);
    fclose(outfile);
}

/**
 * Prints the help text for this program
 *
 * @param name argv[0], the name of the program being run
 */
void printHelp(char *name) {
    printf("Usage %s (options) [file]\n", name);
    printf("Options:\n");
    printf("\t-c: Compress file (appends .lzw");
    printf("\t-d: Decompress file (takes off .lzw or adds .orig)");
}

/**
 * Returns an opened file handle to a file name as defined in the help text
 *
 * When compressing add .lzw, when decompressing, remove .lzw or add .orig to
 * infile.
 *
 * Will modify infile
 *
 * @param infile name to base the outfile name on
 * @param isCompressing 1 if compressing 0 if decompressing
 * @return a file handle to the file to output to
 */
FILE *getOutFile(char *infile, const char isCompressing) {
    if(isCompressing) {
        return fopen(strcat(infile, ".lzw"), "wb");
    }else{
        size_t len = strlen(infile);

        //if .lzw did not exist or was not the last 4 chars
        if(strcmp(infile + len - 4, ".lzw") != 0) {
            return fopen(strcat(infile, ".orig"), "wb");
        }else{
            //take off the last 4 chars, which are .lzw
            infile[strlen(infile) - 4] = '\0';
            return fopen(infile, "wb");
        }
    }
}

int main(int argc, char *argv[]) {
    clock_t last = clock();

    if(argc <= 1 || argc > 3) {
        printHelp(argv[0]);
    }else if(argc == 2) {
        FILE *infile = fopen(argv[1], "rb");
        processFile(infile, getOutFile(argv[1], 1), 1);
    }else if(argc == 3) {
        if(strcmp(argv[1], "-c") == 0) {
            FILE *infile = fopen(argv[2], "rb");
            processFile(infile, getOutFile(argv[2], 1), 1);
        }else if(strcmp(argv[1], "-d") == 0) {
            FILE *infile = fopen(argv[2], "rb");
            processFile(infile, getOutFile(argv[2], 0), 0);
        }else{
            printHelp(argv[0]);
        }
    }

    printf("\nElapsed: %fs\n", (float)(clock() - last) / CLOCKS_PER_SEC);

    return 0;
}
