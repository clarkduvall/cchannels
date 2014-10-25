#ifndef GO_H
#define GO_H

#include <pthread.h>
#include <semaphore.h>

struct queue {
  void* value;
  struct queue* next;
};
typedef struct queue queue;

typedef struct {
  queue* vals;
  sem_t in;
  sem_t out;
  pthread_mutex_t lock;
  int closed;
} chan;

void send_to(chan* c, void* value);
void* get_from(chan* c);
chan* make_chan(size_t size);
void destroy_chan(chan* c);
void close_chan(chan* c);
pthread_t create_thread(void *(*start_routine)(void*), void* arg);

#endif // GO_H
