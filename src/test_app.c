#define _POSIX_C_SOURCE 199309L
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <sys/wait.h>
#include "utils.h"

#define PROGRAM "test_app"
#define PARAMETERS_PATH "/home/riccardo/Desktop/Stage/extract-data/parameters"
#define RESULTS_PATH "/home/riccardo/Desktop/Stage/extract-data/results"
#define MAX_JOBS 10
#define MAX_INPUT_LINE_LENGTH 128
#define MAX_COMMAND_LENGTH 150
#define MAX_FILENAME_LENGTH 80
#define MAX_IDENTIFIER_LENGTH 20
#define DEFAULT_PARAMETER 5

/* Prototypes */
void default_execution();
void custom_execution(int parameter_data);
void save_parameter(char * output_parameter_filename, int parameter, char * identifier);
void usage();

int main(int argc, char *argv[]){
  int parameter_data;                                                     //Parameter data received by the user ì
  int pid, trace_cmd_record_pid, trace_cmd_record_status;                 //Integers used by the program  
  char *str, *end_ptr;                                                    //Char pointers used by the program
  time_t current_time;                                                    //Struct used to get the current time. This will be used as an identifier.
  struct tm *tm;                                                          //Struct to store the actual time in a specific format.
  char line[MAX_INPUT_LINE_LENGTH];                                       //String to store the user input 
  char identifier[MAX_IDENTIFIER_LENGTH];                                 //String that specify the identifier of this actual execution
  char output_trace_filename[MAX_FILENAME_LENGTH];                        //Filename where will be stored the kernel infos of this actual execution
  char output_parameter_filename[MAX_FILENAME_LENGTH];                    //Filaname where will be stored parameters of this actual execution
  char command[MAX_COMMAND_LENGTH];                                       //String used to store a terminal command
  char *args[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};  //Additional arguments for the trace-cmd process
  int max_size_int = ceil(log10(INT_MAX)) + 1;                            //Maximum number of digits that has the integer maximum number 
  char * str_pid = calloc(max_size_int, sizeof(*str_pid));                //String representation of the PID of this process

  current_time = time(NULL);
  tm = localtime(&current_time);
  strftime(identifier, sizeof(identifier), "%Y%m%d%H%M%S", tm);
  sprintf(output_trace_filename, "%s/%s.txt", RESULTS_PATH, identifier);
  sprintf(output_parameter_filename, "%s/%s.txt", PARAMETERS_PATH, identifier);
  pid = getpid();
  sprintf(str_pid, "%d", pid);

  args[0] = "trace-cmd";
  args[1] = "record";
  args[2] = "-e";
  args[3] = "sched:sched_switch";
  args[4] = "-P";
  args[5] = str_pid;
  args[6] = "-o";
  args[7] = "trace.dat";

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


  switch(trace_cmd_record_pid = fork()){
    case -1:
      fprintf(stderr, "Error creating trace-cmd process. Aborting ...\n");
      exit(EXIT_FAILURE);
      break;
    case 0:
      execvp(args[0], args);
      perror("Error execvp failed. Aborting ..."); // execvp only returns if there's an error
      exit(EXIT_FAILURE);
      break;
    default:
      printf("Press enter to continue and start tracing ...\n");
      fgets(line, MAX_INPUT_LINE_LENGTH ,stdin);
      if(strcmp(line, "\n") == 0){
        custom_execution(parameter_data);
        kill(trace_cmd_record_pid, SIGINT);
      }else{
        fprintf(stderr, "Invalid operation. Aborting ...\n");
        kill(trace_cmd_record_pid, SIGINT);
        exit(EXIT_FAILURE);
      }
      break;
  }

  waitpid(trace_cmd_record_pid, &trace_cmd_record_status, 0);
  if (WIFEXITED(trace_cmd_record_status)) {
    printf("\nStatus: trace-cmd exited with status %d\n", WEXITSTATUS(trace_cmd_record_status));
  } else if (WIFSIGNALED(trace_cmd_record_status)) {
    printf("\nStatus: trace-cmd terminated by signal %d\n", WTERMSIG(trace_cmd_record_status));
  }

  sprintf(command, "trace-cmd report > %s", output_trace_filename);
  system(command);
  save_parameter(output_parameter_filename, parameter_data, identifier);
  printf("Saved correctly on file: %s\n", output_parameter_filename);
  

  
  printf("Program terminated.\n");
  return 0;
}

/**
 * @brief It's the task executed with default value of the parameter, these will be
 * passed to function that performs some work.
 */
void default_execution(){
  int parameter_data = DEFAULT_PARAMETER;
  struct timespec tp;
  tp.tv_sec = 1;
  tp.tv_nsec = 0;

  for(int i = 0; i < MAX_JOBS; i++){
    do_work(&parameter_data);
    if(nanosleep(&tp, NULL) != 0){
      fprintf(stderr, "Nanosleep interrotta...");
    }
  }
}

/**
 * @brief It's the task executed with value of parameter data obtained by the user input
 * These will be passed to function that performs some work.
 */
void custom_execution(int parameter_data){
  struct timespec tp;
  tp.tv_sec = 1;
  tp.tv_nsec = 0;

  for (int i = 0; i < MAX_JOBS; i++){
    do_work(&parameter_data);
    if(nanosleep(&tp, NULL) != 0){
      fprintf(stderr, "Nanosleep interrotta...");
    }  
  }
}

/**
 * @brief It saves the parameter used for the inner loop on a file. This file will be
 * manipulted by the Python program, to perform some data analysis on time execution based on
 * the user input
*/
void save_parameter(char * output_parameter_filename, int parameter, char * identifier){
  FILE *fptr;
  fptr = fopen(output_parameter_filename, "w");
  if(fptr == NULL){
    fprintf(stderr, "Error creating parameter file. Aborting ...\n");
    exit(EXIT_FAILURE);
  }
  fprintf(fptr, "%d, %s\n", parameter, identifier);
  fclose(fptr);
}

/**
 * @brief Function that tells the user how to use the application that is invoked 
 * when it's used the dafault execution to specify the user how to use it.
 */
void usage(){
  printf("\nUSAGE with parameters:");
  printf("./test_app <max_number_jobs> <parameter_data>. Starting with default parameter ...\n\n");
}