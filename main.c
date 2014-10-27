#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "chan.h"

typedef struct {
  chan* c;
  chan* end;
} thread_info;

pthread_t create_thread(void *(*start_routine)(void*), void* arg) {
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, start_routine, arg);
  pthread_detach(thread_id);
  return thread_id;
}

static void* grab_from_chan(void *arg) {
  long acc = 0;
  thread_info* tinfo = arg;

  void* v;
  while (!get_from(tinfo->c, &v))
    acc += (intptr_t)v;
  send_to(tinfo->end, (void*)(intptr_t)acc);
  return NULL;
}

static void* add_to_chan(void *arg) {
  int i;
  thread_info* tinfo = arg;

  for (i = 0; i < 10000000; i++)
    send_to(tinfo->c, (void*)(intptr_t)i);
  close_chan(tinfo->c);
  send_to(tinfo->end, 0);
  return NULL;
}

int main() {
  int tnum, num_threads = 5, i;
  long acc = 0;
  thread_info* tinfo;
  chan* c = make_chan(10);
  chan* end = make_chan(1);

  tinfo = calloc(num_threads + 1, sizeof(thread_info));

  tinfo[num_threads].c = c;
  tinfo[num_threads].end = end;
  create_thread(&add_to_chan, &tinfo[num_threads]);

  for (tnum = 0; tnum < num_threads; tnum++) {
    tinfo[tnum].c = c;
    tinfo[tnum].end = end;
    create_thread(&grab_from_chan, &tinfo[tnum]);
  }

  void* v;
  for (i = 0; i < num_threads + 1; i++) {
    get_from(end, &v);
    acc += (intptr_t)v;
  }

  printf("Total Numbers: %ld\n", acc);

  destroy_chan(end);
  destroy_chan(c);
  free(tinfo);
  exit(EXIT_SUCCESS);
}
