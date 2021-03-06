#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <stdint.h>
#include <stddef.h>

typedef struct NodeT {
    uint8_t val;
    uint16_t index;
    uint8_t numChildren;
    struct NodeT *children;
} Node;

typedef struct {
    Node *head;
    uint16_t currIndex;
    uint16_t clearCode;
} Dictionary;

/**
 * Initializes the tree with the first tier alphabet
 *
 * @param head a node to make the head of the tree
 * @param alphabetSize the size of the alphabet to use
 */
extern void tree_init(Node *head, uint8_t alphabetSize);
extern void dict_init(Dictionary *dict, uint8_t alphabetSize);

/**
 * Deallocates the entire tree
 *
 * leaves the head node allocated (with all of its children gone)
 * @param head the tree to deallocate
 */
extern void tree_free(Node *head);
extern void dict_free(Dictionary *dict);

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
extern uint16_t tree_add(Node *head, const uint8_t *code, const size_t codec, uint16_t *currIndex);
extern uint16_t dict_add(Dictionary *dict, const uint8_t *code, const size_t codec);

/**
 * Determines if a code string is in the dictionary or not
 *
 * @param head tree to check
 * @param code the path in the tree to search for
 * @param codec the size of the code array
 * @return 1 if the dictionary contains the value 0 otherwise
 */
extern int tree_contains(const Node *head, const uint8_t *code, const size_t codec);
extern int dict_contains(const Dictionary *dict, const uint8_t *code, const size_t codec);

/**
 * Finds the code phrase that leads to the index
 *
 * Code will be dynamically allocated
 *
 * @param head tree to search
 * @param index the index to search for
 * @param code the result code phrase, must be initialized to NULL
 * @param codec the result code phrase length
 * @return 1 if the index was found, 0 otherwise
 */
extern int tree_search(const Node *head, const uint16_t index, uint8_t **code, size_t *codec);
extern int dict_search(const Dictionary *dict, const uint16_t index, uint8_t **code, size_t *codec);

/**
 * Adds a code as a child of the specified node at the specified index
 *
 * @param node the node to add to
 * @param code the child value to add
 * @param pos the index to add the child at
 */
extern void node_add(Node *node, const uint8_t code, const uint8_t pos, uint16_t *currIndex);

#endif
