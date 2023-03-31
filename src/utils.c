#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "utils.h"
#include "list.h"


/**
 * @brief It allows to manipulate file system file
 * @param file_path is the path to the file system path
 * @param str is the string to be written on the file
*/
void sys_write(char * file_path, char * str){
  FILE *out_file;

  out_file = fopen(file_path, "w");
  if(out_file == NULL){
    fprintf(stderr, "Error opening \"%s\" file in \"w\" mode. Aborting ...\n", file_path);
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }
  if(fprintf(out_file, "%s", str) < 0){
    fprintf(stderr, "Error writing on \"%s\". Aborting ...\n", file_path);
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }
  fclose(out_file);
}


/**
 * @brief It saves the parameter used for the inner loop on a file. This file will be
 * manipulted by a Python program, to perform some data analysis on time execution, context switch number
 * and migrations number based on the user input
 * @param output_parameter_filename is the file path where to save the parameter
 * @param parameter is the parameter value
 * @param identifier is the id associated with this execution
*/
void save_parameter(char * output_parameter_filename, long parameter, char * identifier, char * mode){
  FILE *fptr;
  
  fptr = fopen(output_parameter_filename, "w");
  if(fptr == NULL){
    fprintf(stderr, "Error opening parameter file. Aborting ...\n");
    exit(EXIT_FAILURE);
  }
  fprintf(fptr, "%ld, %s, %s\n", parameter, identifier, mode);
  fclose(fptr);
}


/**
 * @brief It allows to save the the in_file_path in an another path
 * @param in_file_path is the path of the file we want to save somewhere else
 * @param out_file_path is the path of the file where we will create a duplicate of the input file
*/
void save_trace(const char * in_file_path, const char * out_file_path){
  int fd_read, fd_write;
  char buffer[BUFFER_SIZE];

  // Open the trace_pipe file for reading
  fd_read = open(in_file_path, O_RDONLY | O_NONBLOCK);
  if(fd_read == -1){
    fprintf(stderr, "Error opening \"%s\" pipe. Aborting ...\n", in_file_path);
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }

  // Create a new file for writing
  fd_write = open(out_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if(fd_write == -1){
    fprintf(stderr, "Error opening \"%s\" file. Aborting ...\n", out_file_path);
    PRINT_ERROR;
    exit(EXIT_FAILURE);
  }

  // Read from the trace_pipe file and write to the new file
  ssize_t bytes_read;
  while ((bytes_read = read(fd_read, buffer, BUFFER_SIZE)) > 0) {
    if(write(fd_write, buffer, bytes_read) != bytes_read) {
      fprintf(stderr, "Error writing on \"%s\" file. Aborting ...\n", out_file_path);
      PRINT_ERROR;
      exit(EXIT_FAILURE);
    }
  }

  // Close both files
  close(fd_read);
  close(fd_write);
}


/**
 * @brief It performs a basic loop for num_jobs time.
 * @param data is some data passed to the function. It use it to perform some actions inside the function.
 */
void do_work(void * data){
  int num_jobs = *(int *)data;
  for (int i = 0; i < num_jobs; i++){
  }
}

/**
 * @brief It performs some variables swap for num_jobs time.
 * @param data is some data passed to the function. It use it to perform some actions inside the function.
 */
void do_work_exchanging(void * data){
  int num_jobs = *(int *)data;
  int a, b, temp;
  srand(time(NULL));
  for (int i = 0; i < num_jobs; i++){
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
*/
int compare_integer(void * a, void * b){
  int a_value = *(int *)a;
  int b_value = *(int *)b;
  return a_value - b_value;
}

/**
 * @brief It performs some variables swap for num_jobs time.
 * @param data is some data passed to the function. It use it to perform some actions inside the function.
 */
void do_work_ordering(void * data){
  int n1 = 10, n2 = 30, n3 = 20, n4 = 50, n5 = 40;
  int num_jobs = *(int *)data;
  for(int i = 0; i<num_jobs; i++){
    List * list = list_create();
    list_add(list, &n1);
    list_add(list, &n2);
    list_add(list, &n3);
    list_add(list, &n4);
    list_add(list, &n5);
    list->head = (list->head, compare_integer);
    list_destroy(list);
  }
}