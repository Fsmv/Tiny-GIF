#ifndef GIF_H
#define GIF_H

#include <stddef.h>

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
typedef struct __attribute__((__packed__)) {
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
} Gif;

/**
 * Initializes the header data for a gif file
 *
 * @param gif structure to store data in
 * @param width width of the image
 * @param height height of the image
 * @param colorTable colors to use
 * @param numColors number of colors in the table (must be power of 2)
 * @param numRepeats number of times to loop the animation
 */
extern void GIF_Init(Gif *gif, const unsigned short width, const unsigned short height,
                     const unsigned char *colorTable, const unsigned char numColors,
                     const unsigned short numRepeats);

/**
 * Adds an image to the gif animation
 *
 * @param gif gif to add the image to
 * @param data array of color codes to add to the image, must be
 * gif->width*gif->height elemets long
 * @param delayTime ammount of time to show this frame in hundredths of a second
 */
extern void GIF_AddImage(Gif *gif, const unsigned char *data, const unsigned short delayTime);

/**
 * Writes a gif to a file
 *
 * @param gif data to write
 * @param fileName file to write to
 */
extern void GIF_Write(Gif *gif, const char *fileName);

/**
 * Deallocates gif data
 */
extern void GIF_Free(Gif *gif);

#endif
