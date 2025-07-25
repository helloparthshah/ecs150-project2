#include "Deque.h"
#include <stdint.h>
#include <stdlib.h>

Deque *dmalloc() {
  // Allocating the size of the Deque
  Deque *d = (Deque *)malloc(sizeof(Deque));
  if (d != NULL)
    d->head = d->tail = NULL;
  return d;
}

void push_front(volatile Deque *d, TThreadID v) {
  struct Node *n = (struct Node *)malloc(sizeof(struct Node));
  if (n == NULL)
    return;
  // Setting the value of the node
  n->val = v;
  // Adding to the beguining
  n->next = d->head;
  n->prev = NULL;
  // If the only node then set to first
  if (d->tail == NULL) {
    d->head = d->tail = n;
  } else {
    // Set prev of head to node and set head to n
    d->head->prev = n;
    d->head = n;
  }
}

void push_back(volatile Deque *d, TThreadID v) {
  struct Node *n = (struct Node *)malloc(sizeof(struct Node));
  if (n == NULL)
    return;
  // Setting the value of the node
  n->val = v;
  n->prev = d->tail;
  n->next = NULL;
  // If the only node then set to first
  if (d->head == NULL) {
    d->head = d->tail = n;
  } else {
    // Set next of tail to node and set tail to n
    d->tail->next = n;
    d->tail = n;
  }
}

int isEmpty(volatile Deque *d) {
  // Checking if head is null
  if (d->head == NULL)
    return 1;
  return 0;
}

void removeT(volatile Deque *d, TThreadID v) {
  if (d->head == NULL) {
    return;
  }
  struct Node *n = d->head;

  // if first is head then found
  if (n->val == v) {
    d->head = n->next;
    return;
  }

  // traverse through the deque till thread found
  while (n->next != NULL && n->val != v) {
    n = n->next;
  }
  // if found
  if (n->val == v) {
    // Remove node
    n->prev->next = n->next;
    if (n->next != NULL)
      n->next->prev = n->prev;
    else
      d->tail = n->prev;
  }
  free(n);
  return;
}

TThreadID pop_front(volatile Deque *d) {
  // Get value
  TThreadID v = d->head->val;
  struct Node *n = d->head;
  if (d->head == d->tail)
    d->head = d->tail = NULL;
  else
    // Set head to next
    d->head = n->next;
  free(n);
  return v;
}

TThreadID pop_back(volatile Deque *d) {
  TThreadID v = d->tail->val;
  struct Node *n = d->tail;
  if (d->head == d->tail)
    d->head = d->tail = NULL;
  else
    // Set tail to prev
    d->tail = n->prev;
  free(n);
  return v;
}

TThreadID front(volatile Deque *d) {
  // Return head
  return d->head->val;
}

TThreadID end(volatile Deque *d) {
  // Return tail
  return d->tail->val;
}

void writei(uint32_t, uint32_t);

// Debug function to print the deque
void print(volatile Deque *d, uint32_t line) {
  if (d->head == NULL) {
    return;
  }
  struct Node *n = d->head;
  while (n != NULL) {
    writei(n->val, line++);
    n = n->next;
  }
  free(n);
}