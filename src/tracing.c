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


#ifdef __x86_64__
#define __NR_sched_setattr		314
#define __NR_sched_getattr		315
#endif

#ifdef __i386__
#define __NR_sched_setattr		351
#define __NR_sched_getattr		352
#endif

#ifdef __arm__
#define __NR_sched_setattr		380
#define __NR_sched_getattr		381
#endif

/**
 * @brief Generates an identifier based on the timestamp for a program execution.
 * @return a string identifier of the program execution. It must be freed after use.
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
 * @brief Creates an exec_info struct that will store job execution data. This structure must be deallocated using the "destroy_exec_info()" function.
 * @param job_number is an integer that identify the job
 * @param parameter is a long value representing the parameter utilized for the job identified by the job_number parameter
 * @param details is a string useful to store additional data about the execution or job details. If no details are needed,
 * set this parameter to NULL.
 * @return a pointer to the newly created exec_info struct. Remember to deallocate this structure using the "destroy_exec_info()" function when
 * you're done with it.
*/
exec_info* create_exec_info(int job_number, long parameter, char* details){
  exec_info* e_info;
  struct sched_attr* s_attr;
  e_info = (exec_info*)malloc(sizeof(*e_info));
  bzero(e_info, sizeof(*e_info));

  e_info->id = generate_execution_identifier();

  s_attr = get_scheduler_attr(0);
  switch (s_attr->sched_policy){
    case SCHED_FIFO:
      e_info->sched_policy = "SCHED_FIFO";
      break;
    case SCHED_RR:
      e_info->sched_policy = "SCHED_RR";
      break;
    case SCHED_BATCH:
      e_info->sched_policy = "SCHED_BATCH";
      break;
    case SCHED_IDLE:
      e_info->sched_policy = "SCHED_IDLE";
      break;
    case SCHED_DEADLINE:
      e_info->sched_policy = "SCHED_DEADLINE";
      break;
    case SCHED_OTHER:
      e_info->sched_policy = "SCHED_OTHER";
      break;
    default:
      e_info->sched_policy = "UNDEFINED";
      break;
  }
  e_info->sched_priority = s_attr->sched_priority;
  e_info->details = details;
  free(s_attr);
  return e_info;
}

/**
 * @brief It frees up the memory allocated for the exec_info struct pointed by
 * the e_info parameter
 * @param e_info is a pointer to an exec_info struct
*/
void destroy_exec_info(exec_info* e_info){
  free(e_info->id);
  free(e_info);
}

/**
 * @brief Writes a string to the specified file within the tracing infrastructure directory.
 * @param file_path The path to a file of the tracing infrastructure file.
 * @param str A pointer to a string that will be written to the file located in file_path
*/
void tracing_write(const char* file_path, const char* str){
  int fd;
  size_t bytes_written;

  if(str != NULL){
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
}

/**
 * @brief Writes the beginning or the end of a job, identified by the job_number parameter,
 * on the kernel trace
 * @param job_number An integer value that identifies a job during a program execution
 * @param flag A short value that can be START or STOP based on what we want to mark on the kernel trace.
 * The START flag is used to mark the beginning of the job identified by the job_number parameter. The STOP flag
 * is used to mark the end of the job identified by the job_number parameter.
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
 * @brief Sets a custom filter for an event in order to change the trace output based
 * on that filter.
 * @param subsystem A pointer to a string that specifies subsystem's name of the event.
 * @param event A pointer to a string that specifies the event's name.
 * @param filter_str A pointer to a string that specifies the filter value to use.
 * @param flag A short integer value that specifies whether to set or reset the filter. Use the SET macro to 
 * set the filter and the RESET macro to reset it. 
*/
void set_event_filter_custom(const char* subsystem, const char* event, const char* filter_str, short flag){
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
  if(flag == SET)
    tracing_write(filter_path, filter_str);
  else
    tracing_write(filter_path, "0");
  free(filter_path);
}

/**
 * @brief Sets a default filter predefined in the library for an event specified by the event_flag parameter, in 
 * order to change the trace output based on that filter.
 * @param pid The pid of the process used to filter events related to that process.
 * @param event_flag A short integer value indicating the event type. Possible values can be: E_SCHED_SWITCH, 
 * E_SCHED_WAKEUP and E_SCHED_MIGRATE_TASK.
 * @param flag A short integer value that specifies whether to set or reset the filter. Use the SET macro to 
 * set the filter and the RESET macro to reset it. 
*/
void set_event_filter(pid_t pid, short event_flag, short flag){
  int str_max_size = ceil(log10(INT_MAX))*2 + 25;
  char filter_str[str_max_size];

  switch(event_flag){
    case E_SCHED_SWITCH:
      if(flag == SET)
        sprintf(filter_str, "prev_pid==%d || next_pid==%d", pid, pid);
      else
        sprintf(filter_str, "0");
      tracing_write(SCHED_SWITCH_FILTER_PATH, filter_str);
      break;
    case E_SCHED_WAKEUP:
      if(flag == SET)
        sprintf(filter_str, "pid==%d", pid);
      else
        sprintf(filter_str, "0");
      tracing_write(SCHED_WAKEUP_FILTER_PATH, filter_str);
      break;
    case E_SCHED_MIGRATE_TASK:
      if(flag == SET)
        sprintf(filter_str, "pid==%d", pid);
      else
        sprintf(filter_str, "0");
      tracing_write(SCHED_MIGRATE_TASK_FILTER_PATH, filter_str);
      break;
    default:
      fprintf(stderr, "set_event_filter: invalid event_flag (event_flag not found). Aborting ...\n");
      exit(EXIT_FAILURE);
      break;
  }
}

/**
 * @brief A redefinition of the sched_setattr() system call in the Linux kernel. It invokes the system call 
 * using its index in the kernel system call table. The sched_setattr() system call sets the scheduling policy 
 * and attributes for the thread specified by pid. If pid is 0, the calling thread's scheduling policy and 
 * attributes are set.
 * @param pid The pid of the process. Set it to 0 in order to set the scheduler attributes of the
 * calling process.
 * @param attr A pointer to a sched_attr struct which contains the scheduler attributes to be set
 * @param flags Some flags that can be ORed togheter
*/
int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags){
	return syscall(__NR_sched_setattr, pid, attr, flags);
}

/**
 * @brief A redefinition of the sched_getattr() system call in the Linux kernel. It invokes the system call
 * call using its index into the kernel system call table. The sched_getattr() system call fetches the scheduling policy 
 * and the associated attributes for the thread whose ID is specified in pid. If pid equals zero, the scheduling policy and 
 * attributes of the calling thread will be retrieved.
 * @param pid The pid of the process. Set it to 0 in order to retrieve the scheduler attributes of the
 * calling process.
 * @param attr A pointer to a sched_attr strhct which will be filled with the current scheduler attributes of the
 * process identified by the pid parameter
 * @param size The size of the sched_attr struct as known to user space.
 * @param flags is provided to allow for future extensions to the interface; in the current implementation 
 * it must be specified as 0.
*/
int sched_getattr(pid_t pid, struct sched_attr *attr, unsigned int size, unsigned int flags){
  return syscall(__NR_sched_getattr, pid, attr, size, flags);
}

/**
 * @brief It allows to specify a priority and a policy of the scheduler for the specified process. If the
 * policy is set to be SCHED_FIFO or SCHED_RR, then priority must be 0
 * @param pid is the pid of the process whose scheduling policy we want to change
 * @param policy is an integer value that specifies the scheduling policy, as one of the 
 * following SCHED_* values: SCHED_OTHER, SCHED_FIFO, SCHED_RR, SCHED_BATCH and SCHED_IDLE.
 * @param priority it specifies the static priority to be set when specifying sched_policy as SCHED_FIFO
 * or SCHED_RR. The allowed range of priorities for these policies is between 1 (low priority) and 99 
 * (high priority). For other policies, this field must be specified as 0.
 * @param e_info is a pointer to an "exec_info" struct. It is used to update the "sched_policy" and "sched_priority" fields
 * according to the specified parameters. If it set to NULL, the function will ignore this parameter.
*/
void set_scheduler_policy(pid_t pid, __u32 policy, __u32 priority, exec_info* e_info){
  struct sched_attr attr = {0};
  int max_priority = sched_get_priority_max(policy);
  int min_priority = sched_get_priority_min(policy);

  if(priority < min_priority){
    priority = min_priority;
  }

  if(priority > max_priority){
    priority = max_priority;
  }

  if(e_info != NULL){
    switch (policy){
      case SCHED_FIFO:
        e_info->sched_policy = "SCHED_FIFO";
        break;
      case SCHED_RR:
        e_info->sched_policy = "SCHED_RR";
        break;
      case SCHED_BATCH:
        e_info->sched_policy = "SCHED_BATCH";
        break;
      case SCHED_IDLE:
        e_info->sched_policy = "SCHED_IDLE";
        break;
      case SCHED_DEADLINE:
        e_info->sched_policy = "SCHED_DEADLINE";
        break;
      case SCHED_OTHER:
        e_info->sched_policy = "SCHED_OTHER";
        break;
      default:
        e_info->sched_policy = "UNDEFINED";
        break;
    }
    e_info->sched_priority = priority; 
  }

  attr.size = sizeof(attr);
  attr.sched_policy = policy;
  attr.sched_priority = priority;
  attr.sched_flags = 0;
  if(sched_setattr(pid, &attr, 0) < 0){
    fprintf(stderr, "set_scheduler_policy: error setting the scheduler attributes. Aborting ...\n");
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }
}

/**
 * @brief It allows to retrieve the policy and attributes of the scheduler for the specified process. The pointed
 * structure must be freed after use.
 * @param pid is the pid of the process whose scheduling policy and scheduling attributes we want to retrieve
*/
struct sched_attr* get_scheduler_attr(pid_t pid){
  struct sched_attr* attr = (struct sched_attr*)malloc(sizeof(*attr));
  bzero(attr, sizeof(*attr));

  if(sched_getattr(pid, attr, sizeof(*attr), 0) < 0){
    fprintf(stderr, "get_scheduler_policy: error retrieving the scheduler attributes. Aborting ...\n");
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }
  return attr;
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
 * in the kernel trace. Possible values can be: E_SCHED_SWITCH, E_SCHED_WAKEUP and E_SCHED_MIGRATE_TASK.
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
    case E_SCHED_WAKEUP:
      tracing_write(SCHED_WAKEUP_ENABLE_PATH, op_character);
      break;
    case E_SCHED_MIGRATE_TASK:
      tracing_write(SCHED_MIGRATE_TASK_ENABLE_PATH, op_character);
      break;
    default:
      fprintf(stderr, "event_record: invalid event_flag (event_flag not found). Aborting ...\n");
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
 * @return a pointer to the comma separeted string containing all the fields of the "exec_info" struct 
*/
char* exec_info_to_str(void* info){
  char* str;
  int str_len;
  int int_max_size = ceil(log10(INT_MAX)) + 1;
  int long_max_size = ceil(log10(LONG_MAX)) + 1;
  exec_info* e_info = (exec_info*)info;
  
  if(e_info->details != NULL)
    str_len = strlen(e_info->id) + 2 + int_max_size + 2 + long_max_size + 2 + strlen(e_info->sched_policy) + 2 + int_max_size + 2 + strlen(e_info->details) + 2;
  else
    str_len = strlen(e_info->id) + 2 + int_max_size + 2 + long_max_size + 2 + strlen(e_info->sched_policy) + 2 + int_max_size + 2 + 10 + 2 ;

  str = (char*)calloc(str_len, sizeof(*str));
  if(str == NULL){
    fprintf(stderr, "exec_info_to_str: error allocating memory. Aborting ...\n");
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }
  
  if(e_info->details != NULL)
    sprintf(str, "%s, %d, %ld, %s, %d, %s\n", e_info->id, e_info->job_number, e_info->parameter, e_info->sched_policy, e_info->sched_priority, e_info->details);
  else
    sprintf(str, "%s, %d, %ld, %s, %d, No details\n", e_info->id, e_info->job_number, e_info->parameter, e_info->sched_policy, e_info->sched_priority);


  return str;
}

/**
 * @brief It saves the execution details on a file.
 * @param dir_path is the directory path where to log the execution informations. It will create a subfolder in this path 
 * containing the execution details.
 * @param identifier is a user-defined string that identify the execution. It can be obtained by calling the "generate_execution_identifier" function.
 * @param info is a pointer to a user-defined struct that contains information about an execution. It can also be a default struct "exec_info". The default
 * structure "exec_info" has the following predefined fields related to the execution: "id", "job_number", "parameter", "details", "sched_policy" and "sched_priority".
 * To desire the use of the default struct, allocate that structure and pass the pointer to that structure.
 * @param info_to_str is a user-defined function that allows converting the "info" structure into a formatted string as desired. The returned string
 * should be a string with comma separated information of the execution. IT can also be a default function that converts the exec_info structure into a 
 * string. To desire this latter behavior, set this parameter to NULL.
 * @param flag a short value used to specify whether to use the library's internal struct and function related to the execution information or
 * whether to use user-defined struct and function passed as parameters. To obtain the first behavior, set the value of this parameter to DEFAULT_INFO 
 * and pass an "exec_info" struct in "info". To obtain the second behavior, set the value of this parameter to USER_INFO and pass your defined
 * structure and function in "info" and "info_to_str".
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

  //Verify that the path provided by the user actually exists, if not creates it
  if(access(dir_path, F_OK) == -1) {
    if(mkdir(dir_path, 0775) == -1){
      fprintf(stderr, "log_execution_info: error creating the folder \"%s\". Aborting ...\n", dir_path);
      PRINT_ERROR;
      exit(EXIT_FAILURE);
    }
  }

  //Create the subfolder containing execution information
  dir_file_path_len = strlen(dir_path) + 1 + strlen(identifier) + 1;
  dir_file_path = (char*)calloc(dir_file_path_len, sizeof(*dir_file_path));
  if(dir_file_path == NULL){
    fprintf(stderr, "log_execution_info: error allocating memory. Aborting ...\n");
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }
  sprintf(dir_file_path, "%s/%s", dir_path, identifier);
  if(access(dir_file_path, F_OK) == -1) {
    if(mkdir(dir_file_path, 0775) == -1){
      fprintf(stderr, "log_execution_info: error creating the folder \"%s\". Aborting ...\n", dir_file_path);
      PRINT_ERROR;
      exit(EXIT_FAILURE);
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

  fd = open(file_path, O_WRONLY | O_CREAT | O_APPEND, 0775);
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
 * @param mode is a short value that allows you to specify whether to use the trace_pipe or trace file to obtain kernel 
 * trace. To desire the first behaviour set this parameter to USE_TRACE_PIPE, to desire the second behaviour set this
 * parameter to USE_TRACE
*/
void log_trace(const char* dir_path, char* identifier, short mode){
  int fd_read, fd_write;
  char buffer[STR_BUFFER_SIZE];
  char* dir_file_path;
  int dir_file_path_len;
  char* file_path;
  int file_path_len;

  if(mode == USE_TRACE_PIPE){
    fd_read = open(TRACE_PIPE_PATH, O_RDONLY | O_NONBLOCK);
  }else{
    fd_read = open(TRACE_PATH, O_RDWR);
  }

  if(fd_read == -1){
    if(mode == USE_TRACE_PIPE){
      fprintf(stderr, "log_trace: error opening \"%s\" file. Aborting ...\n", TRACE_PIPE_PATH);
    }else{
      fprintf(stderr, "log_trace: error opening \"%s\" file. Aborting ...\n", TRACE_PATH);
    }
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }

  //Verify that the path provided by the user actually exists. If it doesn't exist, create it.
  if(access(dir_path, F_OK) == -1) {
    if(mkdir(dir_path, 0775) == -1){
      fprintf(stderr, "log_execution_info: error creating the folder \"%s\". Aborting ...\n", dir_path);
      PRINT_ERROR;
      exit(EXIT_FAILURE);
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
    if(mkdir(dir_file_path, 0775) == -1){
      fprintf(stderr, "log_execution_info: error creating the folder \"%s\". Aborting ...\n", dir_file_path);
      PRINT_ERROR;
      exit(EXIT_FAILURE);
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

  fd_write = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0775);
  if(fd_write == -1){
    fprintf(stderr, "log_trace: error opening \"%s\" file. Aborting ...\n", file_path);
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }

  ssize_t bytes_read;
  while ((bytes_read = read(fd_read, buffer, STR_BUFFER_SIZE)) > 0) {
    if(write(fd_write, buffer, bytes_read) != bytes_read) {
      fprintf(stderr, "log_trace: error writing to \"%s\" file. Aborting ...\n", file_path);
      PRINT_ERROR;
      exit(EXIT_FAILURE);
    }
  }

  if(mode == USE_TRACE) CLEAN_TRACE
  
  // Close both files
  close(fd_read);
  close(fd_write);
  free(dir_file_path);
  free(file_path);
}