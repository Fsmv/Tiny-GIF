#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "Gif.h"
#include "LZW.h"

//Can't have the \0, so I have to initialize as actual char arrays
static const char SIGNATURE[3] = {'G', 'I', 'F'};
static const char VERSION[3] = {'8', '9', 'a'};
static const char INTRODUCER = 0x21; //extension introducer
static const char GCE_LABEL = 0xF9;  //Graphic Control Extension label
static const char SEPARATOR = 0x2C;  //image block separator
static const short TRAILER = 0x3B00; //End of block + gif trialer (little endian)
static const unsigned char BLOCK_SIZE = 127; //2^LZWCodeSize - 1
static const char REPEAT_HEADER_SIZE = 16; //omitting the last 3 bytes
static const char REPEAT_HEADER[19] = {
    0x21, 0xFF, //application block flags
    0x0B,       //block is 11 bytes
    'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E', '2', '.', '0', //name+code
    0x03,       //3 bytes of data left
    0x01};      //sub-block index
    //These bytes are written dynamically
    //REPEAT_TIMES >> 8, REPEAT_TIMES & 0xFF, //number of repeats
    //0x00};      //end

typedef struct __attribute__((__packed__)) {
    unsigned char blockSize;
    unsigned char *data;
} DataBlock;

typedef struct __attribute__((__packed__)) {
    //Graphic Control Extension Block
    char introducer;
    char label;
    char size;
    char gceFlags;
    unsigned short delayTime;
    char transparentColor;
    char gceTerminator;
    //header
    char separator;
    unsigned short x;
    unsigned short y;
    unsigned short width;
    unsigned short height;
    char imgFlags;

    //image data
    char LZWMinCodeSize;
    DataBlock *imageData;
    size_t numBlocks;
} Image;

//gif89a specification http://www.w3.org/Graphics/GIF/spec-gif89a.txt
struct __attribute__((__packed__)) Gif_priv {
    //header
    char signature[3];    //Header signature "GIF"
    char version[3];      //GIF version "89a"

    //Screen descriptor
    unsigned short width;          //width of the image
    unsigned short height;         //height of the image
    char flags;
    char backgroundColor;          //background color index
    char aspectRatio;              //pixel aspect ratio

    const char *colorTable;        //pointer to the global color table
    Image *images;                 //headers and data for each frame
    size_t numFrames;
    unsigned short repeatTimes;
};

void imageInit(const Gif *gif, Image *img, const unsigned short delayTime) {
    //all of our frames fill the screen
    img->x = img->y = 0;
    img->width = gif->width;
    img->height = gif->height;

    img->separator = SEPARATOR;
    img->imgFlags = 0;
    img->LZWMinCodeSize = (gif->flags & 0xF) + 1; 

    //GCE data
    img->introducer = INTRODUCER;
    img->label = GCE_LABEL;
    img->size = 4;            //there are 4 bytes in this header
    img->gceFlags = 0;
    img->delayTime = delayTime; //hundredths of a second
    img->transparentColor = 0;
    img->gceTerminator = 0;
}

size_t packData(const uint16_t *compressedData, size_t compressedSize, DataBlock *container) {
    unsigned char *packedData = calloc(compressedSize*2, sizeof(char));
    int packedIndex = 0;
    int bitsWritten = 0;

    int i;
    for(i = 0; i < compressedSize && packedIndex < BLOCK_SIZE; i++) {
        int bitsInNum = floor(log(compressedData[i])/log(2)) + 1;

        if(bitsInNum <= 8 - bitsWritten) {
            packedData[packedIndex] |= compressedData[i] << bitsWritten;
            bitsWritten += bitsInNum;

            if(bitsWritten == 8) {
                bitsWritten = 0;
                packedIndex++;
            }
        }else{
            int bitsUsed = 0;
            while(bitsUsed != bitsInNum) {
                int bitsToWrite = ((bitsInNum - bitsUsed) <= 8 - bitsWritten) ?
                        (bitsInNum - bitsUsed) : (8 - bitsWritten);
                int mask = ((1 << bitsToWrite) - 1) << bitsUsed;
                int num = (compressedData[i] & mask) >> bitsUsed;

                packedData[packedIndex] |= num << bitsWritten;
                bitsWritten += bitsToWrite;
                bitsUsed += bitsToWrite;

                if(bitsWritten == 8) {
                    bitsWritten = 0;
                    packedIndex++;
                }
            }
        }
    }

    //if the above for loop limits at BLOCK_SIZE this will always be false
    if(bitsWritten != 0) {
        packedIndex++;
    }

    container->data = realloc(packedData, packedIndex);
    container->blockSize = packedIndex;
    return i;
}

DataBlock *splitDataBlocks(const char *frame, size_t size, const char codeSize,  size_t *numBlocks) {
    uint16_t *compressedData;
    size_t compressedSize;
    LZW_Compress(frame, size, &compressedData, &compressedSize, (1 << codeSize) - 1);

    *numBlocks = compressedSize/BLOCK_SIZE + 1;

    //allocate an array of data blocks
    DataBlock *result = malloc(sizeof(DataBlock) * (*numBlocks));


    //copy all the blocks that fill the max size
    int i;
    for(i = 0; i < *numBlocks && compressedSize > 0; i++) {
        int shortsUsed = packData(compressedData,
                compressedSize <= BLOCK_SIZE ? compressedSize : BLOCK_SIZE,
                result + i);
        compressedData += shortsUsed;
        compressedSize -= shortsUsed;
    }

    DataBlock *last = result + i - 1;
    last->data = realloc(last->data, last->blockSize + 1);
    last->data[last->blockSize] = (1 << codeSize) + 1;
    last->blockSize++;

    return result;
}

void freeImage(Image *image) {
    int i;
    for(i = 0; i < image->numBlocks; i++) {
        free(image->imageData[i].data);
    }

    free(image->imageData);
}

Gif *GIF_Init(const unsigned short width, const unsigned short height,
              const unsigned char *colorTable, const unsigned char numColors,
              const unsigned short numRepeats) {
    Gif *gif = malloc(sizeof(Gif));

    memcpy(gif->signature, SIGNATURE, 3);
    memcpy(gif->version, VERSION, 3);

    gif->width = width;
    gif->height = height;
    gif->flags = 0xF0 | (char) floor(log(numColors)/log(2)) - 1;

    gif->backgroundColor = 0;  //bacground color is the first one
    gif->aspectRatio = 0;      //aspect ratio is square

    gif->colorTable = colorTable;
    gif->images = NULL;
    gif->numFrames = 0;
    gif->repeatTimes = numRepeats;

    return gif;
}

void GIF_AddImage(Gif *gif, const unsigned char *data, const unsigned short delayTime) {
    //TODO: find a way to allow a static array size from the beginning for speed
    //resize the images array
    if(gif->numFrames == 0 || gif->images == NULL) {
        gif->images = malloc(sizeof(Image));
    }else{
        gif->images = realloc(gif->images, sizeof(Image) * (gif->numFrames + 1));
    }

    imageInit(gif, gif->images + gif->numFrames, delayTime);
    gif->images[gif->numFrames].imageData = splitDataBlocks(data,
            gif->width*gif->height,
            gif->images[gif->numFrames].LZWMinCodeSize,
            &(gif->images[gif->numFrames].numBlocks));

    gif->numFrames++;
}

void GIF_Write(const Gif *gif, const char *fileName) {
    FILE *file = fopen(fileName, "wb");

    if(!file) {
        printf("Could not write file: %s\n", fileName);
        abort();
    }

    fwrite(gif, sizeof(Gif) - sizeof(char*) - sizeof(Image*) - sizeof(size_t) - sizeof(short),
            1, file); //write header
    //color table size is 3*(2^(colorSizeFlag + 1)) (3 bytes per color)
    fwrite(gif->colorTable, 3*(1 << ((gif->flags & 0xf) + 1)), 1, file); //write color table

    if(gif->repeatTimes > 0) {
        fwrite(REPEAT_HEADER, REPEAT_HEADER_SIZE, 1, file);
        fputc(gif->repeatTimes >> 8, file);   //first byte of repeatTimes
        fputc(gif->repeatTimes & 0xFF, file); //second byte of repeatTimes
        fputc(0x00, file);                    //end header
    }

    //write each image
    int i;
    for(i = 0; i < gif->numFrames; i++) {
        fwrite(gif->images+i, sizeof(Image) - sizeof(DataBlock*)
                - sizeof(size_t), 1, file);

        //write each data block
        int j;
        for(j = 0; j < gif->images[i].numBlocks; j++) {
            fwrite(&gif->images[i].imageData[j].blockSize, 1, 1, file);

            fwrite(gif->images[i].imageData[j].data,
                    gif->images[i].imageData[j].blockSize, 1, file);
        }
    }

    fwrite(&TRAILER, 2, 1, file); //write trailer

    fclose(file);
}

void GIF_Free(Gif *gif) {
    int i;
    for(i = 0; i < gif->numFrames; i++) {
        freeImage(gif->images + i);
    }

    free(gif->images);
    gif->images = NULL;

    free(gif);
}
