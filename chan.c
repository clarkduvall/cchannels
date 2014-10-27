#include <stdlib.h>

#include "chan.h"

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

static void lock(chan* c) {
  int i, j;
  for (i = 0; i < 10; i++) {
    if (pthread_mutex_trylock(&c->lock)) {
      for (j = 0; j < 5; j++)
        __asm__("PAUSE");
    } else {
      return;
    }
  }
  if (pthread_mutex_trylock(&c->lock))
    sched_yield();
  else
    return;
  pthread_mutex_lock(&c->lock);
}

static void unlock(chan* c) {
  pthread_mutex_unlock(&c->lock);
}

static int wait_nb(pthread_cond_t* c, pthread_mutex_t* m) {
  struct timespec ts;
  ts.tv_sec  = 0;
  ts.tv_nsec = 0;
  return pthread_cond_timedwait(c, m, &ts);
}

static void wait(pthread_cond_t* c, pthread_mutex_t* m, chan* ch) {
  if (wait_nb(c, m))
    sched_yield();
}

void close_chan(chan* c) {
  c->closed = 1;
  pthread_cond_broadcast(&c->send);
  pthread_cond_broadcast(&c->recv);
}

int send_to(chan* c, void* value) {
  if (c->closed)
    return 1;

  lock(c);
  while (c->qsize >= c->size) {
    if (c->closed) {
      unlock(c);
      return 1;
    }

    wait(&c->send, &c->lock, c);
  }

  if (c->closed) {
    unlock(c);
    return 1;
  }

  c->vals = put(c->vals, value);
  c->qsize++;
  unlock(c);
  pthread_cond_signal(&c->recv);
  return 0;
}

int get_from(chan* c, void** value) {
  if (c->closed && !c->vals)
    return 1;

  lock(c);
  while (c->qsize <= 0) {
    if (c->closed) {
      unlock(c);
      return 1;
    }

    wait(&c->recv, &c->lock, c);
  }

  if (!c->qsize && c->closed) {
    unlock(c);
    return 1;
  }

  c->vals = get(c->vals, value);
  c->qsize--;
  unlock(c);
  pthread_cond_signal(&c->send);
  return 0;
}

chan* make_chan(size_t size) {
  chan* c = calloc(1, sizeof(chan));
  c->size = size;
  pthread_mutex_init(&c->lock, NULL);
  pthread_cond_init(&c->send, NULL);
  pthread_cond_init(&c->recv, NULL);
  return c;
}

void destroy_chan(chan* c) {
  pthread_mutex_destroy(&c->lock);
  pthread_cond_destroy(&c->send);
  pthread_cond_destroy(&c->recv);
  queue* q = c->vals;
  while (q) {
    free(q);
    q = q->next;
  }
  free(c);
}
