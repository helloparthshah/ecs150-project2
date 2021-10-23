#ifndef DEQUE
#define DEQUE
#include "RVCOS.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct Node {
  struct Node *next;
  struct Node *prev;
  TThreadID val;
};

typedef struct {
  struct Node *head;
  struct Node *tail;
} Deque;

typedef struct {
  uint32_t *ctx;
  TThreadEntry entry;
  void *param;
  TThreadID id;
  TMemorySize memsize;
  TThreadPriority priority;
  TThreadState state;
  Deque *waited_by;
  uint32_t return_val;
} Thread;

Deque *dmalloc();

void print(Deque *d, uint32_t line);

void push_front(Deque *d, TThreadID v);

void push_back(Deque *d, TThreadID v);

void removeT(Deque *d, TThreadID v);

int isEmpty(Deque *d);

TThreadID pop_front(Deque *d);
TThreadID pop_back(Deque *d);
TThreadID front(Deque *d);
TThreadID end(Deque *d);

#endif