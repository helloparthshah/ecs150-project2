#ifndef DEQUE
#define DEQUE
#include "RVCOS.h"
#include <stdint.h>
#include <stdlib.h>

typedef struct {
  uint32_t *sp;
  TThreadEntry entry;
  void *param;
  TThreadID id;
  TMemorySize memSize;
  TThreadPriority priority;
  TThreadState state;
} Thread;

struct Node {
  struct Node *next;
  struct Node *prev;
  Thread val;
};

typedef struct {
  struct Node *head;
  struct Node *tail;
} Deque;

Deque *dmalloc();

void push_front(Deque *d, Thread v);

void push_back(Deque *d, Thread v);

Thread pop_front(Deque *d);

Thread pop_back(Deque *d);

Thread front(Deque *d);

Thread end(Deque *d);

#endif