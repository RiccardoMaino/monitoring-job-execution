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
 * @brief It allows to save the the in_file_path in an another path
 * @param in_file_path is the path of the file we want to save somewhere else
 * @param out_file_path is the path of the file where we will create a duplicate of the input file
*/
// void save_trace(char * in_file_path, char * out_file_path){
//   FILE *in_file;
//   FILE *out_file;
//   char buffer[4096];
//   size_t read_bytes, written_bytes;

//   in_file = fopen(in_file_path, "r");
//   if(in_file == NULL){
//     fprintf(stderr, "Error reading \"%s\" file. Aborting ...\n", in_file_path);
//     PRINT_ERROR;
//     exit(EXIT_FAILURE);
//   }

//   out_file = fopen(out_file_path, "w");
//   if(out_file == NULL){
//     fprintf(stderr, "Error opening \"%s\" file. Aborting ...\n", out_file_path);
//     PRINT_ERROR;
//     exit(EXIT_FAILURE);
//   }
  
//   printf("DEBUG: init loop\n");
//   while(read_bytes = fread(buffer, sizeof(char), sizeof(buffer), in_file) > 0) {
//     printf("DEBUG: start iteration read_bytes = %ld\n", read_bytes);
//     written_bytes = fwrite(buffer, sizeof(char), read_bytes, out_file);
//     if(written_bytes - read_bytes != 0){
//       PRINT_ERROR;
//       exit(EXIT_FAILURE);
//     }
//     printf("DEBUG: end iteration written_bytes = %ld\n", written_bytes);
//   }

//   fclose(in_file);
//   fclose(out_file);
// }

/**
 * @brief It saves the parameter used for the inner loop on a file. This file will be
 * manipulted by a Python program, to perform some data analysis on time execution, context switch number
 * and migrations number based on the user input
 * @param output_parameter_filename is the file path where to save the parameter
 * @param parameter is the parameter value
 * @param identifier is the id associated with this execution
*/
void save_parameter(char * output_parameter_filename, long parameter, char * identifier){
  FILE *fptr;
  fptr = fopen(output_parameter_filename, "w");
  if(fptr == NULL){
    fprintf(stderr, "Error opening parameter file. Aborting ...\n");
    exit(EXIT_FAILURE);
  }
  fprintf(fptr, "%ld, %s\n", parameter, identifier);
  fclose(fptr);
}

/**
 * cdascsdcsdcsdcsdc
*/
void save_trace(const char * in_file_path, const char * out_file_path){
  int fd_read, fd_write;
  char buffer[BUFFER_SIZE];

  // Open the trace_pipe file for reading
  fd_read = open(in_file_path, O_RDONLY | O_NONBLOCK);
  if(fd_read == -1){
    fprintf(stderr, "Error reading \"%s\" file. Aborting ...\n", in_file_path);
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
    printf("DEBUG: start iteration read_bytes = %ld\n", bytes_read);
    if(write(fd_write, buffer, bytes_read) != bytes_read) {
      perror("Failed to write to trace_pipe_copy");
      exit(EXIT_FAILURE);
    }
    printf("DEBUG: end iteration\n");
  }

  // Close both files
  close(fd_read);
  close(fd_write);
}

// void save_trace(const char *in_file_path, const char *out_file_path) {
//   int in_fd, out_fd;
//   char buffer[BUFFER_SIZE];
//   ssize_t bytes_read, bytes_written;

//   in_fd = open(in_file_path, O_RDONLY | O_NONBLOCK);
//   if (in_fd < 0) {
//     perror("open");
//     exit(EXIT_FAILURE);
//   }

//   out_fd = open(out_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
//   if (out_fd < 0) {
//     perror("open");
//     exit(EXIT_FAILURE);
//   }

//   while (1) {
//     fd_set read_fds;
//     int result;

//     FD_ZERO(&read_fds);
//     FD_SET(in_fd, &read_fds);

//     result = select(in_fd + 1, &read_fds, NULL, NULL, NULL);
//     if (result < 0) {
//       perror("select");
//       exit(EXIT_FAILURE);
//     }

//     if (FD_ISSET(in_fd, &read_fds)) {
//       bytes_read = read(in_fd, buffer, BUFFER_SIZE);
//       if (bytes_read > 0) {
//         bytes_written = write(out_fd, buffer, bytes_read);
//         if (bytes_written != bytes_read) {
//           perror("write");
//           exit(EXIT_FAILURE);
//         }
//       } else if (bytes_read < 0) {
//         if (errno == EAGAIN || errno == EWOULDBLOCK) {
//           // No more data available to read
//         } else {
//           perror("read");
//           exit(EXIT_FAILURE);
//         }
//       } else {
//         // End of file
//         break;
//       }
//     }
//   }

//   close(in_fd);
//   close(out_fd);
// }

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
void do_work_exchange(void * data){
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