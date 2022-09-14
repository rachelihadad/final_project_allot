#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

// pthread_mutex_t listLock = PTHREAD_MUTEX_INITIALIZER;
// pthread_cond_t cv;
struct listelement
{
  int val;
  struct listelement *next;
  struct listelement *prev;
};
struct list
{
  struct listelement *head;
  struct listelement *tail;
  int count;
};

void insertToList(struct list *linkedList, struct listelement *newElement);
void insertToFrontFromList(struct list *linkedList, struct listelement *newElement);
void initEmptyList(struct list *linkedList,int count);
struct listelement * pullFromList(struct list *linkedList);
struct listelement * getEndElementList(struct list *linkedList);
