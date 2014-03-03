#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LZW.h"

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
    printf("Usage %s [options] [file]\n", name);
    printf("Options:\n");
    printf("\t-c: Compress file (appends .lzw)\n");
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

    if(argc <= 1 || argc != 3) {
        printHelp(argv[0]);
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
