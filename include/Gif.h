#ifndef GIF_H
#define GIF_H

struct Gif_priv;
typedef struct Gif_priv Gif;

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
extern Gif *GIF_Init(const unsigned short width, const unsigned short height,
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
extern void GIF_Write(const Gif *gif, const char *fileName);

/**
 * Deallocates gif data
 */
extern void GIF_Free(Gif *gif);

#endif
