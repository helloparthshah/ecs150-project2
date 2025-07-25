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
  int sleep_for;
  int is_sleeping;
} Thread;

Deque *dmalloc();

void print(volatile Deque *d, uint32_t line);

void push_front(volatile Deque *d, TThreadID v);
void push_back(volatile Deque *d, TThreadID v);

void removeT(volatile Deque *d, TThreadID v);

int isEmpty(volatile Deque *d);

TThreadID pop_front(volatile Deque *d);
TThreadID pop_back(volatile Deque *d);

TThreadID front(volatile Deque *d);
TThreadID end(volatile Deque *d);

#endif