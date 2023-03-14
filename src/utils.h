#ifndef UTILS_H_
#define UTILS_H_

/**
 * Macro used to print errors with more information
*/
#define PRINT_ERROR fprintf(stderr,                     \
                            "%s:%d: Error %d \"%s\"\n", \
                            __FILE__, __LINE__, errno, strerror(errno));

/**
 * @brief It performs some kind of job for some time.
 * @param data is some data passed to the function. It use it to perform some actions inside the function.
 */
void do_work(void *data);

#endif