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

Deque *dmalloc() {
  Deque *d = (Deque *)malloc(sizeof(Deque));
  if (d != NULL)
    d->head = d->tail = NULL;
  return d;
}

void push_front(Deque *d, TThreadID v) {
  struct Node *n = (struct Node *)malloc(sizeof(struct Node));
  if (n == NULL)
    return;
  n->val = v;
  n->next = d->head;
  n->prev = NULL;
  if (d->tail == NULL) {
    d->head = d->tail = n;
  } else {
    d->head->prev = n;
    d->head = n;
  }
}

void push_back(Deque *d, TThreadID v) {
  struct Node *n = (struct Node *)malloc(sizeof(struct Node));
  if (n == NULL)
    return;
  n->val = v;
  n->prev = d->tail;
  n->next = NULL;
  if (d->head == NULL) {
    d->head = d->tail = n;
  } else {
    d->tail->next = n;
    d->tail = n;
  }
}

int isEmpty(Deque *d) {
  if (d->head == NULL)
    return 1;
  return 0;
}

void removeT(Deque *d, TThreadID v) {
  if (d->head == NULL) {
    return;
  }
  struct Node *n = d->head;

  if (n->val == v) {
    d->head = n->next;
    return;
  }

  while (n->next != NULL && n->val != v) {
    n = n->next;
  }
  if (n->val == v) {
    n->prev->next = n->next;
    if (n->next != NULL)
      n->next->prev = n->prev;
  }
  free(n);
  return;
}

TThreadID pop_front(Deque *d) {
  TThreadID v = d->head->val;
  struct Node *n = d->head;
  if (d->head == d->tail)
    d->head = d->tail = NULL;
  else
    d->head = n->next;
  free(n);
  return v;
}

TThreadID pop_back(Deque *d) {
  TThreadID v = d->tail->val;
  struct Node *n = d->tail;
  if (d->head == d->tail)
    d->head = d->tail = NULL;
  else
    d->tail = n->prev;
  free(n);
  return v;
}

TThreadID front(Deque *d) { return d->head->val; }

TThreadID end(Deque *d) { return d->tail->val; }

void writei(uint32_t, uint32_t);
void print(Deque *d, uint32_t line) {
  if (d->head == d->tail) {
    writei(-1, line);
    return;
  }
  struct Node *n = d->head;
  while (n != NULL) {
    printf("%i", n->val);
    n = n->next;
  }
  free(n);
}

int main() {
  Deque *d = dmalloc();
  push_back(d, 0);
  push_back(d, 1);
  push_back(d, 2);
  removeT(d, 2);
  print(d, 0);
  return 1;
}