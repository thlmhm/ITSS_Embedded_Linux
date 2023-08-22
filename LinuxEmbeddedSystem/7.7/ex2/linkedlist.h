// linkedlist.h

#ifndef LINKEDLIST_H
#define LINKEDLIST_H


typedef struct node {
    void *data;
    struct node *next;
} Node;

// int compare(void* data1, void* data2);

typedef struct linkedlist {
    Node *head;
    int size;
    int (*compare)(void*, void*);
    void (*free)(void*);
} LinkedList;

LinkedList* createList(int (*compare)(void*, void*), void (*freeData)(void*));
void insertAtBeginning(LinkedList *list, void *data);
void insertAtEnd(LinkedList *list, void *data);
void* searchList(LinkedList* list, void* data);
void printList(LinkedList *list);
void* removeFromList(LinkedList* list, void* data);
void freeList(LinkedList* list);

#endif