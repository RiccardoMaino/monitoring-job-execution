#ifndef UTILS_H_
#define UTILS_H_

#define MAX_JOBS 10
#define MAX_INPUT_LINE_LENGTH 128
#define MAX_COMMAND_LENGTH 150
#define MAX_FILENAME_LENGTH 80
#define MAX_IDENTIFIER_LENGTH 20

#define TRACE_MARKER_PATH "/sys/kernel/tracing/trace_marker"
#define TRACE_PIPE_PATH "/sys/kernel/tracing/trace_pipe"
#define FILTER_PATH "/sys/kernel/tracing/events/sched/sched_switch/filter"
#define ENABLE_PATH "/sys/kernel/tracing/events/sched/sched_switch/enable"

#define STD_FLAG_SEM 0
#define SEM_SEMAPHORE_KEY 5000
#define NUM_SEM 1
#define START_EXECUTION_SEM 0 	

/**
 * Macro used to print errors with more information
*/
#define PRINT_ERROR fprintf(stderr,                     \
                            "%s:%d: Error %d \"%s\"\n", \
                            __FILE__, __LINE__, errno, strerror(errno));

/**
 * @brief It allows to set the signal handler for a specific signal. Returns the structure representing the old handler
 * @param signum is the signal number representing the signal
 * @param handle_signal is the pointer to the function that handles the signal specified by signum
 * @param sa_mask is the signal mask to use in the handler function
 * @param sa_flags are some special flags that can be set
*/
struct sigaction set_signal_handler(int signum, void (*handle_signal)(int), sigset_t sa_mask, int sa_flags);

/**
 * @brief It obtains sem_op resources if possible using the semaphore specified by sem_id with index sem_num
 * @param sem_id is the id number of the semaphore
 * @param sem_num is the index corresponding to the semaphore in the array of semaphore
 * @param sem_op is a negative integer representing the number of resources we want to obtain. For "Zero Waiting" set it to 0
 * @param sem_flag is a integer representing some flags to assign
 * @return if successful it returns 1, otherwise it returns 0 and sets errno
*/
int sem_wait(int sem_id, unsigned short sem_num, short sem_op, short sem_flag); 

/**
 * @brief Release sem_op resources using semaphore sem_id with index sem_num
 * @param sem_id is the id number of the semaphore
 * @param sem_num is the index corresponding to the semaphore in the array of semaphore
 * @param sem_op is a negative integer representing the number of resources we want to obtain
 * @param sem_flag is a integer representing some flags to assign
 * @return if successful it returns 1, otherwise it returns 0 and sets errno
*/
int sem_signal(int sem_id, unsigned short sem_num, short sem_op, short sem_flag);

/**
 * @brief It performs some kind of job for some time.
 * @param data is some data passed to the function. It use it to perform some actions inside the function.
 */
void do_work(void *data);

#endif