#define _POSIX_C_SOURCE 199309L
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <argp.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <sys/wait.h>
#include "../include/event_tracing.h"
#include "list.h"

#define RESULTS_DIR "../results"
#define DEFAULT_PARAMETER 10000
#define DEFAULT_MODE 3
#define DEFAULT_MAX_JOBS 25
#define DEFAULT_POLICY SCHED_OTHER
#define DEFAULT_PRIORITY 0
#define MAX_VALUE 1000000
#define MIN_VALUE 100

/**
 * @brief A structure that contains the command line arguments parsed from the user input, that
 * will be used to set up some options of this execution.
*/
struct arguments {
  long param; //It is a long integer value representing the parameter value to use with the first job
  int mode; //It is an integer value representing the kind of job to perform
  int policy; //It is an integer value representing the scheduling policy to use with the jobs executed
  int priority; //It is an integer value representing the scheduler priority to use with the jobs executed
  int jobs; //It is an integer value representing the number of jobs to perform
};

//Program version.
const char *argp_program_version = "Event Tracing Library 1.0";

//Program bug report address.
const char *argp_program_bug_address = "<riccardo.maino@edu.unito.it>";

// Program documentation.
static char doc[] = "This program demonstrates the usage of the 'event_tracing' library. Possible [OPTION...] could be the following ones:\
\vThe arguments of the options MODE, POLICY and PRIO must comply with and respect the following values.\n\n\
[MODE] possible integer values:\n\
\t1: Empty loop job.\n\
\t2: Variables exchanging job.\n\
\t3: List ordering job.\n\n\
[POLICY] possible string values:\n\
\tSCHED_OTHER: Non-real-time scheduling policy.\n\
\tSCHED_FIFO: Real-time scheduling policy.\n\
\tSCHED_RR: Real-time scheduling policy.\n\
\tSCHED_BATCH: Non-real-time scheduling policy.\n\
\tSCHED_IDLE: Non-real-time scheduling policy.\n\
\tSCHED_DEADLINE: Deadline scheduling policy.\n\n\
[PRIO] possible integer values:\n\
\t1 (low priority) to 99 (high priority): For SCHED_FIFO or SCHED_RR.\n\
\t0: For all the others policies.\n";

// The command line options accepted to obtain the arguments contained in the 'struc arguments' structure
static struct argp_option options[] = {
  {"param", 'p', "PARAM", 0, "Set the parameter value to use with the first job. PARAM must be a positive long integer."},
  {"mode", 'm', "MODE", 0, "Set the kind of the job to perform. MODE must be an integer value. Check the below section to see other details about the MODE argument."},
  {"policy", 's', "POLICY", 0, "Set the scheduling policy of the jobs executed. POLICY must be a string. Check the below section to see other details about the POLICY argument."},
  {"priority", 'r', "PRIO", 0, "Set the scheduler priority of the jobs executed. PRIO must be an intger value. Check the below section to see other details about the PRIO argument."},
  {"jobs", 'j', "JOBS", 0, "Set the number of jobs executed. JOBS must be a positive integer."},
  {0}
};

//Function used by the command line arguments parser to correctly obtain arguments.
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  char *end_ptr;
  /* Get the input argument from argp_parse, which we know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;
  switch(key){
    case 'p':
      arguments->param = strtol(arg, &end_ptr, 10);
      if(errno != 0 || end_ptr == arg || *end_ptr != '\0'){
        argp_error(state, "Error converting to integer the argument for '--param' option");
      }
      if(arguments->param < 0){
        argp_error(state, "Invalid argument for '--param' option");
      }
      break;
    case 'm':
      arguments->mode = strtol(arg, &end_ptr, 10);
      if(errno != 0 || end_ptr == optarg || *end_ptr != '\0'){
        argp_error(state, "Error converting to integer the argument for '--mode' option");
      }
      if(arguments->mode != 1 && arguments->mode != 2 && arguments->mode != 3){
        argp_error(state, "Invalid argument for '--mode' option");
      }
      break;
    case 's':
      if(strcmp("SCHED_OTHER", arg) == 0)
        arguments->policy= SCHED_OTHER;
      else if(strcmp("SCHED_FIFO", arg) == 0)
        arguments->policy = SCHED_FIFO;
      else if(strcmp("SCHED_RR", arg) == 0)
        arguments->policy = SCHED_RR;
      else if(strcmp("SCHED_BATCH", arg) == 0)
        arguments->policy = SCHED_BATCH;
      else if(strcmp("SCHED_IDLE", arg) == 0)
        arguments->policy = SCHED_IDLE;
      else if(strcmp("SCHED_DEADLINE", arg) == 0)
        arguments->policy = SCHED_DEADLINE;
      else
        argp_error(state, "Invalid argument for '--policy' option");
      break;
    case 'r':
      arguments->priority = strtol(arg, &end_ptr, 10);
      if(errno != 0 || end_ptr == optarg || *end_ptr != '\0'){
        argp_error(state, "Error converting to integer the argument for '--priority' option");
      }
      if(arguments->priority < 0 || arguments->priority > 99){
        argp_error(state, "Invalid argument for '--priority' option");
      }
      break;
    case 'j':
      if(arguments->jobs <= 0){
        argp_error(state, "Invalid argument for '--njobs' option");
      }
      break;
    case ARGP_KEY_END:
      if(state->arg_num != 0){
        argp_error(state, "Found one or more no-option arguments");
      }

      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

//The ARGP parser
static struct argp argp = { options, parse_opt, 0, doc };

long update_parameter(long parameter);
long update_parameter_2();
long update_parameter_3(long parameter, long increase);
void do_work(void* param);
void do_work_exchanging(void* param);
void do_work_ordering(void* param);

int main(int argc, char *argv[]){
  struct arguments arguments;
  int pid;                      //It is the PID of the process
  void (*do_work_ptr)(void *);  //It is a pointer to the job to execute based on the mode selected
  char *str, *end_ptr;          //They are char pointers used by the program
  struct timespec tp;           //It is a structure needed by the nanosleep to specify the number of sec and nsec to wait
  exec_info* execution_info;    //It is a pointer to a structure that contains execution information
  
  arguments.param = DEFAULT_PARAMETER;
  arguments.mode = DEFAULT_MODE;
  arguments.policy = DEFAULT_POLICY;
  arguments.priority = DEFAULT_PRIORITY;
  arguments.jobs = DEFAULT_MAX_JOBS;

  // Parse command line arguments
  if(argp_parse(&argp, argc, argv, 0, 0, &arguments) != 0){
    fprintf(stderr, "Parsing command line arguments error ...\n");
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }

  tp.tv_sec = 1;
  tp.tv_nsec = 0;
  pid = getpid();

  // Init the exec_info struct
  execution_info = create_exec_info(0, arguments.param, NULL);

  if(arguments.mode == 3){
    do_work_ptr = do_work_ordering;
    execution_info->details = "ListOrdering";
  }else if(arguments.mode == 2){
    do_work_ptr = do_work_exchanging;
    execution_info->details = "VariablesExchanging";
  }else{
    do_work_ptr = do_work;
    execution_info->details = "EmptyLoop";
  }


  printf("Executing ...\n");
  // Setting the scheduling policy and priority
  set_scheduler_policy(0, arguments.policy, arguments.priority, execution_info);
  // Enabling the tracing infrastructure 
  ENABLE_TRACING;
  // Setting up the filter of the sched_switch event
  set_event_filter(pid, E_SCHED_SWITCH, SET);
  // Enabling the tracing of the sched_switch event
  event_record(E_SCHED_SWITCH, ENABLE);
  for(int i = 0; i<arguments.jobs; i++){
    // Updates the parameter
    execution_info->parameter = arguments.param;
    execution_info->job_number = i+1;
    // Trace mark that the i-th job started
    trace_mark_job(i+1, START);
    // Execute Job
    do_work_ptr(&arguments.param);
    // Trace mark that the i-th job ended
    trace_mark_job(i+1, STOP);
    // Log the execution informations
    log_execution_info(RESULTS_DIR, execution_info->id, execution_info, NULL, DEFAULT_INFO);
    // Wait some time before starting the next job
    if(nanosleep(&tp, NULL) != 0){
      fprintf(stderr, "Nanosleep has been interrupted ...\n");
    }
    arguments.param = update_parameter_3(arguments.param, 10000);
  }
  // Disabling the tracing of the sched_switch event
  event_record(E_SCHED_SWITCH, DISABLE);
  // Log the kernel trace
  log_trace(RESULTS_DIR, execution_info->id, USE_TRACE);
  // Disabling the tracing infrastructure
  DISABLE_TRACING;
  
  printf("DONE. All has been correctly saved. Terminating.\n");
  return 0;
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