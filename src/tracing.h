#ifndef UTILS_H_
#define UTILS_H_

#include <linux/types.h>

#define MAX_IDENTIFIER_SIZE 20
#define BUFFER_SIZE 4096

#define TRACING_ON_PATH "/sys/kernel/tracing/tracing_on"
#define TRACE_MARKER_PATH "/sys/kernel/tracing/trace_marker"
#define TRACE_PIPE_PATH "/sys/kernel/tracing/trace_pipe"
#define TRACE_PATH "/sys/kernel/tracing/trace"
#define EVENTS_PATH "/sys/kernel/tracing/events"

#define E_SCHED_SWITCH 1
#define E_SCHED_WAKE_UP 2
#define E_SCHED_MIGRATE_TASK 3

#define SCHED_SWITCH_FILTER_PATH "/sys/kernel/tracing/events/sched/sched_switch/filter"
#define SCHED_SWITCH_ENABLE_PATH "/sys/kernel/tracing/events/sched/sched_switch/enable"

#define SCHED_WAKE_UP_FILTER_PATH "/sys/kernel/tracing/events/sched/sched_wake_up/filter"
#define SCHED_WAKE_UP_ENABLE_PATH "/sys/kernel/tracing/events/sched/sched_wake_up/enable"

#define SCHED_MIGRATE_TASK_FILTER_PATH "/sys/kernel/tracing/events/sched/sched_migrate_task/filter"
#define SCHED_MIGRATE_TASK_ENABLE_PATH "/sys/kernel/tracing/events/sched/sched_migrate_task/enable"

#define DISABLE 0
#define ENABLE 1

#define STOP 0
#define START 1

#define DEFAULT_INFO 0
#define USER_INFO 1

#define SCHED_OTHER 0			/* non-real-time scheduling policy */
#define SCHED_FIFO 1			/* real-time scheduling policy */
#define SCHED_RR 2				/* real-time scheduling policy */
#define SCHED_BATCH 3			/* non-real-time scheduling policy */
#define SCHED_IDLE 5			/* non-real-time scheduling policy */
#define SCHED_DEADLINE 6	/* deadline scheduling policy */

/**
 * @brief It permits to enable the tracing infrastrucure in order to
 * effectively use the others functions in the library
*/
#define ENABLE_TRACING tracing_write(TRACING_ON_PATH, "1");

/**
 * @brief It permits to disable the tracing infrastrucure that was 
 * previously enabled.
*/
#define DISABLE_TRACING tracing_write(TRACING_ON_PATH, "0");

/**
 * @brief It permits to write on the kernel trace a string passed as parameter.
 * @param str is the string to be written on the kernel trace
*/
#define TRACE_MARK(str) tracing_write(TRACE_MARKER_PATH, str); 

/**
 * @brief It permits to print out on stderror the string associated to
 * the errno value
*/
#define PRINT_ERROR fprintf(stderr, "%s:%d: Error %d \"%s\"\n", __FILE__, __LINE__, errno, strerror(errno));

/**
 * @brief It is a structure that contains information that characterizes the execution. This
 * is a default structure that can be used by the user, or the user can define their own structure
 * to store different execution data.
*/
typedef struct exec_info{
  char* id;
  int job_number;
  long parameter;
  char* mode;
	char* sched_policy;
	int sched_priority;
} exec_info;

/**
 * @brief It is the redefinition of a structure of the linux kernel that serves to indicate the scheduler
 * attributes and scheduling policy related to a process. This structure is utilized to set the scheduling policy and attributes 
 * for a thread
*/
 struct sched_attr {
	__u32 size;							/* Size of this structure */
	__u32 sched_policy;			/* Policy (SCHED_*) */
	__u64 sched_flags;			/* Flags */
	__s32 sched_nice; 			/* Nice value (SCHED_OTHER, SCHED_BATCH) */
	__u32 sched_priority;		/* Static priority (SCHED_FIFO, SCHED_RR) */
	/* Remaining fields are for SCHED_DEADLINE (in nsec)*/
	__u64 sched_runtime;
	__u64 sched_deadline;
	__u64 sched_period;
};

/**
 * @brief It structures the information contained within the "exec_info" struct into a string.
 * @param info is a pointer to an "exec_info" struct.
 * @return a pointer to the formatted string.
*/
char* exec_info_to_str(void* info);

/**
 * @brief It allows to generate an identifier for an execution of a job. The identifier
 * is based on the timestamp
 * @return a string identifier. It must be freed after use.
*/
char* generate_execution_identifier();

/**
 * @brief It allows to write within the specified tracing infrastructure file
 * @param file_path is the path to the tracing infrastructure file 
 * @param str is the string that will be written to the file
*/
void tracing_write(const char* file_path, const char* str);

/**
 * @brief It permits to write on the kernel the start or the end of a generic 
 * job which is identified by the job number parameter
 * @param job_number is a integer value that identify the job
 * @param flag is a short value that can be START or STOP based on what we want to
 * mark on the kernel trace. START is used to mark the start of the job. STOP is used to mark
 * the the end of the job.
*/
void trace_mark_job(int job_number, short flag);

/**
 * @brief It allows you to set a custom filter for any events in any subsystem.
 * @param subsystem is a string that specify subsystem's name of the event.
 * @param event is a string that specify the event's name.
 * @param filter_str is a string indicating the filter value to use for the specified event 
*/
void set_event_filter_custom(const char* subsystem, const char* event, const char* filter_str);

/**
 * @brief It allows you to set default filters of an event
 * @param pid is the pid of the process used in some default filters.
 * @param event_flag is short value indicating the event and it's used to set the default filter defined 
 * in the library for that that event. Possible values can be: E_SCHED_SWITCH, E_SCHED_WAKE_UP and 
 * E_SCHED_MIGRATE_TASK.
*/
void set_event_filter(pid_t pid, short event_flag);

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
void set_scheduler_policy(pid_t pid, __u32 policy, __u32 priority, exec_info* e_info);

/**
 * @brief It allows to retrieve the policy and attributes of the scheduler for the specified process.
 * @param pid is the pid of the process whose scheduling policy and scheduling attributes we want to retrieve
*/
struct sched_attr* get_scheduler_attr(pid_t pid);

/**
 * @brief It allows you to enable or disable the recording of an event specified by the subsystem and event parameters.
 * @param subsystem is a string that specify subsystem's name of the event.
 * @param event is a string that specify the event's name. Set it to NULL if you want to use a default filter
 * specified by the "event_flag" parameter.
 * @param op is short value that can be DISABLE (0) or ENABLE (1) and it will disable or enable the event recording respectively
*/
void event_record_custom(const char* subsystem, const char* event, short op);

/**
 * @brief It allows you to enable or disable the recording of some specific event defined in the library.
 * @param event_flag is short value indicating the event and it's used to enable or disable the recording that event
 * in the kernel trace. Possible values can be: E_SCHED_SWITCH, E_SCHED_WAKE_UP and E_SCHED_MIGRATE_TASK.
 * @param op is short value that can be DISABLE or ENABLE and it will disable or enable the event recording respectively.
*/
void event_record(short event_flag, short op);

/**
 * @brief It allows to save the kernel trace to the specified file
 * @param dir_path is the path of the directory where will be saved the kernel trace along with its
 * execution informations
 * @param identifier is a user-defined string that identify the execution.
*/
void log_trace(const char* dir_path, char* identifier);

/**
 * @brief It saves the execution details on a file.
 * @param dir_path is the directory path where to log the execution informations. It will create a subfolder in this path 
 * containing the execution details.
 * @param identifier is a user-defined string that identify the execution.
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
void log_execution_info(const char* dir_path, const char* identifier, void* info, char* (*info_to_str)(void*), short flag);


#endif