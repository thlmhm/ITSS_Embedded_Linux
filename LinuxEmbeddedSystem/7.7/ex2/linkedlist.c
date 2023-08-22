// linkedlist.c

#include <stdio.h>
#include <stdlib.h>
#include "linkedlist.h"
/*
Initialize a Linked List with Compare and Free Function for Specific Struct
*/
LinkedList* createList(int (*compare)(void*, void*), void (*freeData)(void*)) {
    LinkedList *list = malloc(sizeof(LinkedList));
    list->compare = compare;
    list->free = freeData;
    list->head = NULL;
    list->size = 0;
    return list;
}

void insertAtBeginning(LinkedList *list, void *data) {
    Node *newNode = malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = list->head;
    list->head = newNode;
    list->size++;
}

/*
Insert an Entry to the end of Linked List
*/
void insertAtEnd(LinkedList *list, void *data) {
    if (list->head == NULL) {
        insertAtBeginning(list, data);
    } else {
        Node *newNode = malloc(sizeof(Node));
        newNode->data = data;
        newNode->next = NULL;
        Node *current = list->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
        list->size++;
    }
}

/*
Search an Entry in the Linked List
*/
void* searchList(LinkedList* list, void* data){
    Node *current = list->head;
    while (current != NULL) {
        void* temp = current->data;
        if(list->compare(temp, data) == 0) return temp;
        current = current->next;
    }
    return NULL;
}

/*
Remove and Element which match void* data.
*/
void* removeFromList(LinkedList* list, void* data) {
    Node* current = list->head;
    Node* previous = NULL;

    while (current != NULL) {
        if (list->compare(data, current->data) == 0) {
            if (previous == NULL) {
                list->head = current->next;
            } else {
                previous->next = current->next;
            }
            // list->free(current->data);
            // free(current);
            return (void*) current->data;
        }
        previous = current;
        current = current->next;
    }
    return NULL;
}

void printList(LinkedList *list) {
    Node *current = list->head;
    while (current != NULL) {
        printf("%p ", current->data);
        current = current->next;
    }
    printf("\n");
}

// Free recursively memory allocated to LinkedList's Node and It's Data.
void freeList(LinkedList* list){
    Node *current = list->head;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        void* data = current->data;
        list->free(data);
        free(temp->data);
        free(temp);
    }
}