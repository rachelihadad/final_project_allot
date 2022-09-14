#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"

void insertToList(struct list *linkedList, struct listelement *newElement)
{
    // pthread_mutex_lock(listLock);
    newElement->prev = NULL;
    if (linkedList->head == NULL)
        linkedList->head = linkedList->tail = newElement;
    else
    {
        struct listelement *temporaryElement = linkedList->head;
        newElement->next = temporaryElement;
        temporaryElement->prev = newElement;
        linkedList->head = newElement;
    }
    // pthread_cond_signal(cv);
    // pthread_mutex_unlock(listLock);
    return;
}
void insertToFrontFromList(struct list *linkedList, struct listelement *newElement)
{
    // pthread_mutex_lock(listLock);
    if (newElement->prev == NULL)
        return;
    if (newElement->next != NULL)
        newElement->next->prev = newElement->prev;
    newElement->prev->next = newElement->next;

    newElement->prev = NULL;
    if (linkedList->head == NULL)
        linkedList->head = linkedList->tail = newElement;
    else
    {
        struct listelement *temporaryElement = linkedList->head;
        newElement->next = temporaryElement;
        temporaryElement->prev = newElement;
        linkedList->head = newElement;
    }
    // pthread_cond_signal(cv);
    // pthread_mutex_unlock(listLock);
    return;
}
void initEmptyList(struct list *linkedList,int count)
{
    struct listelement *firstElement = (struct listelement *)malloc(sizeof(struct listelement));
    firstElement->val = 1;
    firstElement->prev = NULL;
    firstElement->next = NULL;
    linkedList->head = firstElement;
    struct listelement *prevElement = firstElement;
    for (size_t i = 2; i < count; i++)
    {
        struct listelement *newElement = (struct listelement *)malloc(sizeof(struct listelement));
        newElement->val = i;
        newElement->next = NULL;
        newElement->prev = prevElement;
        prevElement->next = newElement;
        prevElement = newElement;
    }
    linkedList->tail = prevElement;
}
struct listelement *pullFromList(struct list *linkedList)
{
    // pthread_mutex_lock(listLock);
    //  while (linkedList->head == NULL)
    //      pthread_cond_wait(cv, listLock);
    struct listelement *temporaryElement = linkedList->tail;
    if (linkedList->head == linkedList->tail)
        linkedList->head = linkedList->tail = NULL;
    else
    {
        temporaryElement->prev->next = NULL;
        linkedList->tail = temporaryElement->prev;
        // linkedList->tail->prev->next = NULL;
        // linkedList->tail=linkedList->tail->prev;
    }
    // pthread_mutex_unlock(listLock);
    return temporaryElement;
}
struct listelement *getEndElementList(struct list *linkedList)
{
    if (linkedList->head == NULL)
        return NULL;
    return linkedList->tail;
}
