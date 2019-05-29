#include <unistd.h> 
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void one_p(int *);
void sec_p(int *);
void do_wrap_up(int);
int res[2];
pthread_mutex_t ma = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mb = PTHREAD_MUTEX_INITIALIZER;

int main(int c, int* a) {
  pthread_t th1, th2;
  if (pthread_create(&th1, NULL, (void *)one_p, (void *)&res) != 0) {
    perror("pthread_create");
    exit(1);
  }

  if (pthread_create(&th2, NULL, (void *)sec_p, (void *)&res) != 0) {
    perror("pthread_create");
    exit(1);
  }
  if (pthread_join(th1, NULL) != 0) {
    perror("pthread_join");
    exit(1);
  }
  if (pthread_join(th2, NULL) != 0) {
    perror("pthread_join");
    exit(1);
  }
  return 0;
}

void one_p(int *pnum_times) {
	pthread_mutex_lock(&ma);
	sleep(1);
	pthread_mutex_lock(&mb);
	pthread_mutex_unlock(&ma);
	pthread_mutex_unlock(&mb);
}

void sec_p(int *pnum_times) {
	pthread_mutex_lock(&mb);
	sleep(1);
	pthread_mutex_lock(&ma);
	pthread_mutex_unlock(&mb);
	pthread_mutex_unlock(&ma);
}

void do_wrap_up(int counter) {
  int total;
  printf("All done, counter = %d\n", counter);
}