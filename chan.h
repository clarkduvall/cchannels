#ifndef GO_H
#define GO_H

#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>

struct queue {
  void* value;
  struct queue* next;
};
typedef struct queue queue;

typedef struct {
  queue* vals;
  size_t qsize;
  size_t size;
  pthread_mutex_t lock;
  pthread_cond_t send;
  pthread_cond_t recv;
  uint8_t closed;
} chan;

int send_to(chan* c, void* value);
int get_from(chan* c, void** value);
chan* make_chan(size_t size);
void destroy_chan(chan* c);
void close_chan(chan* c);
pthread_t create_thread(void *(*start_routine)(void*), void* arg);

#endif // GO_H
