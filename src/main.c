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

#include <time.h>
#include <stdio.h>
#include <string.h>
#include "Gif.h"

#define WIDTH  250
#define HEIGHT 250
static const unsigned short NUM_ITERATIONS = 124;
static const unsigned short DELAY_TIME = 100/40; //100/FPS

static const unsigned char NUM_COLORS = 4;
static const unsigned char COLOR_TABLE[12] = {
    0x00, 0x00, 0x00, //black
    0xFF, 0xFF, 0xFF, //white
    0xFF, 0xAA, 0x00, //orange
    0x00, 0x00, 0x00};//black, unused

//these states are color table indicies
typedef enum {
    DEAD  = 0,
    ALIVE = 2
} State;

static const unsigned short NUM_REPEATS = 0xFFFF;

unsigned char cells[WIDTH*HEIGHT];

void setCell(int x, int y, State state) {
    if(x >= 0 && x < WIDTH/2 && y >= 0 && y < HEIGHT/2) {
        cells[x*2     + WIDTH * (y*2)    ] = (unsigned char) state;
        cells[x*2 + 1 + WIDTH * (y*2)    ] = (unsigned char) state;
        cells[x*2     + WIDTH * (y*2 + 1)] = (unsigned char) state;
        cells[x*2 + 1 + WIDTH * (y*2 + 1)] = (unsigned char) state;
    }
}

void iterate(unsigned char *cells) {
}

int main(int argc, char *argv[]) {
    time_t last = clock();
    Gif *gif = GIF_Init(WIDTH, HEIGHT, COLOR_TABLE, NUM_COLORS, NUM_REPEATS);
    printf("Init time: %f ms\n", 1000.0*(clock() - last)/CLOCKS_PER_SEC);

    double elapsed = 0.0;

    int off;
    for(off = -3; off < NUM_ITERATIONS; off++) {
        memset(cells, 1, WIDTH*HEIGHT);
        setCell(1 + off, 0 + off, ALIVE);
        setCell(2 + off, 1 + off, ALIVE);
        setCell(0 + off, 2 + off, ALIVE);
        setCell(1 + off, 2 + off, ALIVE);
        setCell(2 + off, 2 + off, ALIVE);

        //iterate(cells);
        last = clock();
        GIF_AddImage(gif, cells, DELAY_TIME);
        elapsed += (double)(clock() - last) / CLOCKS_PER_SEC;
    }

    printf("Time per frame: %f ms\n", 1000.0*elapsed/NUM_ITERATIONS);

    last = clock();
    GIF_Write(gif, "./out.gif");
    printf("Write time: %f ms\n", 1000.0*(clock() - last)/CLOCKS_PER_SEC);
    last = clock();
    GIF_Free(gif);
    printf("Free time: %f ms\n", 1000.0*(clock() - last)/CLOCKS_PER_SEC);

    return 0;
}
