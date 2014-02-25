#include <stdlib.h> //malloc
#include <string.h> //memcpy
#include "Dictionary.h"

void dict_init(Node *head) {
    head->val = head->index = -1;
    head->numChildren = ALPHABET_SIZE;
    head->children = malloc(sizeof(Node)*ALPHABET_SIZE);

    int i;
    for(i = 0; i < ALPHABET_SIZE; i++) {
        head->children[i].val = head->children[i].index = i;
        head->children[i].numChildren = 0;
    }
}

void node_add(Node *node, const uint8_t value, const uint8_t pos) {
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

int dict_add(Node *head, const uint8_t *value, const size_t valuec) {
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
        //printf("Invalid dictionary value\n");
        return -1;
    }
}

int dict_contains(const Node *head, const uint8_t *value, const size_t valuec) {
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
