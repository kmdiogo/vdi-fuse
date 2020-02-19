//
// Created by Kenny on 2/18/2020.
//

#ifndef VDIFUSE_LINKED_LIST_H
#define VDIFUSE_LINKED_LIST_H

#include "stdlib.h"

struct Node {
    struct Node* next;
    void* value;
};

struct List {
    struct Node* head;
    struct Node* tail;
    int size;
};

struct List* createList();
void add(struct List* list, void* value);
void destroyList(struct List* list);


#endif //VDIFUSE_LINKED_LIST_H
