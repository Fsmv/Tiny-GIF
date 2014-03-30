#include <stdlib.h> //malloc
#include <string.h> //memcpy
#include "Dictionary.h"

void tree_init(Node *head, uint8_t alphabetSize) {
    head->val = head->index = -1;
    head->numChildren = alphabetSize;
    head->children = malloc(sizeof(Node)*alphabetSize);

    int i;
    for(i = 0; i < alphabetSize; i++) {
        head->children[i].val = head->children[i].index = i;
        head->children[i].numChildren = 0;
    }
}

void dict_init(Dictionary *dict, uint8_t alphabetSize) {
    dict->head = malloc(sizeof(Node));
    tree_init(dict->head, alphabetSize);
    dict->clearCode = alphabetSize + 1;
    dict->currIndex = alphabetSize + 3; //skip +2 for stop code
}

void node_add(Node *node, const uint8_t value, const uint8_t pos, uint16_t *currIndex) {
    Node *temp = malloc(sizeof(Node)*(node->numChildren + 1));
    //copy all of the nodes with a value less than this one
    memcpy(temp, node->children, sizeof(Node)*pos);

    //write the new node
    temp[pos].val = value;
    temp[pos].index = *currIndex;
    *currIndex += 1;
    temp[pos].numChildren = 0;

    //copy the rest of the children into the new array
    memcpy(temp+pos+1, node->children+pos, sizeof(Node)*(node->numChildren - pos));

    if(node->numChildren > 0) {
        free(node->children);
    }
    node->children = temp;
    node->numChildren++;
}

uint16_t tree_add(Node *head, const uint8_t *value, const size_t valuec, uint16_t *currIndex) {
    if(head->numChildren == 0 && valuec == 1) {
        node_add(head, value[0], 0, currIndex);
        return head->index;
    }

    int i;
    for(i = 0; i < head->numChildren; i++) {
        if(head->children[i].val == value[0]) {
            if(valuec == 1) { //the child already exists
                return head->index;
            }else{ //we have to go deeper
                return tree_add(&head->children[i], value + 1, valuec - 1, currIndex);
            }
        }else if(head->children[i].val > value[0]) { //val is not a child of head
            if(valuec == 1) { //add the node as head's child
                node_add(head, value[0], i, currIndex);
                return head->index;
            }else{
                //technically I could handle this case but it won't happen in LZW
                break;
            }
        }
    }

    if(valuec == 1) {
        //the new value is the biggest of the children
        node_add(head, value[0], head->numChildren, currIndex);
        return head->index;
    }else{
        //printf("Invalid dictionary value\n");
        return -1;
    }
}

uint16_t dict_add(Dictionary *dict, const uint8_t *value, const size_t valuec) {
    return tree_add(dict->head, value, valuec, &dict->currIndex);
}

int tree_contains(const Node *head, const uint8_t *value, const size_t valuec) {
    if(valuec <= 0) {
        return 0;
    }

    size_t start = 0;
    size_t mid = head->numChildren / 2;
    size_t end = head->numChildren;
    while(start == mid || end == mid) {
        if(head->children[mid].val < value[0]) {
            start = mid;
            mid += (end - mid) / 2;
        }else if(head->children[mid].val > value[0]) {
            end = mid;
            mid /= 2;
        }else {
            if(valuec == 1) {
                return 1;
            }else{
                return tree_contains(&head->children[mid], value + 1, valuec - 1);
            }
        }
    }

    return 0;
}

int dict_contains(const Dictionary *dict, const uint8_t *value, const size_t valuec) {
    return tree_contains(dict->head, value, valuec);
}

int tree_search(const Node *head, const uint16_t index, uint8_t **code, size_t *codec) {
    if(*code == NULL) {
        *code = malloc(sizeof(uint8_t) * 1);
        *codec = 1;
    }

    if(head->numChildren <= 0) {
        return 0;
    }

    //depth first search
    int i;
    for(i = 0; i < head->numChildren; i++) {
        if(head->children[i].index == index) {
            //index found, each layer above will prepend its value
            (*code)[0] = head->children[i].val;
            return 1;
        }else{
            if(tree_search(&(head->children[i]), index, code, codec)) {
                //it was found, on the way up from the recursion prepend this
                //child's value

                (*codec)++;
                uint8_t *result = malloc(sizeof(uint8_t) * (*codec));
                memcpy(result+1, *code, *codec - 1);
                result[0] = head->children[i].val;

                free(*code);
                *code = result;
                return 1;
            }
        }
    }

    return 0;
}

int dict_search(const Dictionary *dict, const uint16_t index, uint8_t **code, size_t *codec) {
    return tree_search(dict->head, index, code, codec);
}

void tree_free(Node *head) {
    int i;
    for(i = 0; i < head->numChildren; i++) {
       if(head->children[i].numChildren > 0) {
            tree_free(&head->children[i]);
       }
    }

    free(head->children);
}

void dict_free(Dictionary *dict) {
    tree_free(dict->head);
    free(dict->head);
}
