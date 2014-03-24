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

#include "string.h"
#include "Gif.h"

static const unsigned short WIDTH  = 10;
static const unsigned short HEIGHT = 10;
static const unsigned short DELAY_TIME = 100/10; //100/FPS

static const unsigned char numColors = 4;
static const unsigned char COLOR_TABLE[12] = {
    0x00, 0x00, 0x00, //black
    0xFF, 0xFF, 0xFF, //white
    0xFF, 0xAA, 0x00, //orange
    0x00, 0x00, 0x00};//black, unused

static const unsigned short NUM_REPEATS = 0xFFFF;
//Initial block size for LZW (which I'm subverting to make an uncompressed gif)
#define CODE_SIZE 7
//2^n and 2^n + 1 are taken
#define LZW_FIRST_INDEX (1 << CODE_SIZE) + 2

int main(int argc, char *argv[]) {
    Gif *gif = GIF_Init(WIDTH, HEIGHT, COLOR_TABLE, numColors, NUM_REPEATS);

    char frame[WIDTH*HEIGHT];
    memset(frame, 2, WIDTH*HEIGHT);
    GIF_AddImage(gif, frame, DELAY_TIME);
    memset(frame, 1, WIDTH*HEIGHT);
    GIF_AddImage(gif, frame, DELAY_TIME);

    GIF_Write(gif, "./out.gif");

    GIF_Free(gif);

    return 0;
}
