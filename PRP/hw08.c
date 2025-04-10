#include "hw08.h"
#include <stdio.h>
#include <string.h>

queue_t *create_queue(int capacity) {
  queue_t *q = malloc(sizeof(queue_t));
  if (q != NULL) {
    void **vals_tmp = malloc(capacity * sizeof(void *));
    if (vals_tmp == NULL) {
      free(q);
      return NULL;
    } else {
      q->vals = vals_tmp;
      q->head = 0;
      q->tail = 0;
      q->size = 0;
      q->capacity = capacity;
    }
  }
  return q;
}

void delete_queue(queue_t *queue) {
  free(queue->vals);
  free(queue);
}

bool resize_queue(queue_t *q, size_t new_capacity) {
  void **vals_tmp = malloc(new_capacity * sizeof(void *));
  if (vals_tmp == NULL) return false;

  for (size_t i = 0; i < q->size; i++) {
    vals_tmp[i] = get_from_queue(q, i);
  }

  free(q->vals);
  q->vals = vals_tmp;
  q->head = 0;
  q->tail = q->size;
  q->capacity = new_capacity;
  return true;
}

bool push_to_queue(queue_t *queue, void *data) {
  if (queue->size == queue->capacity) {
    bool r = resize_queue(queue, queue->capacity * 2);
    if (!r) return false;
  }
  queue->vals[queue->tail] = data;
  queue->tail = (queue->tail + 1) % queue->capacity;
  queue->size++;
  return true;
}

void *pop_from_queue(queue_t *queue) {
  if (queue->size == 0) return NULL;
  void *val = queue->vals[queue->head];
  queue->vals[queue->head] = NULL;
  queue->head = (queue->head + 1) % queue->capacity;
  queue->size--;
  if (queue->size > 0 && queue->size <= queue->capacity / 3) {
    resize_queue(queue, queue->size);
  }
  if (queue->size == 0) queue->tail = 0;
  return val;
}

void *get_from_queue(queue_t *queue, int idx) {
  if (idx < 0 || idx >= queue->size) return NULL;
  void *val = queue->vals[(queue->head + idx) % queue->capacity];
  return val;
}

int get_queue_size(queue_t *queue) { return queue->size; }
