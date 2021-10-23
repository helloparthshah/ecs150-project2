#include "Deque.h"

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
  struct Node *n = d->head;
  if (d->head == d->tail)
    d->head = d->tail = NULL;
  while (n != d->tail) {
    if (n->val == v) {
      n->prev->next = n->next;
      n->next->prev = n->prev;
      break;
    }
    n = n->next;
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