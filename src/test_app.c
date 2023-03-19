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
#include "utils.h"


#define PROGRAM "test_app"
#define PARAMETERS_PATH "/home/riccardo/Desktop/Stage/extract-data/parameters"
#define RESULTS_PATH "/home/riccardo/Desktop/Stage/extract-data/results"
#define DEFAULT_PARAMETER 5

/**
 * @brief Function that tells the user how to use the application that is invoked 
 * when it's used the dafault execution to specify the user how to use it.
 */
void usage(){
  printf("\nUSAGE with parameters:");
  printf("./test_app <parameter_data>. Starting with default parameter ...\n\n");
}

int main(int argc, char *argv[]){
  long parameter_data;                                                    //Parameter data received by the user ì
  int pid;                                                                //Integers used by the program  
  char *str, *end_ptr;                                                    //Char pointers used by the program
  time_t current_time;                                                    //Struct used to get the current time. This will be used as an identifier.
  struct tm *tm;                                                          //Struct to store the actual time in a specific format.
  char identifier[MAX_IDENTIFIER_LENGTH];                                 //String that specify the identifier of this actual execution
  char str_command[MAX_COMMAND_LENGTH];                                   //String that specify the text to write in some sys/kernel/tracing file
  char output_trace_filename[MAX_FILENAME_LENGTH];                        //Filename where will be stored the kernel infos of this actual execution
  char output_parameter_filename[MAX_FILENAME_LENGTH];                    //Filaname where will be stored parameters of this actual execution
  struct timespec tp;                                                     //It's a structure that define the number of sec and nsec to wait using the nanosleep
  
  tp.tv_sec = 1;
  tp.tv_nsec = 0;
  current_time = time(NULL);
  tm = localtime(&current_time);
  strftime(identifier, sizeof(identifier), "%Y%m%d%H%M%S", tm);
  sprintf(output_trace_filename, "%s/%s.txt", RESULTS_PATH, identifier);
  sprintf(output_parameter_filename, "%s/%s.txt", PARAMETERS_PATH, identifier);
  pid = getpid();
  printf("Test App: the process PID is %d\n", pid);

  if(argc < 2){
    usage();
    parameter_data = DEFAULT_PARAMETER;
  }else{
    parameter_data = strtol(argv[1], &end_ptr, 10);
    if(errno != 0 || end_ptr == argv[1] || *end_ptr != '\0'){
      fprintf(stderr, "Error converting string argv[1] to integer.\n");
      exit(EXIT_FAILURE);
    }
  }

  //Setting up the filter of the sched_switch event
  sprintf(str_command, "prev_pid==%d || next_pid==%d", pid, pid);
  sys_write(FILTER_PATH, str_command);

  //Enabling tracing of the sched_switch event
  sprintf(str_command, "1");
  sys_write(ENABLE_PATH, str_command);

  for(int i = 0; i<MAX_JOBS; i++){
    //Trace mark Job started
    sprintf(str_command, "start_job=%d", i+1);
    sys_write(TRACE_MARKER_PATH, str_command);
    //Execute Job
    do_work(&parameter_data);
    //Trace mark Job ended
    sprintf(str_command, "end_job=%d", i+1);
    sys_write(TRACE_MARKER_PATH, str_command);
    if(nanosleep(&tp, NULL) != 0){
      fprintf(stderr, "Nanosleep interrotta...");
    }
  }

  //Disabling tracing of the sched_switch event
  sprintf(str_command, "0");
  sys_write(ENABLE_PATH, str_command);

  save_trace(TRACE_PIPE_PATH, output_trace_filename);
  printf("Tracia salvata \n");
  save_parameter(output_parameter_filename, parameter_data, identifier);
  printf("All has been correctly saved. Terminating.\n");
  return 0;
}