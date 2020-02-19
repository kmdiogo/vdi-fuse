#include "../../include/DataStructures/linked_list.h"

static void freeNodes(struct Node* head);
static struct Node* createNode(void* value);

struct List* createList() {
    struct List* list = malloc(sizeof(struct List));
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

void add(struct List* list, void* value) {
    if (list->tail == NULL) {
        list->head = createNode(value);
        list->tail = list->head;
        list->size++;
        return;
    } else {
        list->tail->next = createNode(value);
        list->tail = list->tail->next;
        list->size++;
    }
}

void destroyList(struct List* list) {
    if (list->head != NULL) freeNodes(list->head);
    free(list);
}


struct Node* createNode(void* value) {
    struct Node* node = malloc(sizeof(struct Node));
    node->value = value;
    node->next = NULL;
    return node;
}

static void freeNodes(struct Node* head) {
    if (head->next != NULL) {
        freeNodes(head->next);
    }
    free(head);
}



