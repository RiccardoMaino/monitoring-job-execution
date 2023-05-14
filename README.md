## <u>Index</u>
- [Event Tracing C Library](#event-tracing-c-library)
- [Analysis Module Python](#analysis-module-python)

------

# <u>Event Tracing C Library</u> 
## <u>Compilation of the Code</u>
- **test_app:** To compile the code it is sufficient to execute *"make app"* command in your current cloned directory.

## <u>Execution of the Code</u>
- **test_app:** To run the app, first move under the *"/bin"* folder by running `cd event_tracing_library/bin` command. After that run `sudo ./test_app` followed by a series of optional command line arguments listed below:
  1. **--param PARAM:**  A long integer used to set the parameter value to use with the first job. Default is 10000. The PARAM argument must be a positive long integer. After the first job, the parameter used with the others jobs will be calculated based on a function defined in the code.
  2. **--mode MODE:** An integer used to set the kind of the job to perform. Default is 3. The MODE argument must be an integer value between the ones here:
      * 1: Empty loop job
      * 2: Exchanging variables job
      * 3: Ordering a list jpb
  3. **--policy POLICY:** A string used to set the scheduling policy of the jobs executed. Default is SCHED_OTHER. The POLICY argument must be a string value between the ones specified here:.
      * SCHED_OTHER: Non-real-time scheduling policy
      * SCHED_FIFO: Real-time scheduling policy
      * SCHED_RR: Real-time scheduling policy
      * SCHED_BATCH: Non-real-time scheduling policy
      * SCHED_IDLE: Non-real-time scheduling policy
      * SCHED_DEADLINE: Deadline scheduling policy
  4. **--priority PRIO:** An integer used to set the scheduler priority of the jobs executed. Default is 0. The PRIO argument must be an intger value between the ones specified here:
      * 1-99: between 1 (low priority) and 99 (high priority) for SCHED_FIFO or SCHED_RR policies
      * 0: for other policies
  5. **--jobs JOBS:** An integer used to set the number of jobs executed. Default is 50. The JOBS argument must be a positive integer.
  6. **--respath RESPATH:** A string used to set the path where to save all the tracing related data. Default is '../results'.
  7. **--nowait**: A flag used to specify to omit waiting of one second between each job.
- **sampling.sh**: Another way to run the app multiple times is by using a script that launches the app with different predefined parameters in order to create different data regarding different types of execution. To launch the script, first move under the *"/bin"* folder by running the `cd event_tracing_library/bin` command. After that launch the script by typing `./sampling.sh` in the terminal. It may be necessary to provide execution permissions to the file, to do this run the command `chmod +x sampling.sh` before launching the script.
## <u>Output</u>
After the execution of the code, the path specified by the command line arguments (or by default *"../results"*) will be used to keep all the data of the related jobs executions with the respective kernel trace produced. Each execution of the program will create a subfolder in the path specified (or by default in "*../results*") and that subfolder will contain all the details of the jobs and the kernel trace of that execution. This subfolder is called as the same as the execution identifier used in the code for the *"id"* field of the *"exec_info"* structure defined in the library.



# <u>Analysis Module Python</u> 
## <u>Requirements</u>
There are two requirements to satisfy: 
1. To make this module work correctly, it is necessary to install some packages if they are not already installed. To do this, we can take advantage of the Python *"pip"* utility.

    So additionally, you’ll need to make sure you have *"pip"* available. You can check this by running:<br>
    &nbsp;&nbsp;&nbsp;`python3 -m pip --version` 

    If pip isn’t already installed, then first try to bootstrap it from the standard library by running:<br>
    &nbsp;&nbsp;&nbsp;`python3 -m ensurepip --default-pip`

    If that still doesn’t allow you to run `python -m pip`:
    - Securely download [get-pip.py](https://bootstrap.pypa.io/get-pip.py)
    - Run `python get-pip.py`. This will install or upgrade *"pip"*


    Once we have *"pip"* set up correctly, the necessary packages to install are:
    - **numpy**: you can install it by running `pip install numpy`
    - **pandas**: you can install it by running `pip install pandas`
    - **matplotlib**: you can install it by running `pip install matplotlib`
    - **seaborn**: you can install it by running `pip install seaborn`
    All the packages could be installed manually one by one or using the *"pydeps"* target of the *"make"* utility to install all them togheter. To do this last thing just run `make pydeps` in the folder cloned *"/monitoring-job-execution"*.
2. Before running the app, obviously we need some tracing data that must be obtained by using the *"event_tracing"* C library. See the above [section](#event-tracing-c-library) for more details.

## <u>Execution of the Code</u>
- **app.py:** To run the app, first move under the *"/py_analysis_module"* folder by running `cd py_analysis_module` command. After that run `python3 main.py` followed by a series of optional command line arguments listed below:
  1. **--respath RESPATH:**: A string used to set the path where to find the information used to create (or to update) the dataset containing records regarding job execution details. (default: ../event_tracing_library/results)
  2. **--csvpath CSVPATH:**: A string used to set the path to a file where to store (or to find) the newly (or the previously) created dataset. (default: dataset.csv)
  3. **--plotspath PLOTSPATH**: A string used to set the path where to store plots regarding the dataset data. (default: plots)
  4. **--name NAME**: a string used to specify the process name of the program traced using the event_tracing C library. (default: test_app)
  5. **--makeplots**: A flag which if specified allows to create useful plots for the analysis of records contained in the dataset created (or updated). (default: False)
  6. **--delcsv**: A flag which if specified allows to delete the dataset previously created. (default: False)
  7. **--delplots**: A flag which if specified allows to delete the plots, related to the dataset, previously created. (default: False)
- **make pyrun** Another way to run the app using the default command line arguments is bu using the *"pyrun"* target of the *"make"* utility. To do this last thing just run `make pyrun` in the folder cloned *"/monitoring-job-execution"*.
## <u>Output</u>
After running the code, a dataset will be created (or updated) in the path specified by the commands line arguments with **--csvpath** option. The data used to create the dataset is taken from the folder used with the *“event_tracing”* C library to save tracing data results, the default one is *"../event_tracing_library/results"*. In addition, several plots useful for analyzing the data contained in the dataset will be generated if the flag **--makeplots** is specified. These plots could be found within the folder specified by the command line arguments with **--plotspath** option, the default one is *"/plots"*.