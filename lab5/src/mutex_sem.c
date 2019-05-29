/********************************************************
 * An example source module to accompany...
 *
 * "Using POSIX Threads: Programming with Pthreads"
 *     by Brad nichols, Dick Buttlar, Jackie Farrell
 *     O'Reilly & Associates, Inc.
 *  Modified by A.Kostin
 ********************************************************
 * mutex_s.c
 *
 * Simple multi-threaded example with a mutex lock.
 */
#include <stdbool.h>
#include <sys/time.h>
#include <getopt.h>

#include <stdint.h>

#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>

void do_one_thing(int *);
void do_another_thing(int *);
void do_wrap_up(int);
int common = 0; /* A shared variable for two threads */
int r1 = 0, r2 = 0, r3 = 0;
int state;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

    sem_t *sem;

int main(int c, int* a) {
    

  if( (sem = sem_open("/my_sem", O_CREAT, 0777, 1)) == SEM_FAILED){
        printf("error semophore!!!\n");
        return 1;
  }
  sem_post(sem);
  int value_sem=0;
  if(sem_getvalue(sem, &value_sem) != 0){
        printf("error semophore sem_getvalue!!!\n");
        return 1;
  };
  printf("sem_getvalue = %d\n", value_sem);
    
  pthread_t thread1, thread2;
	state = c;
  if (pthread_create(&thread1, NULL, (void *)do_one_thing,
			  (void *)&common) != 0) {
    perror("pthread_create");
    exit(1);
  }

  if (pthread_create(&thread2, NULL, (void *)do_another_thing,
                     (void *)&common) != 0) {
    perror("pthread_create");
    exit(1);
  }

  if (pthread_join(thread1, NULL) != 0) {
    perror("pthread_join");
    exit(1);
  }

  if (pthread_join(thread2, NULL) != 0) {
    perror("pthread_join");
    exit(1);
  }

  do_wrap_up(common);
  
sem_close(sem);

  return 0;
}

void do_one_thing(int *pnum_times) {
  int i, j, x;
  unsigned long k;
  int work;
  for (i = 0; i < 50; i++) {
	if(state > 0)
	 sem_wait(sem);
  //  	pthread_mutex_lock(&mut);
    printf("doing one thing\n");
    work = *pnum_times;
    printf("counter = %d\n", work);
    work++; /* increment, but not write */
    for (k = 0; k < 500000; k++)
      ;                 /* long cycle */
    *pnum_times = work; /* write back */
	if(state > 0)
    	sem_post(sem);
	//	 pthread_mutex_unlock(&mut);
  }
}

void do_another_thing(int *pnum_times) {
  int i, j, x;
  unsigned long k;
  int work;
  for (i = 0; i < 50; i++) {
	if(state > 0)
    	 sem_wait(sem);
   		// pthread_mutex_lock(&mut);
    printf("doing another thing\n");
    work = *pnum_times;
    printf("counter = %d\n", work);
    work++; /* increment, but not write */
    for (k = 0; k < 500000; k++)
      ;                 /* long cycle */
    *pnum_times = work; /* write back */
	if(state > 0)
	   sem_post(sem);
   	//	pthread_mutex_unlock(&mut);
  }
}

void do_wrap_up(int counter) {
  int total;
  printf("All done, counter = %d\n", counter);
}
