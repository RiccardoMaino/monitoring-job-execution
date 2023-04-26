## <u>Index</u>
- [Event Tracing C Library](#EventTracingCLibrary)
- [Analysis Module Python](#AnalysisModulePython)

------

# <u>Event Tracing C Library</u> 
## <u>Compilation of the Code</u>
- **test_app:** To compile the code it is sufficient to move to the *"event_tracing_library"* folder and execute the *"make app"* command.

## <u>Execution of the Code</u>
- **test_app:** To run the app, first go to the *"event_tracing_library/bin"* folder and type in a terminal *"sudo ./test_app"* followed by a series of command line arguments listed below:
  1. **parameter_data:**  it is an integer value representing the parameter to use with the first job. After the first job it will be updated based on some function defined in the code.
  2. **mode:** it is an integer value representing the job mode.
      * 1: Empty loop
      * 2: Exchanging variables
      * 3: Ordering a list
  3. **policy:** it is an integer value representing the scheduling policy to use with the jobs.
      * 0: SCHED_OTHER
      * 1: SCHED_FIFO
      * 2: SCHED_RR
      * 3: SCHED_BATCH
      * 5: SCHED_IDLE
      * 6: SCHED_DEADLINE
  4. **priority:** it is an integer value representing the scheduler priority to use with SCHED_FIFO and SCHED_RR policies.
      * 1-99: between 1 (low priority) and 99 (high priority) for SCHED_FIFO or SCHED_RR policies
      * 0: for other policies
- **sampling.sh**: Another way to run the app multiple times is by using a script that launches the execution with different predefined parameters in order to create different data regarding different types of execution. To launch the script, go to the *"event_tracing_library/bin"* folder and type *"./sampling.sh"* in the terminal. It may be necessary to provide execution permissions to the file, to do this execute the command *"chmod +x sampling.sh"*
## <u>Output</u>
After the execution of the code, a *"results"* folder is created which will be used to keep all the data of the related jobs executions with the respective kernel trace produced. Each execution of the program will create a subfolder in *"results"* that will contain the details of the jobs and the kernel trace of that execution. This subfolder is identify by the same identfier used in the code for the *"id"* field of the **exec_info struct**.