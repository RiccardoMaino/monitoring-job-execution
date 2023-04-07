#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sched.h>


#include "tracing.h"

#define USE_TRACE_PIPE

/**
 * @brief It allows to generate an identifier for an execution of a job. The identifier
 * is based on the timestamp
 * @return a string identifier. It must be freed after use.
*/
char* generate_execution_identifier(){
  time_t current_time;
  struct tm *tm;
  char* identifier;

  identifier = calloc(MAX_IDENTIFIER_SIZE, sizeof(*identifier));
  current_time = time(NULL);
  tm = localtime(&current_time);
  strftime(identifier, MAX_IDENTIFIER_SIZE, "%Y%m%d%H%M%S", tm);
  return identifier;
}

/**
 * @brief It allows to write within the specified tracing infrastructure file
 * @param file_path is the path to the tracing infrastructure file 
 * @param str is the string that will be written to the file
*/
void tracing_write(const char* file_path, const char* str){
  int fd;
  size_t bytes_written;

  fd = open(file_path, O_WRONLY);
  if(fd == -1){
    fprintf(stderr, "tracing_write: error opening \"%s\" file. Aborting ...\n", file_path);
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }

  bytes_written = write(fd, str, strlen(str));
  if(bytes_written == -1){
    fprintf(stderr, "tracing_write: error writing to \"%s\" file. Aborting ...\n", file_path);
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }

  close(fd);
}

/**
 * @brief It permits to write on the kernel the start or the end of a generic 
 * job which is identified by the job number parameter
 * @param job_number is a integer value that identify the job
 * @param flag is a short value that can be START or STOP based on what we want to
 * mark on the kernel trace. START is used to mark the start of the job. STOP is used to mark
 * the the end of the job.
*/
void trace_mark_job(int job_number, short flag){
  int str_max_size = ceil(log10(INT_MAX)) + 11;
  char str[str_max_size];

  if(flag == START){
    sprintf(str, "start_job=%d", job_number);
    tracing_write(TRACE_MARKER_PATH, str);
  }else if(flag == STOP){
    sprintf(str, "end_job=%d", job_number);
    tracing_write(TRACE_MARKER_PATH, str);
  }else{
    fprintf(stderr, "trace_mark: invalid flag\n");
  }
}

/**
 * @brief It allows you to set a custom filter for any events in any subsystem.
 * @param subsystem is a string that specify subsystem's name of the event.
 * @param event is a string that specify the event's name.
 * @param filter_str is a string indicating the filter value to use for the specified event 
*/
void set_event_filter_custom(const char* subsystem, const char* event, const char* filter_str){
  char* filter_path;
  int filter_path_len;

  filter_path_len = strlen(EVENTS_PATH) + 1 + strlen(subsystem) + 1 + strlen(event) + strlen("/filter") + 1;
  filter_path = (char*)calloc(sizeof(*filter_path), filter_path_len);
  if(filter_path == NULL){
    fprintf(stderr, "set_event_filter_custom: error allocating memory. Aborting ...\n");
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }

  if(sprintf(filter_path, "/sys/kernel/tracing/events/%s/%s/filter", subsystem, event) < 0){
    fprintf(stderr, "set_event_filter_custom: error creating filter. Aborting ...\n");
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }
  tracing_write(filter_path, filter_str);
  free(filter_path);
}

/**
 * @brief It allows you to set default filters of an event
 * @param pid is the pid of the process used in some default filters.
 * @param event_flag is short value indicating the event and it's used to set the default filter defined 
 * in the library for that that event. Possible values can be: E_SCHED_SWITCH, E_SCHED_WAKE_UP and 
 * E_SCHED_MIGRATE_TASK.
*/
void set_event_filter(pid_t pid, short event_flag){
  int str_max_size = ceil(log10(INT_MAX))*2 + 25;
  char filter_str[str_max_size];

  switch(event_flag){
    case E_SCHED_SWITCH:
      sprintf(filter_str, "prev_pid==%d || next_pid==%d", pid, pid);
      tracing_write(SCHED_SWITCH_FILTER_PATH, filter_str);
      break;
    case E_SCHED_WAKE_UP:
      sprintf(filter_str, "prev_pid==%d || next_pid==%d", pid, pid);
      tracing_write(SCHED_WAKE_UP_FILTER_PATH, filter_str);
      break;
    case E_SCHED_MIGRATE_TASK:
      sprintf(filter_str, "prev_pid==%d || next_pid==%d", pid, pid);
      tracing_write(SCHED_WAKE_UP_FILTER_PATH, filter_str);
      break;
    default:
      fprintf(stderr, "set_event_filter: invalid event_flag (event_flag not found). Aborting ...\n");
      exit(EXIT_FAILURE);
      break;
  }
}

/**
 * @brief It allows you to enable or disable the recording of an event specified by the subsystem and event parameters.
 * @param subsystem is a string that specify subsystem's name of the event.
 * @param event is a string that specify the event's name. Set it to NULL if you want to use a default filter
 * specified by the "event_flag" parameter.
 * @param op is short value that can be DISABLE (0) or ENABLE (1) and it will disable or enable the event recording respectively
*/
void event_record_custom(const char* subsystem, const char* event, short op){
  char* enable_path;
  int enable_path_len;
  char op_character[2] = {0};

  op_character[0] = op + '0';
  if(op == ENABLE || op == DISABLE){
    enable_path_len = strlen(EVENTS_PATH) + 1 + strlen(subsystem) + 1 + strlen(event) + strlen("/enable") + 1;
    enable_path = (char*)calloc(sizeof(*enable_path), enable_path_len);
    if(enable_path == NULL){
      fprintf(stderr, "event_record_custom: error allocating memory. Aborting ...\n");
      PRINT_ERROR;
      exit(EXIT_FAILURE);
    }
    tracing_write(enable_path, op_character);
    free(enable_path);
  }else{
    fprintf(stderr, "event_record_custom: invalid op. Aborting ...\n");
    exit(EXIT_FAILURE);
  }
}

/**
 * @brief It allows you to enable or disable the recording of some specific event defined in the library.
 * @param event_flag is short value indicating the event and it's used to enable or disable the recording that event
 * in the kernel trace. Possible values can be: E_SCHED_SWITCH, E_SCHED_WAKE_UP and E_SCHED_MIGRATE_TASK.
 * @param op is short value that can be DISABLE or ENABLE and it will disable or enable the event recording respectively.
*/
void event_record(short event_flag, short op){
  char op_character[2] = {0};

  op_character[0] = op + '0';
  if(op == ENABLE || op == DISABLE){
    switch(event_flag){
    case E_SCHED_SWITCH:
      tracing_write(SCHED_SWITCH_ENABLE_PATH, op_character);
      break;
    case E_SCHED_WAKE_UP:
      tracing_write(SCHED_WAKE_UP_ENABLE_PATH, op_character);
      break;
    case E_SCHED_MIGRATE_TASK:
      tracing_write(SCHED_MIGRATE_TASK_ENABLE_PATH, op_character);
      break;
    default:
      fprintf(stderr, "event_record: event_flag specified doesn't exist in the library. Aborting ...\n");
      exit(EXIT_FAILURE);
      break;
    }
  }else{
    fprintf(stderr, "event_record: invalid op. Aborting ...\n");
    exit(EXIT_FAILURE);
  }
  
}

/**
 * @brief It structures the information contained within the "exec_info" struct into a string.
 * @param info is a pointer to an "exec_info" struct.
 * @return a pointer to the formatted string.
*/
char* exec_info_to_str(void* info){
  char* str;
  int str_len;
  int int_max_size = ceil(log10(INT_MAX)) + 1;
  int long_max_size = ceil(log10(LONG_MAX)) + 1;
  exec_info* e_info = (exec_info*)info;
  
  
  str_len = strlen(e_info->id) + 2 + int_max_size + 2 + long_max_size + 2 + strlen(e_info->mode) + 2;
  str = (char*)calloc(str_len, sizeof(*str));
  if(str == NULL){
    fprintf(stderr, "exec_info_to_str: error allocating memory. Aborting ...\n");
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }
  sprintf(str, "%s, %d, %ld, %s\n", e_info->id, e_info->job_number, e_info->parameter, e_info->mode);
  return str;
}

/**
 * @brief It saves the execution details on a file.
 * @param dir_path is the directory path where to log the execution informations. It will create a subfolder in this path 
 * containing the execution details.
 * @param identifier is a user-defined string that identify the execution. It can be obtained by calling the "generate_execution_identifier" function.
 * @param info is a pointer to a user-defined struct that contains information about an execution. It can also be a default struct "exec_info". The default
 * structure "exec_info" has the following predefined fields related to the execution: "id", "job_number", "parameter" and "mode". To desire the use of the default struct,
 * allocate that structure and pass the pointer to that structure.
 * @param info_to_str is a user-defined function that allows converting the "info" structure into a formatted string as desired. It 
 * can also be a default function that converts the exec_info structure into a string. To desire this latter behavior, set this 
 * parameter to NULL.
 * @param flag a short value used to specify whether to use the library's internal struct and function related to the execution information or
 * whether to use user-defined struct and function passed as parameters. To obtain the first behavior, set the value of this parameter 
 * to DEFAULT_INFO and pass an "exec_info" struct in "info". To obtain the second behavior, set the value of this parameter to USER_INFO.
*/
void log_execution_info(const char* dir_path, const char* identifier, void* info, char* (*info_to_str)(void*), short flag){
  int fd;
  char* str;
  char* dir_file_path;
  int dir_file_path_len;
  char* file_path;
  int file_path_len;
  size_t bytes_written;

  if(flag == USER_INFO && info_to_str != NULL){
    str = info_to_str(info);
  }else if(flag == DEFAULT_INFO){
    str = exec_info_to_str(info);
  }else{
    fprintf(stderr, "log_execution_info: invalid flag. Aborting ...\n");
    exit(EXIT_FAILURE);
  }

  //Verify that the path provided by the user actually exists, if not create it
  if(access(dir_path, F_OK) == -1) {
    if(mkdir(dir_path, 0666) == -1){
      fprintf(stderr, "log_execution_info: error creating the folder \"%s\". Aborting ...\n", dir_path);
      PRINT_ERROR;
      exit(EXIT_FAILURE);
    }else{
      if (chown(dir_path, getuid(), getgid()) != 0) {
        fprintf(stderr, "log_execution_info: chown failed. Aborting ...\n");
        PRINT_ERROR;
        exit(EXIT_FAILURE);
      }
    }
  }

  //Create the subfolder containing execution informations
  dir_file_path_len = strlen(dir_path) + 1 + strlen(identifier) + 1;
  dir_file_path = (char*)calloc(dir_file_path_len, sizeof(*dir_file_path));
  if(dir_file_path == NULL){
    fprintf(stderr, "log_execution_info: error allocating memory. Aborting ...\n");
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }
  sprintf(dir_file_path, "%s/%s", dir_path, identifier);
  if(access(dir_file_path, F_OK) == -1) {
    if(mkdir(dir_file_path, 0666) == -1){
      fprintf(stderr, "log_execution_info: error creating the folder \"%s\". Aborting ...\n", dir_file_path);
      PRINT_ERROR;
      exit(EXIT_FAILURE);
    }else{
      if (chown(dir_file_path, getuid(), getgid()) != 0) {
        fprintf(stderr, "log_execution_info: chown failed. Aborting ...\n");
        PRINT_ERROR;
        exit(EXIT_FAILURE);
      }
    }
  }

  file_path_len = dir_file_path_len + 9; // strlen("/exec.txt") == 9. The '\0' character already counted in dir_file_path_len 
  file_path = (char*)calloc(file_path_len, sizeof(*file_path));
  if(file_path == NULL){
    fprintf(stderr, "log_execution_info: error allocating memory. Aborting ...\n");
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }
  sprintf(file_path, "%s/exec.txt", dir_file_path);

  fd = open(file_path, O_WRONLY | O_CREAT, 0666);
  if(fd == -1){
    fprintf(stderr, "log_execution_info: error opening \"%s\" file. Aborting ...\n", file_path);
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }

  bytes_written = write(fd, str, strlen(str));
  if(bytes_written == -1){
    fprintf(stderr, "log_execution_info: error writing to \"%s\" file. Aborting ...\n", file_path);
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }
  
  close(fd);
  free(str);
  free(dir_file_path);
  free(file_path);
}


/**
 * @brief It allows to save the kernel trace to the specified file
 * @param dir_path is the path of the directory where will be saved the kernel trace along with its
 * execution informations
 * @param identifier is a user-defined string that identify the execution.
*/
void log_trace(const char* dir_path, char* identifier){
  int fd_read, fd_write;
  char buffer[BUFFER_SIZE];
  char* dir_file_path;
  int dir_file_path_len;
  char* file_path;
  int file_path_len;

  #ifdef USE_TRACE_PIPE
  fd_read = open(TRACE_PIPE_PATH, O_RDONLY | O_NONBLOCK);
  #else
  fd_read = open(TRACE_PATH, O_RDWR);
  #endif
  if(fd_read == -1){
    #ifdef USE_TRACE_PIPE
    fprintf(stderr, "log_trace: error opening \"%s\" file. Aborting ...\n", TRACE_PIPE_PATH);
    #else
    fprintf(stderr, "log_trace: error opening \"%s\" file. Aborting ...\n", TRACE_PATH);
    #endif
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }

  //Verify that the path provided by the user actually exists. If it doesn't exist, create it.
  if(access(dir_path, F_OK) == -1) {
    if(mkdir(dir_path, 0666) == -1){
      fprintf(stderr, "log_execution_info: error creating the folder \"%s\". Aborting ...\n", dir_path);
      PRINT_ERROR;
      exit(EXIT_FAILURE);
    }else{
      if (chown(dir_path, getuid(), getgid()) != 0) {
        fprintf(stderr, "log_trace: chown failed. Aborting ...\n");
        PRINT_ERROR;
        exit(EXIT_FAILURE);
      }
    }
  }

  //Create the subfolder containing the kernel trace only if it hasn't already been created
  dir_file_path_len = strlen(dir_path) + 1 + strlen(identifier) + 1;
  dir_file_path = (char*)calloc(dir_file_path_len, sizeof(*dir_file_path));
  if(dir_file_path == NULL){
    fprintf(stderr, "log_execution_info: error allocating memory. Aborting ...\n");
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }
  sprintf(dir_file_path, "%s/%s", dir_path, identifier);
  if(access(dir_file_path, F_OK) == -1) {
    if(mkdir(dir_file_path, 0666) == -1){
      fprintf(stderr, "log_execution_info: error creating the folder \"%s\". Aborting ...\n", dir_file_path);
      PRINT_ERROR;
      exit(EXIT_FAILURE);
    }else{
      if (chown(dir_file_path, getuid(), getgid()) != 0) {
        fprintf(stderr, "log_trace: chown failed. Aborting ...\n");
        PRINT_ERROR;
        exit(EXIT_FAILURE);
      }
    }
  }

  file_path_len = dir_file_path_len + 10; // strlen("/trace.txt") == 10. The '\0' character already counted in dir_file_path_len 
  file_path = (char*)calloc(file_path_len, sizeof(*file_path));
  if(file_path == NULL){
    fprintf(stderr, "log_execution_info: error allocating memory. Aborting ...\n");
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }
  sprintf(file_path, "%s/trace.txt", dir_file_path);

  fd_write = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if(fd_write == -1){
    fprintf(stderr, "log_trace: error opening \"%s\" file. Aborting ...\n", file_path);
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }

  ssize_t bytes_read;
  while ((bytes_read = read(fd_read, buffer, BUFFER_SIZE)) > 0) {
    if(write(fd_write, buffer, bytes_read) != bytes_read) {
      fprintf(stderr, "log_trace: error writing to \"%s\" file. Aborting ...\n", file_path);
      PRINT_ERROR;
      exit(EXIT_FAILURE);
    }
  }

  #ifndef USE_TRACE_PIPE
  write(fd_read, "0", 1);
  #endif

  // Close both files
  close(fd_read);
  close(fd_write);
  free(dir_file_path);
  free(file_path);
}