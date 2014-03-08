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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define WIDTH  250
#define HEIGHT 250
#define FPS    10
//gifs don't repeat forever
#define REPEAT_TIMES 0xffff
//actual size = 2^(TABLE_SIZE + 1)
#define TABLE_SIZE 1

static const char COLOR_TABLE[(1 << (TABLE_SIZE + 1))*3] = {
    0x00, 0x00, 0x00, //black
    0xFF, 0xFF, 0xFF, //white
    0xFF, 0xAA, 0x00, //orange
    0x00, 0x00, 0x00};//black, unused

//********** change only constants above here *********

//Initial block size for LZW (which I'm subverting to make an uncompressed gif)
#define CODE_SIZE 7
//2^n and 2^n + 1 are taken
#define LZW_FIRST_INDEX (1 << CODE_SIZE) + 2

//Can't have the \0, so I have to initialize as actual char arrays
static const char SIGNATURE[3] = {'G', 'I', 'F'};
static const char VERSION[3] = {'8', '9', 'a'};
static const char INTRODUCER = 0x21; //extension introducer
static const char GCE_LABEL = 0xF9;  //Graphic Control Extension label
static const char SEPARATOR = 0x2C;  //image block separator
static const short TRAILER = 0x3B00; //End of block + gif trialer (little endian)
static const unsigned char BLOCK_SIZE = 127; //2^LZWCodeSize - 1
static const char REPEAT_HEADER_SIZE = 19;
static const char REPEAT_HEADER[19] = {
    0x21, 0xFF, //application block flags
    0x0B,       //block is 11 bytes
    'N', 'E', 'T', 'S', 'C', 'A', 'P', 'E', '2', '.', '0', //name+code
    0x03,       //3 bytes of data left
    0x01,       //sub-block index
    REPEAT_TIMES >> 8, REPEAT_TIMES & 0xFF, //number of repeats
    0x00};      //end

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
} Gif;

void fillGifHeader(Gif *gif) {
    memcpy(gif->signature, SIGNATURE, 3);
    memcpy(gif->version, VERSION, 3);

    gif->width = WIDTH;
    gif->height = HEIGHT;
    gif->flags = 0xF0 | TABLE_SIZE; //global color table with 8 bit colors

    gif->backgroundColor = 0;  //bacground color is the first one
    gif->aspectRatio = 0;      //aspect ratio is square
}

void fillImageHeader(Image *img) {
    //all of our frames fill the screen
    img->x = img->y = 0;
    img->width = WIDTH;
    img->height = HEIGHT;

    img->separator = SEPARATOR;
    img->imgFlags = 0;
    img->LZWMinCodeSize = CODE_SIZE;

    //GCE data
    img->introducer = INTRODUCER;
    img->label = GCE_LABEL;
    img->size = 4;            //there are 4 bytes in this header
    img->gceFlags = 0;
    img->delayTime = 100/FPS; //hundredths of a second
    img->transparentColor = 0;
    img->gceTerminator = 0;
}

void writeToFile(Gif *gif, const char *fileName) {
    FILE *file = fopen(fileName, "wb");

    if(!file) {
        printf("Could not write file: %s\n", fileName);
        abort();
    }

    fwrite(gif, sizeof(Gif) - sizeof(char*) - sizeof(Image*) - sizeof(size_t),
            1, file); //write header
    fwrite(gif->colorTable, 3*(1 << (TABLE_SIZE + 1)), 1, file); //write color table

    if(REPEAT_TIMES > 0) {
        fwrite(REPEAT_HEADER, REPEAT_HEADER_SIZE, 1, file);
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

//subtracted 2 from blocksize for the CLEAR and STOP codes
DataBlock *splitDataBlocks(const char *frame, size_t size, size_t *numBlocks) {
    *numBlocks = size/(BLOCK_SIZE-1); //-1 because each block needs a CLEAR char
    int lastSize = (size%(BLOCK_SIZE-1)) + 2; //CLEAR in the beginning plus STOP at the end

    //allocate an array of data blocks
    DataBlock *result = malloc(sizeof(DataBlock) * 
            (lastSize > 0 ? *numBlocks+1 : *numBlocks));

    //copy all the blocks that fill the max size
    int i;
    for(i = 0; i < *numBlocks; i++) {
        result[i].blockSize = BLOCK_SIZE;
        result[i].data = malloc(BLOCK_SIZE);
        memcpy(result[i].data+1, frame + (i*(BLOCK_SIZE-1)), BLOCK_SIZE-1);

        result[i].data[0] = 1 << CODE_SIZE; //write clear to reset the LZW table
    }

    //copy the last block too if necessary
    if(lastSize > 0) {
        result[*numBlocks].blockSize = lastSize;
        result[*numBlocks].data = malloc(lastSize);
        memcpy(result[*numBlocks].data+1, frame + (*numBlocks*(BLOCK_SIZE-1)), lastSize-2);

        result[*numBlocks].data[0] = 1 << CODE_SIZE; //clear
        result[*numBlocks].data[lastSize-1] = result[*numBlocks].data[0]+1; //stop

        (*numBlocks)++;
    }

    return result;
}

void freeImageData(DataBlock *blocks, size_t size) {
    int i;
    for(i = 0; i < size; i++) {
        free(blocks[i].data);
    }

    free(blocks);
}


typedef struct NodeT {
    int val, index;
    size_t numChildren;
    struct NodeT *children;
} Node;

void dict_init(Node *head, const int numColors) {
    head->val = head->index = -1;
    head->numChildren = numColors;
    head->children = malloc(sizeof(Node)*numColors);

    int i;
    for(i = 0; i < numColors; i++) {
        head->children[i].val = head->children[i].index = i;
        head->children[i].numChildren = 0;
    }
}

void node_add(Node *node, const int value, const int pos) {
    //this is the first value I can use in GIF LZW
    static int currIndex = LZW_FIRST_INDEX;

    Node *temp = malloc(sizeof(Node)*(node->numChildren + 1));
    //copy all of the nodes with a value less than this one
    memcpy(temp, node->children, sizeof(Node)*pos);

    //write the new node
    temp[pos].val = value;
    temp[pos].index = currIndex++;
    temp[pos].numChildren = 0;

    //copy the rest of the children into the new array
    memcpy(temp+pos+1, node->children+pos, sizeof(Node)*(node->numChildren - pos));

    if(node->numChildren > 0) {
        free(node->children);
    }
    node->children = temp;
    node->numChildren++;
}

/**
 * Adds a new node to the dictionary and returns the code to output when using
 * LZW
 *
 * @param head the dictionary to process
 * @param value the array of values that represents the address into the
 * dictionary to add
 * @return the index of the node above the new value (the LZW code to output)
 */
int dict_add(Node *head, const int *value, const int valuec) {
    if(head->numChildren == 0 && valuec == 1) {
        node_add(head, value[0], 0);
        return head->index;
    }

    int i;
    for(i = 0; i < head->numChildren; i++) {
        if(head->children[i].val == value[0]) {
            if(valuec == 1) { //the child already exists
                return head->index;
            }else{ //we have to go deeper
                return dict_add(&head->children[i], value + 1, valuec - 1);
            }
        }else if(head->children[i].val > value[0]) { //val is not a child of head
            if(valuec == 1) { //add the node as head's child
                node_add(head, value[0], i);
                return head->index;
            }else{
                //technically I could handle this case but it won't happen in LZW
                break;
            }
        }
    }

    if(valuec == 1) {
        //the new value is the biggest of the children
        node_add(head, value[0], head->numChildren);
        return head->index;
    }else{
        printf("Invalid dictionary value\n");
        return -1;
    }
}

/**
 * @return 1 if the dictionary contains the value 0 otherwise
 */
int dict_contains(const Node *head, const int *value, const int valuec) {
    if(valuec <= 0) {
        return 0;
    }

    int i;
    for(i = 0; i < head->numChildren; i++) {
        if(head->children[i].val == value[0]) {
            if(valuec == 1) {
                return 1;
            }else{
                return dict_contains(&head->children[i], value + 1, valuec - 1);
            }
        }else if(head->children[i].val > value[0]) {
            return 0;
        }
    }

    return 0;
}

void dict_free(Node *head) {
    int i;
    for(i = 0; i < head->numChildren; i++) {
       if(head->children[i].numChildren > 0) {
            dict_free(&head->children[i]);
       }
    }

    free(head->children);
}

int main(int argc, char *argv[]) {
    //TODO: More test cases?
    Node dict;
    dict_init(&dict, 4);
    int value[4] = {1, 2, 3, 4};
    printf("added %x\n", dict_add(&dict, value, 2)); //correct
    printf("added %x\n", dict_add(&dict, value, 3)); //correct
    printf("added %x\n", dict_add(&dict, value, 4)); //correct

    value[1] = 1;
    printf("added %x\n", dict_add(&dict, value, 2)); //correct

    value[1] = 4;
    printf("added %x\n", dict_add(&dict, value, 2)); //fails "Invalid..."

    value[1] = 3;
    printf("added %x\n", dict_add(&dict, value, 2)); //fails "Invalid..."

    printf("%d\n", dict_contains(&dict, value, 4)); //correct
    value[1] = 1;
    printf("%d\n", dict_contains(&dict, value, 2)); //correct
    value[1] = 2;
    printf("%d\n", dict_contains(&dict, value, 2)); //correct
    value[1] = 3;
    printf("%d\n", dict_contains(&dict, value, 2)); //correct, the add failed
    value[1] = 4;
    printf("%d\n", dict_contains(&dict, value, 2)); //correct, the add failed
    value[1] = 2;
    printf("%d\n", dict_contains(&dict, value, 3)); //correct
    printf("%d\n", dict_contains(&dict, value, 4)); //correct


    dict_free(&dict);

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
