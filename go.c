#include "go.h"
#include <stdlib.h>

queue* put(queue* q, void* val) {
  queue* item = calloc(1, sizeof(queue));
  item->value = val;
  item->next = q;
  return item;
}

queue* get(queue* q, void** val) {
  queue* pq = NULL, *mq = q, *ret = q;
  while (mq->next) {
    pq = mq;
    mq = pq->next;
  }
  *val = mq->value;

  if (ret == mq)
    ret = NULL;
  else
    pq->next = NULL;
  free(mq);
  return ret;
}

void close_chan(chan* c) {
  c->closed = 1;
  sem_post(&c->in);
}

void send_to(chan* c, void* value) {
  sem_wait(&c->out);
  pthread_mutex_lock(&c->lock);
  c->vals = put(c->vals, value);
  pthread_mutex_unlock(&c->lock);
  sem_post(&c->in);
}

void* get_from(chan* c) {
  if (c->closed && !c->vals) {
    return NULL;
  }

  void* val;
  sem_wait(&c->in);

  pthread_mutex_lock(&c->lock);
  if (c->closed && !c->vals) {
    pthread_mutex_unlock(&c->lock);
    sem_post(&c->in);
    return NULL;
  }
  c->vals = get(c->vals, &val);
  pthread_mutex_unlock(&c->lock);
  sem_post(&c->out);
  return val;
}

chan* make_chan(size_t size) {
  chan* c = calloc(1, sizeof(chan));
  sem_init(&c->in, 0, 0);
  sem_init(&c->out, 0, size);
  pthread_mutex_init(&c->lock, NULL);
  return c;
}

void destroy_chan(chan* c) {
  sem_destroy(&c->in);
  sem_destroy(&c->out);
  pthread_mutex_destroy(&c->lock);
  queue* q = c->vals;
  while (q) {
    free(q);
    q = q->next;
  }
  free(c);
}

pthread_t create_thread(void *(*start_routine)(void*), void* arg) {
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, start_routine, arg);
  pthread_detach(thread_id);
  return thread_id;
}
