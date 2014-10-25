#include "go.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
  int thread_num;
  chan* c;
  chan* end;
} thread_info;

static void* grab_from_chan(void *arg) {
  int acc = 0;
  thread_info* tinfo = arg;
  printf("starting thread %d\n", tinfo->thread_num);

  void* v = NULL;
  while ((v = get_from(tinfo->c))) {
    acc++;
    printf("Got %d (%d)\n", (int)(intptr_t)v, tinfo->thread_num);
  }
  send_to(tinfo->end, (void*)(intptr_t)acc);
  return NULL;
}

static void* add_to_chan(void *arg) {
  int i;
  thread_info* tinfo = arg;
  printf("starting adder %d\n", tinfo->thread_num);

  for (i = 1; i < 10000; i++) {
    send_to(tinfo->c, (void*)(intptr_t)i);
  }
  close_chan(tinfo->c);
  send_to(tinfo->end, 0);
  return NULL;
}

int main() {
  int tnum, num_threads = 10, i, acc = 0;
  thread_info* tinfo;
  chan* c = make_chan(10);
  chan* end = make_chan(1);

  tinfo = calloc(num_threads + 1, sizeof(thread_info));

  tinfo[num_threads].c = c;
  tinfo[num_threads].end = end;
  tinfo[num_threads].thread_num = 1000;
  create_thread(&add_to_chan, &tinfo[num_threads]);

  for (tnum = 0; tnum < num_threads; tnum++) {
    tinfo[tnum].c = c;
    tinfo[tnum].end = end;
    tinfo[tnum].thread_num = tnum + 1;
    create_thread(&grab_from_chan, &tinfo[tnum]);
  }

  for (i = 0; i < num_threads + 1; i++) {
    acc += (intptr_t)get_from(end);
  }

  printf("Total Numbers: %d\n", acc);

  destroy_chan(end);
  destroy_chan(c);
  free(tinfo);
  exit(EXIT_SUCCESS);
}
