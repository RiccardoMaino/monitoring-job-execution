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
#define DEFAULT_PARAMETER 1000
#define MAX_JOBS 100
#define MAX_VALUE 100000000
#define MIN_VALUE 100

//CON IFDEF FACCIO POSSIBILITA DI LEGGERE LA TRACE O LA TRACE_PIPE

void usage();
long update_parameter(long parameter);
long update_parameter_2();
long update_parameter_3(long parameter, long increase);
void do_work(void* param);
void do_work_exchanging(void* param);
void do_work_ordering(void* param);
void print_sched_attr(struct sched_attr* attr);

int main(int argc, char *argv[]){
  long parameter_data;                  //It is the parameter value received by the user input 
  int mode;                             //It specify the job mode of the program
  int pid;                              //It is the PID of the process
  void (*do_work_ptr)(void *);          //It is a pointer to the job to execute based on the mode selected
  char *str, *end_ptr;                  //They are char pointers used by the program
  struct timespec tp;                   //It is a structure needed by the nanosleep to specify the number of sec and nsec to wait
  exec_info execution_info;             //It is a structure that contains execution information
  
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
  //Setting the scheduling policy and priority
  set_scheduler_policy(0, SCHED_OTHER, 0, &execution_info);
  //Enabling the tracing infrastructure
  ENABLE_TRACING;
  //Setting up the filter of the sched_switch event
  set_event_filter(pid, E_SCHED_SWITCH);
  //Setting the identifier value
  execution_info.id = generate_execution_identifier();
  //Enabling the tracing of the sched_switch event
  event_record(E_SCHED_SWITCH, ENABLE);
  for(int i = 0; i<MAX_JOBS; i++){
    //Updates the parameter
    execution_info.parameter = parameter_data;
    execution_info.job_number = i+1;
    //Trace mark that the i-th job started
    trace_mark_job(i+1, START);
    //Execute Job
    do_work_ptr(&parameter_data);
    //Trace mark that the i-th job ended
    trace_mark_job(i+1, STOP);
    //Log the execution informations
    log_execution_info(RESULTS_DIR, execution_info.id, &execution_info, NULL, DEFAULT_INFO);
    //Wait some time before starting the next job
    if(nanosleep(&tp, NULL) != 0){
      fprintf(stderr, "Nanosleep has been interrupted ...\n");
    }
    parameter_data = update_parameter_3(parameter_data, 1000000);
  }
  //Disabling the tracing of the sched_switch event
  event_record(E_SCHED_SWITCH, DISABLE);
  //Log the kernel trace
  log_trace(RESULTS_DIR, execution_info.id);
  //Disabling the tracing infrastructure
  DISABLE_TRACING;

  free(execution_info.id);
  printf("DONE. All has been correctly saved. Terminating.\n");

  return 0;
}

/**
 * @brief It allows to print the sched_attr structure passed as parameter
 * @param attr is a pointer to the sched_attr structure that we want to print out
*/
void print_sched_attr(struct sched_attr* attr){
  printf("The process scheduling attributes are the following:\n");
  printf("{\n\tSize = %u\n\tPolicy = %u\n\tFlags = %llu\n\tNice = %d\n\tPriority = %u\n\n\t**** Fields for SCHED_DEADLINE ****\n\tRuntime = %llu\n\tDeadline = %llu\n\tPeriod = %llu\n}\n", 
          attr->size, attr->sched_policy, attr->sched_flags, attr->sched_nice, attr->sched_priority, attr->sched_runtime, attr->sched_deadline, attr->sched_period);
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
long update_parameter(long parameter){
  return (parameter%2 == 0) ? parameter/2 : 3*parameter + 1;
}

/**
 * @brief It updates the parameter based on the math function provided
*/
long update_parameter_2(){
  long num;
  
  srand(time(NULL));
  num = (rand() % (MAX_VALUE - MIN_VALUE + 1)) + MIN_VALUE;
  return num;
}

/**
 * @brief It updates the parameter based on the math function provided
*/
long update_parameter_3(long parameter, long increase){
  return parameter + increase;
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