#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "LZW.h"

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

uint16_t LZW_CompressOne(const char data, LZW *state) {
    if(state->dict == NULL || state->currSym == NULL) {
        //This is the first byte, its an error to not have both NULL
        dict_init(state->dict, state->alphabetSize);
        state->currSym = malloc(sizeof(uint8_t) * state->symLen);
    }else if(state->dict->currIndex == MAX_INDEX || state->dict->currIndex == -1) {
        //if we go over the max size of the variable reset the dictionary
        dict_free(state->dict);
        dict_init(state->dict, state->alphabetSize);
        //return the clear code, caller must call with the same data again
        return state->alphabetSize + 1;
    }

    //append the data to the current symbol
    setChar((char **) &state->currSym, &state->symLen, state->symIndex, data);
    state->symIndex++;

    if(!dict_contains(state->dict, state->currSym, state->symIndex)) {
        //set the current symbol to last char read
        state->currSym[0] = data;
        state->symIndex = 1;

        //If the dictionary does not cantain the symbol, add it
        return dict_add(state->dict, state->currSym, state->symIndex);
    }
}

uint16_t LZW_Free(LZW *state) {
    setChar((char**) &state->currSym, &state->symLen, state->symIndex, '\0');
    uint16_t result = dict_add(state->dict, state->currSym, state->symIndex + 1);

    dict_free(state->dict);
    free(state->currSym);

    return result;
}

void LZW_Compress(const char *string, size_t stringc, uint16_t **code, size_t *codec, uint8_t alphabetSize) {
    LZW state;
    LZW_Init(alphabetSize, &state);

    size_t resultLen = 64;
    uint16_t *result = malloc(sizeof(uint16_t) * resultLen);
    size_t resultIndex = 1;
    setCode(&result, &resultLen, resultIndex-1, alphabetSize+1); //first code should be clear

    size_t charIndex;
    for(charIndex = 0; charIndex < stringc; charIndex++) {
        uint16_t nextCode = LZW_CompressOne(string[charIndex], &state);
        if(nextCode == alphabetSize + 1) {
            setCode(&result, &resultLen, resultIndex, nextCode);
            resultIndex++;
            nextCode = LZW_CompressOne(string[charIndex], &state);
        }

        setCode(&result, &resultLen, resultIndex, nextCode);
        resultIndex++;
    }


    uint16_t lastCode = LZW_Free(&state);
    setCode(&result, &resultLen, resultIndex, lastCode);
    resultIndex++;

    *code = result;
    *codec = resultIndex;
}

void LZW_Decompress(const uint16_t *code, const size_t codec, char **string, uint8_t alphabetSize) {
    Dictionary dict;
    dict_init(&dict, alphabetSize);

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
        if(code[i] == dict.clearCode) {
            dict_free(&dict);
            dict_init(&dict, alphabetSize);
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
