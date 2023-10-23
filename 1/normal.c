#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS 4
#define NUM_CLIENTS 2
#define NUM_REPEAT 100

int SOMA = 0;

void *client(void *thread_id) {
  int id = *(int *)thread_id;
  for (int i = 0; i < NUM_REPEAT; i++) {
    int local = SOMA;
    usleep(rand() % 2);
    SOMA = local + 1;
  }
  pthread_exit(NULL);
}

int main() {
  pthread_t threads[NUM_THREADS];
  int thread_ids[NUM_THREADS];

  for (int i = 0; i < NUM_THREADS; i++) {
    thread_ids[i] = i;
    pthread_create(&threads[i], NULL, client, &thread_ids[i]);
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("SOMA final: %d\n", SOMA);

  return 0;
}
