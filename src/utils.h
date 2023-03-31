#ifndef UTILS_H_
#define UTILS_H_

#define MAX_JOBS 10
#define MAX_INPUT_LINE_LENGTH 128
#define MAX_COMMAND_LENGTH 150
#define MAX_FILENAME_LENGTH 80
#define MAX_IDENTIFIER_LENGTH 20

#define BUFFER_SIZE 4096

#define TRACING_ON_PATH "/sys/kernel/tracing/tracing_on"
#define TRACE_MARKER_PATH "/sys/kernel/tracing/trace_marker"
#define TRACE_PIPE_PATH "/sys/kernel/tracing/trace_pipe"
#define FILTER_PATH "/sys/kernel/tracing/events/sched/sched_switch/filter"
#define ENABLE_PATH "/sys/kernel/tracing/events/sched/sched_switch/enable"

#define PRINT_ERROR fprintf(stderr,                     \
                            "%s:%d: Error %d \"%s\"\n", \
                            __FILE__, __LINE__, errno, strerror(errno));

/**
 * @brief It allows to manipulate file system file
 * @param file_path is the path to the file system path
 * @param str is the string to be written on the file
*/
void sys_write(char * file_path, char * str);

/**
 * @brief It allows to save the the in_file_path in an another path
 * @param in_file_path is the path of the file we want to save somewhere else
 * @param out_file_path is the path of the file where we will create a duplicate of the input file
*/
void save_trace(const char * in_file_path, const char * out_file_path);


/**
 * @brief It saves the parameter used for the inner loop on a file. This file will be
 * manipulted by a Python program, to perform some data analysis on time execution, context switch number
 * and migrations number based on the user input
 * @param output_parameter_filename is the file path where to save the parameter
 * @param parameter is the parameter value
 * @param identifier is the id associated with this execution
*/
void save_parameter(char * output_parameter_filename, long parameter, char * identifier, char * mode);

/**
 * @brief It performs some kind of job for some time.
 * @param data is some data passed to the function. It use it to perform some actions inside the function.
 */
void do_work(void *data);

/**
 * @brief It performs some variables swap for num_jobs time.
 * @param data is some data passed to the function. It use it to perform some actions inside the function.
 */
void do_work_exchanging(void * data);

/**
 * @brief It performs a creation of a list and then orders it for num_jobs time.
 * @param data is some data passed to the function. It use it to perform some actions inside the function.
 */
void do_work_ordering(void * data);

#endif