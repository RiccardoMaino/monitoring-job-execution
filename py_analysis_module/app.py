import argparse
import os
import shutil
import tracing_analysis_module as tam


class CustomFormatter(argparse.HelpFormatter):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs, max_help_position=80, width=80)


def main():
    parser = argparse.ArgumentParser(description="Dataset and Plots Maker 1.0: this program is used to create or update"
                                                 " a dataset that contains records regarding the execution details of "
                                                 "jobs. It also allows you to create plots useful for analyzing the "
                                                 "data contained in the dataset.",
                                     formatter_class=CustomFormatter)
    parser.add_argument("-r", "--respath",
                        type=str,
                        default="../event_tracing_library/results",
                        help="a string used to set the path where to find the information used to create the dataset "
                             "containing records regarding job execution details. (default: "
                             "../event_tracing_library/results)",
                        required=False)
    parser.add_argument("-c", "--csvpath",
                        type=str,
                        default="dataset.csv",
                        help="a string used to set the path to a file where to store (or to find) the newly (or the "
                             "previously) created dataset. (default: dataset.csv)",
                        required=False)
    parser.add_argument("-p", "--plotspath",
                        type=str,
                        default="plots",
                        help="a string used to set the path where to store plots regarding the dataset data. ("
                             "default: plots)",
                        required=False)
    parser.add_argument("-n", "--name",
                        type=str,
                        default="test_app",
                        help="a string used to specify the process name of the program traced using the event_tracing "
                             "C library. (default: test_app)",
                        required=False),
    parser.add_argument("-l", "--makeplots",
                        action="store_true",
                        help="a flag which if specified allows to create useful plots for the analysis of records "
                             "contained in the dataset created (or updated). (default: False)")
    parser.add_argument("-d", "--delcsv",
                        action="store_true",
                        help="a flag which if specified allows to delete the dataset previously created. (default: "
                             "False)",
                        required=False)
    parser.add_argument("-k", "--delplots",
                        action="store_true",
                        help="a flag which if specified allows to delete the plots, related to the dataset, "
                             "previously created. (default: False)",
                        required=False)
    args = parser.parse_args()

    if args.delcsv and os.path.isfile(args.csvpath):
        print("*** Deleting the dataset previously created ...")
        os.remove(args.csvpath)

    if args.delplots and os.path.isdir(args.plotspath):
        print("*** Deleting the plots previously created ...")
        shutil.rmtree(args.plotspath, ignore_errors=True)

    print("*** Loading the dataset ...", end='')
    df = tam.load_dataframe(args.csvpath)
    print("DONE")
    print("*** Updating the dataset ...", end='')
    tam.update_data(df=df, dir_result_path=args.respath, process_name=args.name)
    print("DONE")
    print("*** Saving the dataset ...", end='')
    tam.save_dataframe(df, args.csvpath, sort_by=["mode", "parameter", "job_number"])
    print("DONE")

    if args.makeplots:
        print("*** Creating plots for analysis ...", end='')
        tam.set_sns_config(100, (15, 10), 1.2)

        df["effective_cpu_time"] = df["effective_cpu_time"] * 1000
        df["total_cpu_time"] = df["total_cpu_time"] * 1000
        df["diff_cpu_time"] = df["diff_cpu_time"] * 1000

        df_empty = df[df["mode"] == "EmptyLoop"]
        df_variable = df[df["mode"] == "VariablesExchanging"]
        df_list = df[df["mode"] == "ListOrdering"]

        if len(df_empty) >= 20:
            tam.heatmap_plot(df=df_empty, file_name="hmap_emptyjob.png", dir_path=args.plotspath,
                             title="Correlation between variables - Empty Loop Job",
                             to_save=True)
            tam.join_plot(df=df_empty, x_var="parameter", y_var="num_sched_switches", dir_path=args.plotspath,
                          title="Correlation between Parameter and number of Context Switches - Empty Loop Job",
                          file_name="join_parameter_schedswitches_emptyjob.png", to_save=True)
            tam.join_plot(df=df_empty, x_var="parameter", y_var="num_migrations", dir_path=args.plotspath,
                          title="Correlation between Parameter and number of Migrations - Empty Loop Job",
                          file_name="join_parameter_migrations_emptyjob.png", to_save=True)
            tam.join_plot(df=df_empty, x_var="parameter", y_var="effective_cpu_time", dir_path=args.plotspath,
                          title="Correlation between Parameter and Effective CPU Time - Empty Loop Job",
                          file_name="join_parameter_ect_emptyjob.png", to_save=True)
        else:
            print("\n\tinsufficient number of records for Empty Loop Job, skipping ...")

        if len(df_variable) >= 20:
            tam.heatmap_plot(df=df_variable, file_name="hmap_variablejob.png", dir_path=args.plotspath,
                             title="Correlation between variables - Variables Exchanging Job",
                             to_save=True)
            tam.join_plot(df=df_variable, x_var="parameter", y_var="num_sched_switches", dir_path=args.plotspath,
                          title="Correlation between Parameter and number of Context Switches - Variable Exchanging Job",
                          file_name="join_parameter_schedswitches_variablejob.png", to_save=True)
            tam.join_plot(df=df_variable, x_var="parameter", y_var="num_migrations", dir_path=args.plotspath,
                          title="Correlation between Parameter and number of Migrations - Variables Exchaging Job",
                          file_name="join_parameter_migrations_variablejob.png", to_save=True)
            tam.join_plot(df=df_variable, x_var="parameter", y_var="effective_cpu_time", dir_path=args.plotspath,
                          title="Correlation between Parameter and Effective CPU Time - Variables Exchanging Job",
                          file_name="join_parameter_ect_variablejob.png", to_save=True)
        else:
            print("\n\tinsufficient number of records for Variable Exchanging Job, skipping ...")

        if len(df_list) >= 20:
            tam.heatmap_plot(df=df_list, file_name="hmap_listjob.png", dir_path=args.plotspath,
                             title="Correlation between variables - List Ordering Job",
                             to_save=True)
            tam.join_plot(df=df_list, x_var="parameter", y_var="num_sched_switches", dir_path=args.plotspath,
                          title="Correlation between Parameter and number of Context Switches - List Ordering Job",
                          file_name="join_parameter_schedswitches_listjob.png", to_save=True)
            tam.join_plot(df=df_list, x_var="parameter", y_var="num_migrations", dir_path=args.plotspath,
                          title="Correlation between Parameter and number of Migrations - List Ordering Job",
                          file_name="join_parameter_migrations_listjob.png", to_save=True)
            tam.join_plot(df=df_list, x_var="parameter", y_var="effective_cpu_time", dir_path=args.plotspath,
                          title="Correlation between Parameter and Effective CPU Time - List Ordering Job",
                          file_name="join_parameter_ect_listjob.png", to_save=True)
        else:
            print("\n\tinsufficient number of records for List Ordering Job, skipping ...")
        print("DONE")
    print("*** All done. Terminating.")


if __name__ == "__main__":
    main()
