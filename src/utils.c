#include <stdio.h>
#include "utils.h"

/**
 * @brief It performs some kind of job for some time.
 * @param data is some data passed to the function. It use it to perform some actions inside the function.
 */
void do_work(void * data){
  int num_jobs = *(int *)data;
  for (int i = 0; i < num_jobs; i++){
    //Non faccio nulla
  }
}