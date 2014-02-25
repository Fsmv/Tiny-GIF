#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <stdint.h>
#include <stddef.h>

#define ALPHABET_SIZE 0xFF
#define CLEAR 0x100
#define LZW_FIRST_INDEX 0x101

typedef struct NodeT {
    uint8_t val;
    uint16_t index;
    uint8_t numChildren;
    struct NodeT *children;
} Node;

/**
 * Initializes the tree with the first tier alphabet
 *
 * @param head a node to make the head of the tree
 */
extern void dict_init(Node *head);

/**
 * Deallocates the entire tree
 *
 * leaves the head node allocated (with all of its children gone)
 * @param head the tree to deallocate
 */
extern void dict_free(Node *head);

/**
 * Adds a new node to the dictionary and returns the code to output when using
 * LZW
 *
 * @param head the dictionary to process
 * @param code the array of values that represents the address into the
 * dictionary to add
 * @param codec the size of the code array
 * @return the index of the node above the new value (the LZW code to output)
 */
extern int dict_add(Node *head, const uint8_t *code, const size_t codec);

/**
 * Determines if a code string is in the dictionary or not
 *
 * @param head tree to check
 * @param code the path in the tree to search for
 * @param codec the size of the code array
 * @return 1 if the dictionary contains the value 0 otherwise
 */
extern int dict_contains(const Node *head, const uint8_t *code, const size_t codec);

/**
 * Adds a code as a child of the specified node at the specified index
 *
 * @param node the node to add to
 * @param code the child value to add
 * @param pos the index to add the child at
 */
extern void node_add(Node *node, const uint8_t code, const uint8_t pos);

#endif
