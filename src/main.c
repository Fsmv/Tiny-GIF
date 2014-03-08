/**
 * Creates an uncompressed gif file which displays an animation of conway's
 * game of life.
 *
 * Avoids using LZW because of the code length requirement, although I could
 * have added a hash map and an LZW function.
 *
 * Author: Andrew Kallmeyer
 * Created on 2014-2-1
 */

#include "Gif.h"

static const unsigned short WIDTH  = 250;
static const unsigned short HEIGHT = 250;
static const unsigned short DELAY_TIME = 100/10; //100/FPS

static const unsigned char numColors = 4;
static const unsigned char COLOR_TABLE[12] = {
    0x00, 0x00, 0x00, //black
    0xFF, 0xFF, 0xFF, //white
    0xFF, 0xAA, 0x00, //orange
    0x00, 0x00, 0x00};//black, unused

//Initial block size for LZW (which I'm subverting to make an uncompressed gif)
#define CODE_SIZE 7
//2^n and 2^n + 1 are taken
#define LZW_FIRST_INDEX (1 << CODE_SIZE) + 2

int main(int argc, char *argv[]) {
    /*Gif gif;
    fillGifHeader(&gif);
    gif.colorTable = COLOR_TABLE;

    Image image[2];
    fillImageHeader(image);
    fillImageHeader(image+1);

    gif.images = image;
    gif.numFrames = 2;

    char frame[WIDTH*HEIGHT];
    memset(frame, 2, WIDTH*HEIGHT);
    image[0].imageData = splitDataBlocks(frame, WIDTH*HEIGHT, &(image[0].numBlocks));
    memset(frame, 1, WIDTH*HEIGHT);
    image[1].imageData = splitDataBlocks(frame, WIDTH*HEIGHT, &(image[1].numBlocks));

    writeToFile(&gif, "./out.gif");

    freeImageData(image[0].imageData, image[0].numBlocks);
    freeImageData(image[1].imageData, image[1].numBlocks);*/

    return 0;
}
