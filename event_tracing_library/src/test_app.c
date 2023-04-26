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
#include "event_tracing.h"
#include "list.h"

#define RESULTS_DIR "../results"
#define DEFAULT_PARAMETER 1000
#define MAX_JOBS 50
#define MAX_VALUE 100000000
#define MIN_VALUE 100

void usage();
long update_parameter(long parameter);
long update_parameter_2();
long update_parameter_3(long parameter, long increase);
void do_work(void* param);
void do_work_exchanging(void* param);
void do_work_ordering(void* param);

int main(int argc, char *argv[]){
  long parameter_data;                  ///>It is the parameter value received by the user input 
  int mode;                             ///>It is an integer representing the job mode and it is received by the user input
  int policy;                           ///>It is an integer representing the scheduling policy to use with a job and it is received by the user input
  int priority;                         ///>It is an integer representing the scheduler priority to use with a job and it is received by the user input
  int pid;                              ///>It is the PID of the process
  void (*do_work_ptr)(void *);          ///>It is a pointer to the job to execute based on the mode selected
  char *str, *end_ptr;                  ///>They are char pointers used by the program
  struct timespec tp;                   ///>It is a structure needed by the nanosleep to specify the number of sec and nsec to wait
  exec_info* execution_info;            ///>It is a pointer to a structure that contains execution information
  
  tp.tv_sec = 1;
  tp.tv_nsec = 0;
  pid = getpid();

  if(argc < 2){
    usage();
    exit(EXIT_FAILURE);
  }else{
    parameter_data = strtol(argv[1], &end_ptr, 10);
    if(errno != 0 || end_ptr == argv[1] || *end_ptr != '\0'){
      fprintf(stderr, "main: error converting string argv[1] to integer.\n");
      exit(EXIT_FAILURE);
    }
    mode = strtol(argv[2], &end_ptr, 10);
    if(errno != 0 || end_ptr == argv[2] || *end_ptr != '\0'){
      fprintf(stderr, "main: error converting string argv[2] to integer.\n");
      exit(EXIT_FAILURE);
    }
    policy = strtol(argv[3], &end_ptr, 10);
    if(errno != 0 || end_ptr == argv[3] || *end_ptr != '\0'){
      fprintf(stderr, "main: error converting string argv[3] to integer.\n");
      exit(EXIT_FAILURE);
    }
    if(policy != SCHED_OTHER && policy != SCHED_FIFO && policy != SCHED_RR && policy != SCHED_BATCH && policy != SCHED_IDLE && policy != SCHED_DEADLINE){
      fprintf(stderr, "main: error invalid policy provided.\n");
      usage();
      exit(EXIT_FAILURE);
    }
    priority = strtol(argv[4], &end_ptr, 10);
    if(errno != 0 || end_ptr == argv[4] || *end_ptr != '\0'){
      fprintf(stderr, "main: error converting string argv[4] to integer.\n");
      exit(EXIT_FAILURE);
    }
    if(priority < 0 && priority > 99){
      fprintf(stderr, "main: error invalid priority provided.\n");
      usage();
      exit(EXIT_FAILURE);
    }
    if((priority >= 1 && priority <= 99) && (policy != SCHED_FIFO && policy != SCHED_RR)){
      fprintf(stderr, "main: setting priority to 0 since SCHED_FIFO or SCHED_RR aren't used.\n");
      priority = 0;
    }
    if(priority == 0 && (policy == SCHED_FIFO || policy == SCHED_RR)){
      fprintf(stderr, "main: setting priority to 1 since SCHED_FIFO or SCHED_RR used and zero can be used only with other policies.\n");
      priority = 1;
    }
  }

  execution_info = create_exec_info(0, parameter_data, NULL);

  if(mode == 3){
    do_work_ptr = do_work_ordering;
    execution_info->details = "ListOrdering";
  }else if(mode == 2){
    do_work_ptr = do_work_exchanging;
    execution_info->details = "VariablesExchanging";
  }else{
    do_work_ptr = do_work;
    execution_info->details = "EmptyLoop";
  }


  printf("Executing ...\n");
  //Setting the scheduling policy and priority
  set_scheduler_policy(0, policy, priority, execution_info);
  //Enabling the tracing infrastructure
  ENABLE_TRACING;
  //Setting up the filter of the sched_switch event
  set_event_filter(pid, E_SCHED_SWITCH, SET);
  //Enabling the tracing of the sched_switch event
  event_record(E_SCHED_SWITCH, ENABLE);
  for(int i = 0; i<MAX_JOBS; i++){
    //Updates the parameter
    execution_info->parameter = parameter_data;
    execution_info->job_number = i+1;
    //Trace mark that the i-th job started
    trace_mark_job(i+1, START);
    //Execute Job
    do_work_ptr(&parameter_data);
    //Trace mark that the i-th job ended
    trace_mark_job(i+1, STOP);
    //Log the execution informations
    log_execution_info(RESULTS_DIR, execution_info->id, execution_info, NULL, DEFAULT_INFO);
    //Wait some time before starting the next job
    if(nanosleep(&tp, NULL) != 0){
      fprintf(stderr, "Nanosleep has been interrupted ...\n");
    }
    parameter_data = update_parameter_3(parameter_data, 10000);
  }
  //Disabling the tracing of the sched_switch event
  event_record(E_SCHED_SWITCH, DISABLE);
  //Log the kernel trace
  log_trace(RESULTS_DIR, execution_info->id, USE_TRACE);
  //Disabling the tracing infrastructure
  DISABLE_TRACING;

  
  printf("DONE. All has been correctly saved. Terminating.\n");

  return 0;
}


/**
 * @brief It is a function that tells the user how to use the application that is invoked 
 * when the user omits one or more command line arguments
 */
void usage(){
  printf("\nMust provide all the parameters. Usage: ");
  printf("sudo ./test_app <parameter_data> <mode> <policy> <priority>\n \
                \t<parameter_data>: an integer value representing the first parameter to use with the first job. After it will be updated based on some function.\n \
                \t<mode>: an integer value representing the job mode.\n \
                \t\t1: Empty loop\n \
                \t\t2: Exchanging variables\n \
                \t\t3: Ordering a list\n \
                \t<policy>: an integer value representing the scheduling policy to use with the jobs.\n \
                \t\t0: SCHED_OTHER\n \
                \t\t1: SCHED_FIFO\n \
                \t\t2: SCHED_RR\n \
                \t\t3: SCHED_BATCH\n \
                \t\t5: SCHED_IDLE\n \
                \t\t6: SCHED_DEADLINE\n \
                \t<priority>: an integer value representing the scheduler priority with SCHED_FIFO and SCHED_RR policies.\n \
                \t\t1-99: between 1 (low priority) and 99 (high priority) for SCHED_FIFO or SCHED_RR policies\n \
                \t\t0: for other policies\n \
          \nAborted.\n");
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