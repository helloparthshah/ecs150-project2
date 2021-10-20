#include "Deque.h"

Deque *dmalloc() {
  Deque *d = (Deque *)malloc(sizeof(Deque));
  if (d != NULL)
    d->head = d->tail = NULL;
  return d;
}

void push_front(Deque *d, Thread v) {
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

void push_back(Deque *d, Thread v) {
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

Thread pop_front(Deque *d) {
  Thread v = d->head->val;
  struct Node *n = d->head;
  if (d->head == d->tail)
    d->head = d->tail = NULL;
  else
    d->head = n->next;
  free(n);
  return v;
}

Thread pop_back(Deque *d) {
  Thread v = d->tail->val;
  struct Node *n = d->tail;
  if (d->head == d->tail)
    d->head = d->tail = NULL;
  else
    d->tail = n->prev;
  free(n);
  return v;
}

Thread front(Deque *d) { return d->head->val; }

Thread end(Deque *d) { return d->tail->val; }