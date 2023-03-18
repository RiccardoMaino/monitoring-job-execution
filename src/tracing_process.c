#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <signal.h>
#include <errno.h>
#include "utils.h"


int main(int argc, char *argv[]){
    int sem_semaphore_id = -1;
    char command[MAX_COMMAND_LENGTH];

    sem_semaphore_id = semget(SEM_SEMAPHORE_KEY, NUM_SEM, 0600);
    sprintf(command, "trace-cmd record -e sched:sched_switch -P %s -o trace.dat", argv[1]);
    system(command);
    printf("Tracing Process: Started tracing ...\n");
    sem_signal(sem_semaphore_id, START_EXECUTION_SEM, 1, STD_FLAG_SEM);
}