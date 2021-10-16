#ifndef DEQUE
#define DEQUE
#include "RVCOS.h"
#include <stdlib.h>

struct Thread {
  TThreadID id;
  TMemorySize memSize;
  TThreadPriority priority;
  TThreadState state;
};

struct Node {
  struct Node *next;
  struct Node *prev;
  struct Thread val;
};

struct Deque {
  struct Node *head;
  struct Node *tail;
};

struct Deque *dmalloc();

void push_front(struct Deque *d, struct Thread v);

void push_back(struct Deque *d, struct Thread v);

struct Thread pop_front(struct Deque *d);

struct Thread pop_back(struct Deque *d);

struct Thread front(struct Deque *d);

struct Thread end(struct Deque *d);

#endif