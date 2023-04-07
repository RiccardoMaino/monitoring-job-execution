#define _POSIX_C_SOURCE 199309L
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <sys/wait.h>
#include "tracing.h"
#include "list.h"

#define RESULTS_DIR "/home/riccardo/Desktop/Stage/demo/results"
// #define PARAMETERS_PATH "/home/riccardo/Desktop/Stage/extract-data/parameters"
// #define RESULTS_PATH "/home/riccardo/Desktop/Stage/extract-data/results"
#define DEFAULT_PARAMETER 1000
#define MAX_JOBS 10

//CON IFDEF FACCIO POSSIBILITA DI LEGGERE LA TRACE O LA TRACE_PIPE

void usage();
long update_parameter(long parameter_data, int i);
void do_work(void* param);
void do_work_exchanging(void* param);
void do_work_ordering(void* param);

int main(int argc, char *argv[]){
  long parameter_data;                                                    //Parameter data received by the user ì
  int mode;                                                               //Launch mode of the program
  int pid;                                                                //Integers used by the program
  void (*do_work_ptr)(void *);                                            //A pointer to the function to execute based on the mode selected
  char *str, *end_ptr;                                                    //Char pointers used by the program
  struct timespec tp;                                                     //It's a structure that define the number of sec and nsec to wait using the nanosleep
  exec_info execution_info;
    
  tp.tv_sec = 1;
  tp.tv_nsec = 0;
  pid = getpid();
  printf("Test App: the process PID is %d.\n", pid);

  if(argc < 2){
    usage();
    mode = 1;
    parameter_data = DEFAULT_PARAMETER;
  }else{
    mode = strtol(argv[1], &end_ptr, 10);
    if(errno != 0 || end_ptr == argv[1] || *end_ptr != '\0'){
      fprintf(stderr, "main: error converting string argv[1] to integer.\n");
      exit(EXIT_FAILURE);
    }
    parameter_data = strtol(argv[2], &end_ptr, 10);
    if(errno != 0 || end_ptr == argv[2] || *end_ptr != '\0'){
      fprintf(stderr, "main: error converting string argv[2] to integer.\n");
      exit(EXIT_FAILURE);
    }
  }

  if(mode == 3){
    do_work_ptr = do_work_ordering;
    execution_info.mode = "ListOrdering";
  }else if(mode == 2){
    do_work_ptr = do_work_exchanging;
    execution_info.mode = "VariableExchanging";
  }else{
    do_work_ptr = do_work;
    execution_info.mode = "EmptyLoop";
  }
  
  printf("Executing ...\n");
  //Enabling the tracing infrastructure
  ENABLE_TRACING;
  //Setting up the filter of the sched_switch event
  set_event_filter(pid, E_SCHED_SWITCH);
  //Setting the identifier value
  execution_info.id = generate_execution_identifier();

  printf("DEBUG: %s\n", execution_info.id);
  //Enabling the tracing of the sched_switch event
  event_record(E_SCHED_SWITCH, ENABLE);
  for(int i = 0; i<MAX_JOBS; i++){
    //Updates the parameter
    execution_info.parameter = update_parameter(parameter_data, i);
    execution_info.job_number = i;
    //Trace mark that the i-th job started
    trace_mark_job(i, START);
    //Execute Job
    do_work_ptr(&parameter_data);
    //Trace mark that the i-th job ended
    trace_mark_job(i, STOP);
    //Log the execution informations
    log_execution_info(RESULTS_DIR, execution_info.id, &execution_info, NULL, DEFAULT_INFO);
    //Wait some time before starting the next job
    if(nanosleep(&tp, NULL) != 0){
      fprintf(stderr, "Nanosleep has been interrupted ...\n");
    }
  }
  //Disabling the tracing of the sched_switch event
  event_record(E_SCHED_SWITCH, DISABLE);
  //Log the kernel trace
  log_trace(RESULTS_DIR, execution_info.id);
  //Disabling the tracing infrastructure
  DISABLE_TRACING;

  free(execution_info.id);
  printf("DONE.\n");
  printf("All has been correctly saved. Terminating.\n");
  return 0;
}

/**
 * @brief It is a function that tells the user how to use the application that is invoked 
 * when the user omits one or more command line arguments
 */
void usage(){
  printf("\nUSAGE with parameters: ");
  printf("./test_app <mode> <parameter_data>\n\t<mode> 1: Empty loop\n\t<mode> 2: Exchanging variables\n\t<mode> 3: Ordering a list\n\nStarting with default parameter ...\n\n");
}

/**
 * @brief It updates the parameter based on the math function provided
 * @param parameter is the original value
 * @param i is an integer value used to choose the next value calculated by the math function
*/
long update_parameter(long parameter, int i){
  return (i%2 == 0) ? parameter/2 : 3*parameter + 1;
}

/**
 * @brief It performs a basic empty loop.
 * @param param is a void pointer that refers to an integer value. It's used as the upper bound of the loop index.
 */
void do_work(void* param){
  int num = *(int *)param;
  for (int i = 0; i < num; i++){
  }
}

/**
 * @brief It performs a loop where some random variables are swapped each other.
 * @param param is a void pointer that refers to an integer value. It's used as the upper bound of the loop index.
 */
void do_work_exchanging(void * param){
  int num = *(int *)param;
  int a, b, temp;
  srand(time(NULL));
  for (int i = 0; i < num; i++){
    a = rand() % 1000 + 1;
    b = rand() % 1000 + 1;
    temp = a;
    a = b;
    b = temp;
  }
}


/**
 * @brief It is the comparing function for integers
 * @param a is the pointer to the first integer value to be compared
 * @param b is the pointer to the second integer values to be compared
 * @return an integer greater than 0 if and only if a > b or an integer lower than 0 if and only if a < b,
 * otherwise it return 0 if and only if a == b
*/
int compare_integer(void* a, void* b){
  return *(int *)a - *(int *)b;
}

/**
 * @brief It performs a loop where it creates, sorts and deletes a list.
 * @param param is a void pointer that refers to an integer value. It's used as the upper bound of the loop index.
 */
void do_work_ordering(void* param){
  int n1 = 10, n2 = 30, n3 = 20, n4 = 50, n5 = 40;
  int num = *(int *)param;
  for(int i = 0; i<num; i++){
    List * list = list_create();
    list_add(list, &n1);
    list_add(list, &n2);
    list_add(list, &n3);
    list_add(list, &n4);
    list_add(list, &n5);
    list->head = mergesort(list->head, compare_integer);
    list_destroy(list);
  }
}