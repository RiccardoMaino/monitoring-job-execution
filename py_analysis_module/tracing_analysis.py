import os
import numpy as np
import pandas as pd
import decimal as dc
import matplotlib.pyplot as plt
import seaborn as sns
import re
from typing import Callable, Any, TextIO, List, Tuple
from datetime import datetime


# Defined regular expressions pattern to match sched_switch events printed in the kernel trace.
# Regular expression that match regular line of a kernel trace
#   Group 1: Process Name
#   Group 2: Process PID
#   Group 3: CPU Core
#   Group 4: Flags
#   Group 5: Timestamp
#   Group 6: Previous Process Name (prev_comm)
#   Group 7: Previous Process PID (prev_pid)
#   Group 8: Previous Process Priority (prev_prio). An integer between 0 and 139. Default priority is 120.
#   Group 9: Previous Process State (prev_state). A letter representing the current state of the process.
#   Group 10: Next Process Name (next_comm)
#   Group 11: Next Process PID (next_pid)
#   Group 12: Next Process Priority (next_prio). An integer between 0 and 139. Default priority is 120.


# Defined regular expressions pattern to match start and end of a job in the kernel trace.
# Regular expression that match regular line of a kernel trace
#   Group 1: Process Name
#   Group 2: Process PID
#   Group 3: CPU Core
#   Group 4: Flags
#   Group 5: Timestamp
#   Group 6: Start or End signal
#   Group 7: Job number

TRACE_LINE_PATTERN = r'([A-Za-z_<>./:0-9-]+)-(\d+)\s+(\[\d+\])\s+([A-Za-z0-9\.]+)\s+(\d+\.\d+):\s+sched_switch:\sprev_comm=([A-Za-z_<>.:\s/0-9-]+)\sprev_pid=(\d+)\sprev_prio=(\d+)\sprev_state=[A-Z]\s==>\snext_comm=([A-Za-z_<>.:\s/0-9-]+)\snext_pid=(\d+)\snext_prio=(\d+)'
TRACE_JOB_LINE_PATTERN = r'(test_app|[<>.]+)-(\d+)\s+(\[\d+\])\s+([A-Za-z0-9\.]+)\s+(\d+\.\d+):\s+tracing_mark_write:\s(start|end)_job=([0-9]+)'
JOB_GROUP_MATCHER = {
    "name": 1,
    "pid": 2,
    "cpu_core": 3,
    "flags": 4,
    "timestamp": 5,
    "job_state": 6,
    "job_number": 7
}
LINE_GROUP_MATCHER = {
    "name": 1,
    "pid": 2,
    "cpu_core": 3,
    "flags": 4,
    "timestamp": 5,
    "prev_comm": 6,
    "prev_pid": 7,
    "prev_prio": 8,
    "prev_state": 9,
    "next_comm": 10,
    "next_pid": 11,
    "next_prio": 12
}
COLUMNS = ["id", "effective_cpu_time", "total_cpu_time", "diff_cpu_time", "num_sched_switches", "num_migrations", "parameter", "job_number", "mode", "sched_policy", "sched_priority"]


def load_dataframe(df_path: str, columns: List[str] = None) -> pd.DataFrame:
    """
    Retrieves a pandas DataFrame from a file path. If the DataFrame does not exist, it creates a new one
    with the specified columns. If there isn't a column with the name 'id', it will add that cause this column is used
    in others functions of this module to perform some operations, specifically the 'id' column represent the execution
    identifier of a record in the DataFrame, and it must be an integer.


    Parameters:
        df_path (str): A string representing the file path to the pandas DataFrame.
        columns (List[str], optional): A list of string representing the columns of the pandas DataFrame. Default is None.

    Returns:
        pandas.DataFrame: The retrieved or newly created DataFrame.

    Note:
        If the 'columns' parameter is not provided, the function will use default columns defined in the module to create
        the DataFrame.
    """
    if os.path.exists(df_path):
        df = pd.read_csv(df_path)
    else:
        if columns is None:
            df = pd.DataFrame(columns=COLUMNS)
        else:
            if "id" not in columns:
                columns.insert(0, "id")
            df = pd.DataFrame(columns=columns)
    if "id" in df.columns:
        df["id"] = df["id"].astype(str)
    return df


def save_dataframe(df: pd.DataFrame, path: str, drop_na: bool = True, sort_by: List[str] = None, ascending: bool = True):
    """
    Saves a given pandas DataFrame to a specified file path after performing some preprocessing on the DataFrame.
    The preprocessing includes the following operations: sorting of the DataFrame entries based on the 'sort_by' parameter,
    and deleting records containing NaNs values.

    Parameters:
        df (pandas.DataFrame): The pandas DataFrame to save.
        path (str): The file path where the DataFrame will be saved.
        drop_na (bool): A boolean value used to specify if it is necessary to drop records containing NaNs from the DataFrame.
        sort_by (List[str], optional): A list of string used to sort values in the DataFrame before saving it to a csv file.
        ascending (bool, optional): A boolean value representing if the sorting should be performed in ascending order (True)
        or descending order (False).

    Returns:
        None
    """
    if drop_na:
        df.dropna(how="any", inplace=True)
    if sort_by is not None:
        df.sort_values(by=sort_by, ascending=ascending, inplace=True)
    if "id" in df.columns:
        df["id"] = df["id"].astype(str)
    if "num_sched_switches" in df.columns:
        df["num_sched_switches"] = df["num_sched_switches"].astype(int)
    if "num_migrations" in df.columns:
        df["num_migrations"] = df["num_migrations"].astype(int)
    df.to_csv(path, index=False)


def update_data(df: pd.DataFrame, dir_result_path: str, process_name: str, trace_filename: str = "trace.txt", execution_filename: str = "exec.txt", execution_data: List[str] = None,  analysis_function: Callable[[pd.DataFrame, str, TextIO, str], Any] = None):
    """
    Updates a given pandas DataFrame with new data records that haven't yet been recorded in the DataFrame, found in the
    directory specified by the 'dir_result_path' parameter. The hierarchy of the 'dir_result_path' directory must be of
    specific structure as defined below in the 'Parameters' section. A new record will be created in the DataFrame for
    each job found in this hierarchy of folders, if and only if the job is associated with an execution identifier which
    isn't already present in any records of the current DataFrame. If a DataFrame with default columns is used and the
    information contained within the 'execution_filename' file are the default one, then the new record will contain
    information about the job execution times and scheduling. Specifically in this case, the record information includes:
    program execution identifier, job actual execution time, job total execution time, job execution time difference between
    the actual and total execution time, job scheduling policy and priority, number of context switches obtained during
    the job execution, number of migrations obtained during the job execution, and an optional user-defined string. In
    the case where the values present within the ‘execution_filename’ file refer to other values not previously mentioned,
    it is assumed that the provided DataFrame is different from the default one. To indicate which column of the non-default
    DataFrame the values contained in the ‘execution_filename’ file refer to, use the ‘execution_data’ parameter as indicated
    below in the ‘Parameters’ section.

    Parameters:
        df (pandas.DataFrame): The DataFrame to update with the new data.
        dir_result_path (str): The path to the directory that contains a subfolders for each execution. In turn, each
        subfolder must contain the kernel trace file of the execution called as specified by the `trace_filename` parameter
        and a file with the jobs details of that execution called as specified by the `execution_filename` parameter.
        process_name (str): The name of the process within the kernel trace that we want to analyze.
        trace_filename (str, optional): The name of the kernel trace file. Default is "trace.txt".
        execution_filename (str, optional): The name of the file that contains all the job details related to the kernel
        trace file, line by line for each job that occurred during the execution. These job details are some values that
        will be stored in the corresponding columns of the new DataFrame records. Default is "exec.txt".
        execution_data (List[str], optional): A list of strings containing the names, in the corresponding order, of the
        columns in the DataFrame that refer to a line in the 'execution_filename' file. For example assume we have a DataFrame
        with the columns ["A", "B", "C", "D"] and the 'execution_filename' contains lines like this one <abc, xyz, 1>.
        Assume that the value 'abc' correspond to the C column of the DataFrame, the value '1' correspond to the A column
        and the value 'xyz' correspond to the D column. Therefore, in this case we need to pass to the 'execution_data'
        parameter the following list ["C", "A", "D"]. Default is None.
        analysis_function (Callable[[pd.DataFrame, str, TextIO, str], Any], optional): An aptional function used to implement
        a custom analysis. This function will be used in the analyze_trace() function of this module, and basically it will
        be called instead of performing the default analysis. To see more details about this parameter check the docs
        of the analyze_trace() function of this module.

    Returns:
        None

    Note:
        If no optional parameters are provided, the function will check the following two assumed facts:
            * the DataFrame provided contains exactly the default columns
            * the lines in the 'execution_filename' file are arranged in a certain type which is automatically recognizable
            by this function
        Therefore, this function will update the DataFrame with new data records regarding executions times and scheduling
        information of jobs whose start and end are marked on the kernel trace.
    """
    is_default_dataframe = all(column in df.columns for column in COLUMNS) and len(df.columns) == len(COLUMNS)
    if os.path.exists(dir_result_path):
        for dir_name in os.listdir(dir_result_path):
            if not df["id"].isin([dir_name]).any():
                try:
                    execution_file_path = os.path.join(dir_result_path, dir_name, execution_filename)
                    trace_file_path = os.path.join(dir_result_path, dir_name, trace_filename)
                    if os.path.exists(execution_file_path) and os.path.exists(trace_file_path):
                        with open(execution_file_path, "r") as execution_file:
                            if not is_default_dataframe and execution_data is not None:
                                for execution_line in execution_file:
                                    execution_data_values = [data.strip() for data in execution_line.split(",")]
                                    row = {}
                                    for index, column_name in enumerate(execution_data):
                                        row[column_name] = execution_data_values[index]
                                    if "id" not in execution_data:
                                        row["id"] = dir_name
                                    df.loc[len(df)] = row
                            elif not is_default_dataframe:
                                print(f"\nError: update_data error, you must provide the 'execution_data' parameter "
                                      f"since the DataFrame columns are not the default ones. Terminating.")
                                exit(1)
                            else:
                                for execution_line in execution_file:
                                    execution_data_values = [data.strip() for data in execution_line.split(",")]
                                    id = execution_data_values[0]
                                    job_number = int(execution_data_values[1])
                                    parameter = int(execution_data_values[2])
                                    sched_policy = execution_data_values[3]
                                    sched_priority = int(execution_data_values[4])
                                    details = execution_data_values[5]
                                    row = {"id": id, "parameter": parameter, "job_number": job_number, "mode": details, "sched_policy": sched_policy, "sched_priority": sched_priority}
                                    df.loc[len(df)] = row
                        with open(trace_file_path) as trace_file:
                            if is_default_dataframe:
                                analyze_trace(df=df, identifier=dir_name, trace_file=trace_file, process_name=process_name)
                            else:
                                if analysis_function is not None:
                                    analyze_trace(df=df, identifier=dir_name, trace_file=trace_file, process_name=process_name, analysis_function=analysis_function)
                                else:
                                    print(f"\nError: update_data error, you must provide the "
                                          f"'analyze_function' parameter since the DataFrame columns are not the "
                                          f"default ones. Terminating.")
                                    exit(1)
                except FileNotFoundError as e:
                    print(f"\nWarning: update_data warning, file \"{e.filename}\" not found. Skipping ...", end='')
                except PermissionError as e:
                    print(f"\nWarning: update_data warning, permission denied to open the following file \"{e.filename}\". Skipping ...", end='')
                except IOError as e:
                    print(f"\nWarning: update_data warning, io problem, {e.strerror}. Skipping ...", end='')
    else:
        print(f"\nError: update_data error, directory \"{dir_result_path}\" not found. Terminating.")
        exit(1)


def analyze_trace(df: pd.DataFrame, identifier: str, trace_file: TextIO, process_name: str, states: Tuple[str, str] = ("start", "end"), re_traceline_job: str = TRACE_JOB_LINE_PATTERN, re_traceline_line: str = TRACE_LINE_PATTERN, re_job_group_matcher: dict = None, re_line_group_matcher: dict = None, analysis_function: Callable[[pd.DataFrame, str, TextIO, str], Any] = None):
    """
    Calculates and updates job records of a given pandas DataFrame with execution times and scheduling information
    extracted from the kernel trace file associated to the program execution identified by the 'identifier' parameter.
    Therefore, the 'identifier' parameter is used to match and correctly update a job record in the DataFrame.
    Each job will be analyzed such that its start and end must are marked on the kernel trace using the trace_mark_job()
    function of the C library 'event_tracing.h'. The string which represent the start of a job in the kernel trace is
    specified by the first element of the 'states' parameter, default is 'start'. The string which represent the end of
    a job in the kernel trace is specified by the second element of the 'states' parameter. The kernel trace must be
    obtained using the NOP tracer of the Linux kernel tracing infrastructure. If the user is interested in obtaining data
    related with the default analysis of this function then during the tracing the sched_switch event must be enabled.
    In order to perform a different analysis, the user can set optional parameters to customize this function. This is
    explained below in the 'Parameters' section.

    Parameters:
        df (pandas.DataFrame): The DataFrame to update with the kernel trace information.
        identifier (str): The execution identifier used to match and update records in the DataFrame with execution time
        information. It must be the same as the name of the directory in which the 'trace_file' is contained.
        trace_file (TextIO): The kernel trace file to analyze.
        process_name (str): The name of the process to analyze within the kernel trace.
        states (Tuple[str, str], optional): A tuple indicating the strings used to mark the start and the end of a job
        in the kernel trace. Default is ("start", "end").
        re_traceline_job (str, optional): An optioanl regular expression used to match kernel trace lines which contains
        the reference to the start or the end of job that have to be recognized. Default is None.
        re_traceline_line (str, optional): An optional regular expression used to match kernel trace lines that have to
        be recognized. Default is None.
        re_job_group_matcher (dict, optional): An optional dictionary containing the matching groups for the `re_traceline_job`
        regular expression. It must contain at least the matching group of the following keys: "job_number", "job_state",
        "cpu_core", and "timestamp". Default is None.
        re_line_group_matcher (dict, optional): A optional dictionary containing the matching groups for the `re_traceline_line`
        regular expression. It must contain at least the matching group of the following keys: "prev_comm", "cpu_core",
        and "timestamp". Default is None.
        analysis_function (Callable[[pd.DataFrame, str, TextIO, str], Any], optional): An aptional function used to implement
        a custom analysis. This function will be called instead of performing the default analysis. This function must have
        the following parameters: a pandas.DataFrame, a string representing the execution identifier, a reference to the
        kernel trace file to analyze and a string parameter representing the process name to search in the kernel trace
        lines.

    Returns:
        None

    Note:
        If no optional parameters are provided, the function will perform a default analysis regarding executions times
        and scheduling information of jobs whose start and end are marked on the kernel trace, based on the sched_switch
        event. All of this using default regular expressions and matchers built-in the module.
    """
    if analysis_function is None:
        dc.getcontext().prec = 10
        if re_line_group_matcher is None:
            re_line_group_matcher = LINE_GROUP_MATCHER
        if re_job_group_matcher is None:
            re_job_group_matcher = JOB_GROUP_MATCHER
            re_traceline_job = re.sub(r'test_app', process_name, re_traceline_job)

        previous_timestamp: dc.Decimal = dc.Decimal("NaN")
        previous_core: str = ""
        migrations_count: int = 0
        sched_switches_count: int = 0
        effective_cpu_time: dc.Decimal = dc.Decimal('0.0')
        start_timestamp: dc.Decimal = dc.Decimal('0.0')
        bool_job_started: bool = False
        current_job_number: int = -1

        for line in trace_file:
            line.strip()
            match_job = re.search(re_traceline_job, line)
            match_line = re.search(re_traceline_line, line)

            if match_job:
                job_state = match_job.group(re_job_group_matcher["job_state"])
                job_number = int(match_job.group(re_job_group_matcher["job_number"]))
                if job_state == states[0]:
                    start_timestamp = previous_timestamp = dc.Decimal(match_job.group(re_job_group_matcher["timestamp"]))
                    previous_core = match_job.group(re_job_group_matcher["cpu_core"])
                    current_job_number = job_number
                    bool_job_started = True
                elif job_state == states[1] and job_number == current_job_number:
                    end_timestamp = dc.Decimal(match_job.group(re_job_group_matcher["timestamp"]))
                    total_cpu_time = end_timestamp - start_timestamp
                    effective_cpu_time = total_cpu_time if effective_cpu_time == dc.Decimal('0.0') else effective_cpu_time + (end_timestamp - previous_timestamp)
                    condition = (df["id"] == identifier) & (df["job_number"] == job_number)
                    new_values = {
                        "effective_cpu_time": float(effective_cpu_time.quantize(dc.Decimal('0.000001'))),
                        "total_cpu_time": float(total_cpu_time.quantize(dc.Decimal('0.000001'))),
                        "diff_cpu_time": float(total_cpu_time.quantize(dc.Decimal('0.000001')) - effective_cpu_time.quantize(dc.Decimal('0.000001'))),
                        "num_sched_switches": sched_switches_count,
                        "num_migrations": migrations_count
                    }
                    if condition.any():
                        df.loc[condition, new_values.keys()] = new_values.values()
                    else:
                        new_values["id"] = identifier
                        new_values["parameter"] = np.NaN
                        new_values["job_number"] = job_number
                        new_values["mode"] = np.NaN
                        new_values["sched_policy"] = np.NaN
                        new_values["sched_priority"] = np.NaN
                        df.loc[len(df)] = new_values
                    previous_timestamp = dc.Decimal("NaN")
                    previous_core = ""
                    effective_cpu_time = dc.Decimal('0.0')
                    migrations_count = 0
                    sched_switches_count = 0
                    start_timestamp = dc.Decimal('0.0')
                    bool_job_started = False
                    current_job_number = -1
                elif job_state == states[1] and job_number != current_job_number:
                    previous_timestamp = dc.Decimal("NaN")
                    previous_core = ""
                    effective_cpu_time = dc.Decimal('0.0')
                    migrations_count = 0
                    sched_switches_count = 0
                    start_timestamp = dc.Decimal('0.0')
                    bool_job_started = False
                    current_job_number = -1

            if match_line and bool_job_started:
                prev_comm = match_line.group(re_line_group_matcher["prev_comm"])
                current_cpu_core = match_line.group(re_line_group_matcher["cpu_core"])
                current_timestamp = dc.Decimal(match_line.group(re_line_group_matcher["timestamp"]))
                if prev_comm == process_name:
                    if previous_core != current_cpu_core:
                        migrations_count += 1
                        previous_core = current_cpu_core
                    sched_switches_count += 1
                    delta_time = current_timestamp - previous_timestamp
                    effective_cpu_time += delta_time
                    previous_timestamp = current_timestamp
                if prev_comm != process_name:
                    previous_timestamp = current_timestamp
    else:
        analysis_function(df, identifier, trace_file, process_name)


def heatmap_plot(df: pd.DataFrame, file_name: str = None, dir_path: str = None, title: str = None, color_palette: str = "Blues", to_save: bool = False):
    """
    Creates and optionally saves a heatmap plot with all the numeric variables in the DataFrame provided. A high value
    at the intersection of two variables means that there is a high correlation between them. A low value, in the other
    hand, indicates a low correlation between the two variables.

    Parameters:
        df (pandas.DataFrame): The DataFrame that contains data to be plotted.
        file_name (str, optional): The name of the file  with which the heatmap plot will be saved. Default is None.
        dir_path (str, optional): The path to a directory where to save the heatmap plot. If this parameter isn't
        provided the heatmap plot will be saved in a subfolder of the working directory called 'plots'. Default is None.
        title (str, optional): The title of the plot. Default is None.
        color_palette (str, optional): The value of the color palette to use in the heatmap plot. Default is "Blues".
        to_save (bool, optional): A boolean value used to determine whether to save the plot or not. If set to False,
        the plot will only be showed to the user. Default is False.

    Returns:
        None

    Note:
        If no optional parameters are provided, the function will use the default values defined internally in the
        module.
    """
    fig, ax = plt.subplots()
    corr_matrix = df.corr(numeric_only=True).abs()
    if "sched_priority" in df.columns:
        corr_matrix.drop('sched_priority', axis=0, inplace=True)
        corr_matrix.drop('sched_priority', axis=1, inplace=True)
    if "job_number" in df.columns:
        corr_matrix.drop('job_number', axis=0, inplace=True)
        corr_matrix.drop('job_number', axis=1, inplace=True)
    sns.heatmap(ax=ax, data=corr_matrix, annot=True, cmap=color_palette)
    if title is not None:
        plt.suptitle(title, fontsize=20)
    else:
        plt.suptitle("Heatmap of Correlations between variables", fontsize=20)
    save_show_plot(file_name, dir_path, to_save)


def join_plot(df: pd.DataFrame, x_var: str, y_var: str, file_name: str = None, dir_path: str = None, title: str = None, x_label: str = None, y_label: str = None, to_save: bool = False):
    """
    Creates and optionally saves a joinplot using the 'x_var' and 'y_var' present in the DataFrame specified by the 'df'
    parameter. A joinplot is a useful graph for seeing the trend of two variables and for visualizing how their
    correlation changes.

    Parameters:
        df (pandas.DataFrame): The DataFrame that contains data to be plotted.
        x_var (str): The name of the column in the DataFrame to use as the variable on the X-axis.
        y_var (str): The name of the column in the DataFrame to use as the variable on the Y-axis.
        file_name (str, optional): The name of the file with which the joinplot will be saved. Default is None.
        dir_path (str, optional): The path to a directory where to save the joinplot. If this parameter isn't provided
        the joinplot will be saved in a subfolder of the working directory called 'plots'. Default is None.
        title (str, optional): The title of the plot. Default is None.
        x_label (str, optional): The label to use along with the x-axis. Default is None.
        y_label (str, optional): The label to use along with the y-axis. Default is None.
        to_save (bool, optional): A boolean value used to determine whether to save the plot or not. If set to False,
        the plot will only be showed to the user. Default is False.

    Returns:
        None

    Note:
        If no optional parameters are provided, the function will use the default values defined internally in the
        module.
    """
    plot = sns.jointplot(x=x_var, y=y_var, data=df, kind="reg")
    xl = x_var.title() if x_label is None else x_label
    yl = y_var.title() if y_label is None else y_label
    plot.set_axis_labels(xl, yl, fontsize=12)
    plot.fig.subplots_adjust(top=0.9)
    if title is not None:
        plot.fig.suptitle(title)
    else:
        plot.fig.suptitle(f"Correlation between {x_var.title()} and {y_var.title()}", fontsize=12)
    save_show_plot(file_name, dir_path, to_save)


def distribution_plot(df: pd.DataFrame, var: str, file_name: str = None, dir_path: str = None, title: str = None, x_label: str = None, to_save: bool = False):
    """
    Creates and optionally saves a distribution plot of the 'x_var' present in the DataFrame specified by the 'df' parameter.
    The distribution plot is a graph that shows the distribution of a variable, which is useful for assuming some attributes
    of the variable such as the mean, median, variance and standard deviation.

    Parameters:
        df (pandas.DataFrame): The DataFrame that contains data to be plotted.
        var (str): The name of the column in the DataFrame to use as the variable for which to calculate the distribution.
        file_name (str, optional): The name of the file with which the distribution plot will be saved. Default is None.
        dir_path (str, optional): The path to a directory where to save the distribution plot. If this parameter isn't
        provided the distribution plot will be saved in a subfolder of the working directory called 'plots'. Default is None.
        title (str, optional): The title of the plot. Default is None.
        x_label (str, optional): The label to use along with the x-axis. Default is None.
        to_save (bool, optional): A boolean value used to determine whether to save the plot or not. If set to False,
        the plot will only be showed to the user. Default is False.

    Returns:
        None

    Note:
        If no optional parameters are provided, the function will use the default values defined internally in the
        module.
    """
    plot = sns.displot(df[var], kde=True, bins=20)
    plot.fig.subplots_adjust(top=0.9)
    if title is not None:
        plot.fig.suptitle(title)
    else:
        plot.fig.suptitle(f"Distribution of the {var.title()}")
    xl = var.title() if x_label is None else x_label
    plot.set_axis_labels(xl, fontsize=12)
    save_show_plot(file_name, dir_path, to_save)


def scatter_plot(df: pd.DataFrame, x_var: str, y_var: str, hue: str = None, hue_order: List[str] = None, style: str = None, file_name: str = None, dir_path: str = None, title: str = None, x_label: str = None, y_label: str = None, to_save: bool = False):
    """
    Creates and optionally saves a scatterplot the 'x_var' and 'y_var' present in the DataFrame specified by the 'df' parameter.
    This graph shows the trend of the 'x_var' correlated on 'y_var'. It is possible to distinguish the trends of the
    'x_var' based on some categorical value, and to define a different style of the trends based on another value specified
    by the 'style' parameter.

    Parameters:
        df (pandas.DataFrame): The DataFrame that contains data to be plotted.
        x_var (str): The name of the column in the DataFrame to use as the variable on the X-axis.
        y_var (str): The name of the column in the DataFrame to use as the variable on the Y-axis.
        hue (str, optional): The name of a categorical variable used to distinguish trends based on that variable. Default is None.
        hue_order (List[str], optional): The order of processing and plotting for categorical levels of the 'hue' semantic. Default is None
        style (str, optional): The name of a variable used to distinguish the style of the points on the graph. Default is None.
        file_name (str, optional): The name of the file with which the distribution plot will be saved. Default is None.
        dir_path (str, optional): The path to a directory where to save the distribution plot. If this parameter isn't
        provided the distribution plot will be saved in a subfolder of the working directory called 'plots'. Default is None.
        title (str, optional): The title of the plot. Default is None.
        x_label (str, optional): The label to use along with the x-axis. Default is None.
        y_label (str, optional): The label to use along with the y-axis. Default is None.
        to_save (bool, optional): A boolean value used to determine whether to save the plot or not. If set to False,
        the plot will only be showed to the user. Default is False.

    Returns:
        None

    Note:
        If no optional parameters are provided, the function will use the default values defined internally in the
        module.
    """
    plot = sns.relplot(data=df, x=x_var, y=y_var, hue=hue, hue_order=hue_order, style=style)
    plot.fig.subplots_adjust(top=0.9)
    if title is not None:
        plot.fig.suptitle(title)
    else:
        plot.fig.suptitle(f"Scatterplot of the {x_var.title()} and {y_var.title()}")
    xl = x_var.title() if x_label is None else x_label
    yl = y_var.title() if y_label is None else y_label
    plt.xlabel(xl)
    plt.ylabel(yl)
    save_show_plot(file_name, dir_path, to_save)


def grid_plots(df: pd.DataFrame, stats_function: Callable, x_var: str, y_var: str, col: str, row: str = None, hue: str = None, height: int = 5, aspect: float = 0.5, file_name: str = None, dir_path: str = None, title: str = None, to_save: bool = False):
    """
    Creates a FacetGrid which maps a dataset onto multiple axes arrayed in a grid of rows and columns
    that correspond to levels of variables in the dataset. The plots it produces are often called “lattice”,
    “trellis”, or “small-multiple” graphics. It can also represent levels of a third variable with the 'hue'
    parameter, which plots different subsets of data in different colors. This uses color to resolve elements on a
    third dimension, but only draws subsets on top of each other and will not tailor the hue parameter for the
    specific visualization the way that axes-level functions that accept hue will.

    Parameters:
        df (pandas.DataFrame): The DataFrame that contains data to be plotted.
        stats_function (Callable): A statistical function used to produce the plots
        x_var (str): The name of the column in the DataFrame to use as the variable on the X-axis.
        y_var (str): The name of the column in the DataFrame to use as the variable on the Y-axis.
        col (str): The name of the variable that define subsets of the data, which will be drawn on separate facets
        along the columns of the grid.
        row (str, optional): The name of the variable that define subsets of the data, which will be drawn on separate
        facets along the rows of the grid. Default is None.
        hue (str, optional): The name of a categorical variable used to distinguish trends based on that variable. Default is None.
        height (int, optional): The height (in inches) of each facet. Default is 5.
        aspect (float, optional): The aspect ratio of each facet. Default is 0.5.
        file_name (str, optional): The name of the file with which the distribution plot will be saved. Default is None.
        dir_path (str, optional): The path to a directory where to save the distribution plot. If this parameter isn't
        provided the distribution plot will be saved in a subfolder of the working directory called 'plots'. Default is None.
        title (str, optional): The title of the plot. Default is None.
        to_save (bool, optional): A boolean value used to determine whether to save the plot or not. If set to False,
        the plot will only be showed to the user. Default is False.

    Returns:
        None

    Note:
        If no optional parameters are provided, the function will use the default values defined internally in the
        module.
    """
    if row is not None:
        g = sns.FacetGrid(df, row=row, col=col, hue=hue, height=height, aspect=aspect)
    else:
        g = sns.FacetGrid(df, col=col, hue=hue, height=height, aspect=aspect)
    g.map(stats_function, x_var, y_var, alpha=0.5)
    g.add_legend()
    g.fig.subplots_adjust(top=0.9)
    if title is not None:
        g.fig.suptitle(title)
    else:
        g.fig.suptitle(f"Grid of {stats_function.__name__.title()} on {x_var.title()} and {y_var.title()}")
    save_show_plot(file_name, dir_path, to_save)


def save_show_plot(file_name: str, dir_path: str, to_save: bool):
    """
    Shows and optionally saves the last plot created using the Matplolib package.

    Parameters:
        dir_path (str, optional): The path where to save the plot.
        file_name (str, optional): The name of the file with which the plot will be saved.
        to_save (bool, optional): A boolean value used to determine if the plot should be saved in a file or not. If not
        the plot will be only showed to the user.

    Returns:
        None
    """
    if to_save:
        if dir_path is not None:
            if not os.path.exists(dir_path):
                os.makedirs(dir_path)
        else:
            dir_path = "plots"
            if not os.path.exists("plots"):
                os.makedirs("plots")
        if file_name is None:
            timestamp = datetime.now().strftime("%Y-%m-%d_%H-%M-%S_%f")
            file_name = "plot_" + timestamp
        file_path = os.path.join(dir_path, file_name)
        plt.savefig(file_path)
    else:
        plt.show()


def set_sns_config(dpi: int = 200, figsize: Tuple[int, int] = (8, 6), font_scale: float = 1.0):
    """
    Sets some options values used in the Seaborn package regarding the plot creation. More specifically it allows to set
    the dpi, the font size and the size of the figure of a plot.

    Parameters:
        dpi (int, optional): The dpi of the figure used to display a plot. Default is 200.
        figsize (Tuple[int,int]): A tuple that specify the width and the height of a figure used to display a plot. Default is (8, 6).
        font_scale (float): The size of text within the plot. Default is 1.0.

    Returns:
        None

    Note:
        If no optional parameters are provided, the function will use the default values defined internally in the
        module.
    """
    sns.set_theme()
    sns.set(rc={"figure.dpi": dpi})
    sns.set(rc={"figure.figsize": figsize})
    sns.set_context("paper", font_scale=font_scale)
