#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <strings.h>
#include "utils.h"

/**
 * @brief It allows to set the signal handler for a specific signal. Returns the structure representing the old handler
 * @param signum is the signal number representing the signal
 * @param handle_signal is the pointer to the function that handles the signal specified by signum
 * @param sa_mask is the signal mask to use in the handler function
 * @param sa_flags are some special flags that can be set
*/
struct sigaction set_signal_handler(int signum, void (*handle_signal)(int), sigset_t sa_mask, int sa_flags){
  struct sigaction sa, sa_old;
  bzero(&sa, sizeof(sa));
	sa.sa_handler = handle_signal;
  sa.sa_mask = sa_mask;
  sa.sa_flags = sa_flags;
	sigaction(signum, &sa, &sa_old);
  return sa_old;
}

/**
 * @brief It obtains sem_op resources if possible using the semaphore specified by sem_id with index sem_num
 * @param sem_id is the id number of the semaphore
 * @param sem_num is the index corresponding to the semaphore in the array of semaphore
 * @param sem_op is a negative integer representing the number of resources we want to obtain. For "Zero Waiting" set it to 0
 * @param sem_flag is a integer representing some flags to assign
 * @return if successful it returns 1, otherwise it returns 0 and sets errno
*/
int sem_wait(int sem_id, unsigned short sem_num, short sem_op, short sem_flag){ 
  struct sembuf sops;
  sops.sem_flg = sem_flag;
  sops.sem_num = sem_num;
  sops.sem_op = sem_op;
  return semop(sem_id, &sops, 1);
}

/**
 * @brief Release sem_op resources using semaphore sem_id with index sem_num
 * @param sem_id is the id number of the semaphore
 * @param sem_num is the index corresponding to the semaphore in the array of semaphore
 * @param sem_op is a negative integer representing the number of resources we want to obtain
 * @param sem_flag is a integer representing some flags to assign
 * @return if successful it returns 1, otherwise it returns 0 and sets errno
*/
int sem_signal(int sem_id, unsigned short sem_num, short sem_op, short sem_flag) {
	struct sembuf sops;
	sops.sem_flg = sem_flag;
  sops.sem_num = sem_num;
	sops.sem_op = sem_op;
	return semop(sem_id, &sops, 1);
}


/**
 * @brief It performs some kind of job for some time.
 * @param data is some data passed to the function. It use it to perform some actions inside the function.
 */
void do_work(void * data){
  int num_jobs = *(int *)data;
  // int a, b, temp;
  // srand(time(NULL));
  for (int i = 0; i < num_jobs; i++){
    // a = rand() % 1000 + 1;
    // b = rand() % 1000 + 1;
    // temp = a;
    // a = b;
    // b = temp;
    // printf("A and B exachanged: a = %d and b = %d\n", a, b);
  }
}