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
static const char TRAILER = 0x3B;    //End of block + gif trailer (little endian)
static const unsigned char BLOCK_SIZE = 0xFD;
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

static void imageInit(const Gif *gif, Image *img, const unsigned short delayTime) {
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

static int getBitsInNum(int num) {
    return floor(log(num)/log(2)) + 1;
}

/**
 * Takes uncompressed color mappings and compresses it then packs it in the gif
 * data format until it has filled a block or used the last of the data in the
 * frame
 *
 * Frees lzwState when it runs out of data to compress
 *
 * @param frame data to compress
 * @param size size of the frame array
 * @param lzwState pre-initialized state used for this image
 * @param container the DataBlock to store the compressed data (dynamically
 * allocated)
 * @return the number of elements in the frame array used
 */
static size_t packData(const char *frame, size_t size, LZW *lzwState, DataBlock *container) {
    static char overflow = 0;
    static char overflowSize = 0;
    unsigned char *packedData = calloc(BLOCK_SIZE + 1, sizeof(char));
    int bitsWritten = 0;
    if(lzwState->dict == NULL) {
        lzwState->codeSize = getBitsInNum(lzwState->alphabetSize + 1);
    }
    size_t packedIndex = 0;
    size_t frameIndex = 0;
    int writeSTOP = 0;

    //printf("\n");
    while((packedIndex < BLOCK_SIZE && frameIndex <= size) || writeSTOP) {
        uint16_t code;
        if(overflowSize != 0) {
            code = overflow;
            overflow = 0;
            frameIndex--;
        }else if(frameIndex < size) {
            code = LZW_CompressOne(frame[frameIndex], lzwState);
        }else if(!writeSTOP){
            code = LZW_Free(lzwState); //write the last code
            writeSTOP = 1;
        }else{
            code = lzwState->alphabetSize + 2; //write the stop code
            writeSTOP = 0;
        }

        if(code == 0xFFFF) { //no code output
            frameIndex++;
            continue;
        }

        if(code == lzwState->alphabetSize + 1) {
            frameIndex--; //do the same code again, we got the clear code
            lzwState->codeSize = getBitsInNum(lzwState->alphabetSize + 1);
        }

        int bitsInNum = getBitsInNum(code);
        if(overflowSize != 0) {
            bitsInNum = overflowSize;
            overflowSize = 0;
        }

        if(bitsInNum < lzwState->codeSize) {
            bitsInNum = lzwState->codeSize;
        }else{
            lzwState->codeSize = bitsInNum;
            //printf("top: %d, %d\n", frameIndex, lzwState->codeSize);
        }

        //printf("code: %x, size: %d\n", code, lzwState->codeSize);

        if(bitsInNum <= 8 - bitsWritten) {
            packedData[packedIndex] |= code << bitsWritten;
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
                int num = (code & mask) >> bitsUsed;

                packedData[packedIndex] |= num << bitsWritten;
                bitsWritten += bitsToWrite;
                bitsUsed += bitsToWrite;

                if(bitsWritten == 8) {
                    bitsWritten = 0;
                    packedIndex++;

                    if(packedIndex == BLOCK_SIZE) {
                        overflow = code >> bitsUsed;
                        overflowSize = bitsInNum - bitsUsed;
                        break;
                    }
                }
            }
        }

        frameIndex++;
        if(code != 0xffff) {
            lzwState->codeSize = getBitsInNum(lzwState->dict->currIndex - 1);
            //printf("bottom: %d, %d\n", lzwState->dict->currIndex, lzwState->codeSize);
        }
    }

    //if the above for loop limits at BLOCK_SIZE this will always be false
    if(bitsWritten != 0) {
        packedIndex++;
    }

    container->data = realloc(packedData, packedIndex);
    container->blockSize = packedIndex;

    return frameIndex;
}

/**
 * Takes a frame and converts it to an array of compressed and packed data
 * blocks in the gif format.
 *
 * @param frame frame to encode
 * @param size size of the frame
 * @param codeSize initial code size to use for compressing
 * @param container return value, array to store the blocks in
 * @return number of blocks created
 */
static size_t splitDataBlocks(const char *frame, size_t size, const char codeSize, DataBlock **container) {
    //TODO: do size doubling for speed (instead of reallocing by 1 each time)
    //allocate an array of data blocks
    DataBlock *result = malloc(1);
    LZW lzwState;
    LZW_Init((1 << codeSize) - 1, &lzwState);

    //copy all the blocks that fill the max size
    int frameIndex = 0;
    int blockIndex = 0;
    while(frameIndex <= size) {
        result = realloc(result, sizeof(DataBlock) * (blockIndex + 1));
        int numUsed = packData(frame + frameIndex, size - frameIndex, &lzwState, result + blockIndex);

        frameIndex += numUsed;
        blockIndex++;
    }

    *container = result;
    return blockIndex;
}

static void freeImage(Image *image) {
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

    gif->backgroundColor = 0;  //background color is the first one
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
    gif->images[gif->numFrames].numBlocks = splitDataBlocks(data,
            gif->width*gif->height,
            gif->images[gif->numFrames].LZWMinCodeSize,
            &(gif->images[gif->numFrames].imageData));

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

        fputc(0x00, file); //block separator
    }

    fputc(TRAILER, file); //write trailer

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
