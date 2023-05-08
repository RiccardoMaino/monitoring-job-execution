#ifndef EVENT_TRACING_H_
#define EVENT_TRACING_H_

#include <linux/types.h>

#define E_SCHED_SWITCH 1 ///>Macro used to refer to the sched_swtich event.
#define E_SCHED_WAKEUP 2 ///>Macro used to refer to the sched_wakeup event.
#define E_SCHED_MIGRATE_TASK 3 ///>Macro used to refer to the sched_migrate_task event.

#define DISABLE 0	///>Macro used in event_record(), event_record_custom() and event_record_subsystem() to disable the record of one or all events.
#define ENABLE 1 	///>Macro used in event_record(), event_record_custom() and event_record_subsystem() to enable the record of one or all events.

#define STOP 0		///>Macro used in trace_mark_job() to trace mark the end of a job.
#define START 1		///>Macro used in trace_mark_job() to trace mark the beginning of a job.

#define SET 0			///>Macro used in set_event_filter() and set_event_filter_custom() to set the filter provided as a parameter.
#define RESET 1		///>Macro used in set_event_filter() and set_event_filter_custom() to reset/clear the filter.

#define DEFAULT_INFO 0	///>Macro used in log_execution_info() to specify that a user-defined struct is being used.
#define USER_INFO 1			///>Macro used in log_execution_info() to specify that the default exec_info struct is being used.

#define USE_TRACE_PIPE 0	///>Macro used in log_trace() to specify that the kernel trace will be read using /sys/kernel/tracing/trace_pipe.
#define USE_TRACE 1				///>Macro used in log_trace() to specify that the kernel trace will be read using /sys/kernel/tracing/trace.

#define SCHED_OTHER 0			///>Non-real-time scheduling policy.
#define SCHED_FIFO 1			///>Real-time scheduling policy.
#define SCHED_RR 2				///>Real-time scheduling policy. 
#define SCHED_BATCH 3			///>Non-real-time scheduling policy.
#define SCHED_IDLE 5			///>Non-real-time scheduling policy.
#define SCHED_DEADLINE 6	///>Deadline scheduling policy.

#define BUFFER_SIZE_KB 1408 ///>Default ring buffer size of the tracing infrastrucuture expressed in KB.

#define TRACING_ON_PATH "/sys/kernel/tracing/tracing_on" ///>Path to the tracing_on file of the tracefs.
#define TRACE_MARKER_PATH "/sys/kernel/tracing/trace_marker" ///>Path to the trace_marker file of the tracefs.
#define TRACE_PIPE_PATH "/sys/kernel/tracing/trace_pipe" ///>Path to the trace_pipe file of the tracefs.
#define TRACE_PATH "/sys/kernel/tracing/trace" ///>Path to the trace file of the tracefs.
#define BUFFER_SIZE_KB_PATH "/sys/kernel/tracing/buffer_size_kb" ///>Path to the buffer_size_kb file of the tracefs.
#define EVENTS_PATH "/sys/kernel/tracing/events" ///>Path to the events directory of the tracefs.
#define SCHED_SWITCH_FILTER_PATH "/sys/kernel/tracing/events/sched/sched_switch/filter" ///>Path to the filter file of the sched_switch event.
#define SCHED_SWITCH_ENABLE_PATH "/sys/kernel/tracing/events/sched/sched_switch/enable" ///>Path to the enable file of the sched_switch event.
#define SCHED_WAKEUP_FILTER_PATH "/sys/kernel/tracing/events/sched/sched_wakeup/filter" ///>Path to the filter file of the sched_wakeup event.
#define SCHED_WAKEUP_ENABLE_PATH "/sys/kernel/tracing/events/sched/sched_wakeup/enable" ///>Path to the enable file of the sched_wakeup event.
#define SCHED_MIGRATE_TASK_FILTER_PATH "/sys/kernel/tracing/events/sched/sched_migrate_task/filter" ///>Path to the filter file of the sched_migrate_task event.
#define SCHED_MIGRATE_TASK_ENABLE_PATH "/sys/kernel/tracing/events/sched/sched_migrate_task/enable" ///>Path to the enable file of the sched_migrate_task event.

#define MAX_IDENTIFIER_SIZE 20	///>Max length of an identifier generated using generate_execution_identifier() function.
#define STR_BUFFER_SIZE 4096		///>Buffer size for reading and writing operations.

/**
 * @brief Changes the ring buffer size used to store the kernel trace.
 * @param str_buffer_kb A pointer to a string which represents the new number of kb of the ring buffer.
*/
#define SET_BUFFER_SIZE(str_buffer_kb)	if(str_buffer_kb != NULL) { \
																					tracing_write(BUFFER_SIZE_KB_PATH, str_buffer_kb);\
																				}

/**
 * @brief Cleans the kernel trace file located in /sys/kernel/tracing/trace.
*/
#define CLEAN_TRACE if(system("echo 0 > /sys/kernel/tracing/trace") == -1) { \
     	 								fprintf(stderr, "log_trace: error cleaning the \"%s\" file. Aborting ...\n", TRACE_PATH); \
      								exit(EXIT_FAILURE); \
    								}

/**
 * @brief Allows to enable the tracing infrastructure in order to effectively use the others functions in the library.
*/
#define ENABLE_TRACING tracing_write(TRACING_ON_PATH, "1"); \
											 CLEAN_TRACE

/**
 * @brief Allows to disable the tracing infrastructure that was previously enabled.
*/
#define DISABLE_TRACING tracing_write(TRACING_ON_PATH, "0");

/**
 * @brief Writes to the kernel trace a string passed as parameter.
 * @param str A pointer to a the string to be written to the kernel trace.
*/
#define TRACE_MARK(str) tracing_write(TRACE_MARKER_PATH, str); 

/**
 * @brief Prints out on stderror the string associated to the errno value.
*/
#define PRINT_ERROR fprintf(stderr, "%s:%d: Error %d \"%s\"\n", __FILE__, __LINE__, errno, strerror(errno));

/**
 * @brief A structure that contains information that characterizes the job execution. This
 * is a default structure that can be used by the user, or the user can define their own structure
 * to store different job execution data.
*/
typedef struct exec_info{
  char* id;	///> The program execution identifier.
  int job_number; ///> A number that identifies the job in this execution.
  long parameter; ///> The value of the parameter used for this job.
	char* sched_policy; ///> The scheduling policy used for this job.
	int sched_priority; ///> The scheduling priority used for this job.
	char* details; ///> Additional details that the user can provide as a string.
} exec_info;

/**
 * @brief A redefinition of a structure of the linux kernel that serves to set or to retrieve the scheduler
 * attributes and scheduling policy related to a thread.
*/
 struct sched_attr {
	__u32 size;	///> Size in bytes of this structure
	__u32 sched_policy;	///> Policy (SCHED_*)
	__u64 sched_flags; ///> Flags
	__s32 sched_nice; ///> Nice value (SCHED_OTHER, SCHED_BATCH)
	__u32 sched_priority; ///> Static priority (SCHED_FIFO, SCHED_RR)
	__u64 sched_runtime; ///> Field for SCHED_DEADLINE (in nsec)
	__u64 sched_deadline; ///> Field for SCHED_DEADLINE (in nsec)
	__u64 sched_period; ///> Field for SCHED_DEADLINE (in nsec)
};

/**
 * @brief Creates an exec_info struct that will store job execution data.
 * @param job_number  An integer value that identifies a job during a program execution.
 * @param parameter A long integer value representing the parameter utilized for the job identified by the "job_number" parameter.
 * @param details A pointer to a string useful to store additional data about execution or job details. If no details
 * are needed, set this parameter to NULL.
 * @return A pointer to the newly created exec_info struct correctly initialized. This structure must be deallocated using
 * the "destroy_exec_info()" function when you're done with it.
*/
exec_info* create_exec_info(int job_number, long parameter, char* details);

/**
 * @brief Frees up the memory allocated for the exec_info struct pointed by the "e_info" parameter.
 * @param e_info A pointer to an exec_info struct.
*/
void destroy_exec_info(exec_info* e_info);

/**
 * @brief Sets a custom filter for an event in order to change the trace output based
 * on that filter.
 * @param subsystem A pointer to a string that specifies subsystem name of the event.
 * @param event A pointer to a string that specifies the event name.
 * @param filter_str A pointer to a string that specifies the filter value to use. If the RESET flag is used in
 * the "flag" parameter, this parameter could be NULL.
 * @param flag A short integer value that specifies whether to set or reset the filter. Use the SET macro to set
 * the filter and the RESET macro to reset it. 
*/
void set_event_filter_custom(const char* subsystem, const char* event, const char* filter_str, short flag);

/**
 * @brief Sets a default filter predefined in the library for an event specified by the "event_flag" parameter,
 * in order to change the trace output based on that filter.
 * @param pid The pid of the process used to filter events related to that process.
 * @param event_flag A short integer value indicating the event type. Possible values can be: E_SCHED_SWITCH, 
 * E_SCHED_WAKEUP and E_SCHED_MIGRATE_TASK.
 * @param flag A short integer value that specifies whether to set or reset the filter. Use the SET macro to 
 * set the filter and the RESET macro to reset it. 
*/
void set_event_filter(pid_t pid, short event_flag, short flag);

/**
 * @brief Allows specifying scheduler priority and policy of a process identified by the "pid" parameter.
 * @param pid The pid of the process whose scheduling policy and priority we want to change.
 * @param policy An integer value that specifies the scheduling policy, as one of the 
 * following SCHED_* values: SCHED_OTHER, SCHED_FIFO, SCHED_RR, SCHED_BATCH and SCHED_IDLE.
 * @param priority An integer value that specifies the static priority to be set when specifying sched_policy as SCHED_FIFO
 * or SCHED_RR. The allowed range of priorities for these policies is between 1 (low priority) and 99 
 * (high priority). For other policies, this field must be specified as 0.
 * @param e_info A pointer to an exec_info struct. It is used to update the "sched_policy" and "sched_priority" fields of
 * the exec_info struct according to the specified parameters. If it is set to NULL, the function will ignore this parameter.
*/
void set_scheduler_policy(pid_t pid, __u32 policy, __u32 priority, exec_info* e_info);

/**
 * @brief Allows to retrieve the scheduling policy and attributes of the scheduler for the specified process.
 * @param pid The pid of the process whose scheduling policy and scheduling attributes we want to retrieve.
 * @return A pointer to a newly created sched_attr struct containing all scheduler data retrived. This structure
 * must be freed after you're done with it.
*/
struct sched_attr* get_scheduler_attr(pid_t pid);

/**
 * @brief Allows to enable or disable the recording of all events contained in the specified subsystem.
 * @param subsystem A pointer to a string that specifies subsystem name.
 * @param op A short integer value that can be the macros DISABLE or ENABLE and it will disable or enable the
 * recording of all subsystem event respectively.
*/
void event_record_subsystem(const char* subsystem, short op);

/**
 * @brief Allows to enable or disable the recording of an event contained in the specified subsystem.
 * @param subsystem A pointer to a string that specifies subsystem name of the event.
 * @param event A pointer to a string that specifies the event's name. Set it to NULL if you want to use a default 
 * filter.
 * @param op A short integer value that can be the macros DISABLE or ENABLE and it will disable or enable the event 
 * recording respectively.
*/
void event_record_custom(const char* subsystem, const char* event, short op);

/**
 * @brief Allows to enable or disable the recording of some specific event defined in the library.
 * @param event_flag A short value indicating the event and it's used to enable or disable the recording of that event
 * in the kernel trace. Possible values can be: E_SCHED_SWITCH, E_SCHED_WAKEUP and E_SCHED_MIGRATE_TASK.
 * @param op A short value that can be the macros DISABLE or ENABLE and it will disable or enable the event recording respectively.
*/
void event_record(short event_flag, short op);

/**
 * @brief Writes the beginning or the end of a job, identified by the "job_number" parameter,
 * on the kernel trace.
 * @param job_number An integer value that identifies a job during a program execution.
 * @param flag A short integer value that can be START or STOP based on what we want to mark on the kernel trace.
 * The START flag is used to mark the beginning of the job identified by the "job_number" parameter. The STOP flag
 * is used to mark the end of the job identified by the "job_number" parameter.
*/
void trace_mark_job(int job_number, short flag);

/**
 * @brief Saves the job execution information to a specific file within a specified directory.
 * @param dir_path The path to a directory where to save various job executions and trace results. It will create a subfolder in this path
 * specific to this execution (if it doesn't already exist), and then it will create or update the "exec.txt" file by appending an new entry of 
 * the current job execution information. The "dir_path" parameter must be the same as the one provided in the "log_trace()" in order to save
 * job execution information along with its kernel trace.
 * @param identifier The user-defined string that identifies the program execution. It can be obtained by calling the "generate_execution_identifier"
 * function. It should match the identifier field contained in the struct pointed by the "info" parameter, if that field exists in the struct.
 * @param info A pointer to a user-defined struct that contains information about an execution. It can also be a default exec_info struct. The 
 * default structure exec_info has the following predefined fields related to the execution: "id", "job_number", "parameter", "details", 
 * "sched_policy" and "sched_priority". To desire the use of the default struct, allocate that structure and pass a pointer to that structure.
 * @param info_to_str A user-defined function that allows to create a comma separeted string based on the fields contained in the struct pointed 
 * by "info" parameter. If the "info" parameter is pointer to an exec_info struct, set this parameter to NULL.
 * @param flag A short integer value used to specify whether the interal library structure exec_info is being used to store job data or if 
 * a user-defined structure is being used for this purpose. To specify the first behavior, set the value of this 
 * parameter to DEFAULT_INFO and pass an exec_info struct in "info" parameter. To specify the second behavior, set the value of this parameter
 * to USER_INFO and pass your defined structure and function in "info" and "info_to_str" parameters respectively.
*/
void log_execution_info(const char* dir_path, const char* identifier, void* info, char* (*info_to_str)(void*), short flag);

/**
 * @brief Saves the kernel trace to a specific file within a specified directory.
 * @param dir_path The path to a directory where to save various job executions and trace results. It will create a subfolder in this path
 * specific to this execution (if it doesn't already exist), and then it will create the "trace.txt" file which contains the kernel trace of
 * this program execution. The "dir_path" parameter must be the same as the one provided in the "log_execution_info()" in order to save
 * the kernel trace along with its jobs execution information.
 * @param identifier The user-defined string that identifies the program execution. It can be obtained by calling
 * the "generate_execution_identifier()" function. It should match the identifier provided in the "log_execution_info()"
 * during this program execution.
 * @param mode A short integer value that allows you to specify whether to use the trace_pipe or trace file of the tracefs to obtain kernel 
 * trace. To desire the first behaviour set this parameter to USE_TRACE_PIPE, to desire the second behaviour set this
 * parameter to USE_TRACE
*/
void log_trace(const char* dir_path, char* identifier, short mode);

/**
 * @brief Writes a string to the specified file within the tracing infrastructure directory.
 * @param file_path The path to a file of the tracing infrastructure file.
 * @param str A pointer to a string that will be written to the file located in the path specified by 
 * "file_path" parameter.
*/
void tracing_write(const char* file_path, const char* str);

/**
 * @brief Generates an identifier based on the timestamp that can be used for the entire program execution.
 * @param reset A short integer value used to determine whether to create a new identifier or use the one previously created if it exists.
 * To obtain a new identifier use the value 1 for this parameter. To obtain the identifier previously created use the value 0 for this
 * parameter.
 * @return A pointer to a string identifier of the program execution. It must be freed after use.
*/
char* generate_execution_identifier(short reset);

/**
 * @brief Structures and format the information contained within the exec_info struct into a comma separated string.
 * @param info A pointer to an exec_info struct.
 * @return A pointer to the comma separeted string containing all the fields of the exec_info struct.
*/
char* exec_info_to_str(void* info);

#endif