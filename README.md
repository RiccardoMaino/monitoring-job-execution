## <u>Index</u>
- [Event Tracing C Library](#EventTracingCLibrary)
- [Analysis Module Python](#AnalysisModulePython)

------

# <u>Event Tracing C Library</u> 
## <u>Compilation of the Code</u>
- **test_app:** To compile the code it is sufficient to execute *"make app"* command in your current cloned directory.

## <u>Execution of the Code</u>
- **test_app:** To run the app, first move under the *"/bin"* folder by running ```cd event_tracing_library/bin``` command. After that run ```sudo ./test_app``` followed by a series of optional command line arguments listed below:
  1. **--param PARAM:**  A long integer used to set the parameter value to use with the first job. Default is 10000. The PARAM argument must be a positive long integer. After the first job, the parameter used with the others jobs will be calculated based on a function defined in the code.
  2. **--mode MODE:** An integer used to set the kind of the job to perform. Default is 3. The MODE argument must be an integer value between the ones here:
      * 1: Empty loop
      * 2: Exchanging variables
      * 3: Ordering a list
  3. **--policy POLICY:** A string used to set the scheduling policy of the jobs executed. Default is SCHED_OTHER. The POLICY argument must be a string value between the ones specified here:.
      * 0: SCHED_OTHER
      * 1: SCHED_FIFO
      * 2: SCHED_RR
      * 3: SCHED_BATCH
      * 5: SCHED_IDLE
      * 6: SCHED_DEADLINE
  4. **--priority PRIO:** An integer used to set the scheduler priority of the jobs executed. Default is 0. The PRIO argument must be an intger value between the ones specified here:
      * 1-99: between 1 (low priority) and 99 (high priority) for SCHED_FIFO or SCHED_RR policies
      * 0: for other policies
  5. **--jobs JOBS:** An integer used to set the number of jobs executed. Default is 50. The JOBS argument must be a positive integer.
  6. **--respath RESPATH:** A string used to set the path where to save all the tracing related data. Default is '../results'.
- **sampling.sh**: Another way to run the app multiple times is by using a script that launches the app with different predefined parameters in order to create different data regarding different types of execution. To launch the script, first move under the *"/bin"* folder by running the ```cd event_tracing_library/bin``` command. After that launch the script by typing ```./sampling.sh``` in the terminal. It may be necessary to provide execution permissions to the file, to do this run the command ```chmod +x sampling.sh``` before launching the script.
## <u>Output</u>
After the execution of the code, the path specified by the command line arguments (or by default *"../results"*) will be used to keep all the data of the related jobs executions with the respective kernel trace produced. Each execution of the program will create a subfolder in the path specified (or by default in "*../results*") and that subfolder will contain all the details of the jobs and the kernel trace of that execution. This subfolder is called as the same as the identfier used in the code for the *"id"* field of the **exec_info** structure.



# <u>Analysis Module Python</u> 
## <u>Requirements</u>
To make this module work correctly, it is necessary to install some packages if they are not already installed. To do this, we can take advantage of the Python *"pip"* utility.

So additionally, you’ll need to make sure you have *"pip"* available. You can check this by running:<br>
&nbsp;&nbsp;&nbsp;`python3 -m pip --version` 

If pip isn’t already installed, then first try to bootstrap it from the standard library by running:<br>
&nbsp;&nbsp;&nbsp;`python3 -m ensurepip --default-pip`

If that still doesn’t allow you to run `python -m pip`:
- Securely download [get-pip.py](https://https://bootstrap.pypa.io/get-pip.py)
- Run `python get-pip.py`. This will install or upgrade *"pip"*


Once we have *"pip"* set up correctly, the necessary packages to install are:
- **pandas**: you can install it by running `pip install pandas`
- **numpy**: you can install it by running `pip install numpy`
- **matplotlib**: you can install it by running `pip install matplotlib`
- **seaborn**: you can install it by running `pip install seaborn`

## <u>Execution of the Code</u>
- **main:** To run the app, first move under the *"/py_analysis_module"* folder by running ```cd py_analysis_module``` command. After that run `python3 main.py` followed by a series of optional command line arguments listed below:
  1. **--param PARAM:**  A long integer used to set the parameter value to use with the first job. Default is 10000. The PARAM argument must be a positive long integer. After the first job, the parameter used with the others jobs will be calculated based on a function defined in the code.
  2. **--csvpath MODE:** An integer used to set the kind of the job to perform. Default is 3. The MODE argument must be an integer value between the ones here:
      * 1: Empty loop
      * 2: Exchanging variables
      * 3: Ordering a list
## <u>Output</u>
After running the code, a dataset will be created (or updated) in the path specified by the commands line arguments with **--csvpath** option. The data used to create the dataset is taken from the folder used with the *“event_tracing”* C library to save tracing data results. In addition, several plots useful for analyzing the data contained in the dataset will be generated within the folder specified by the command line arguments.