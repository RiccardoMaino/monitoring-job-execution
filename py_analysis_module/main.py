import argparse
import os
import event_tracing_module as etm


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
                        required=False)
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
        os.remove(args.csvpath)

    if args.delplots and os.path.isdir(args.plotspath):
        os.rmdir(args.plotspath)

    df = etm.load_dataframe(args.csvpath)
    etm.update_data(df=df, dir_result_path=args.respath, process_name=args.name)

    # print(df)
    # etd.save_dataframe(df, df_path, sort_by=["mode", "parameter", "job_number"])

    # etd.set_sns_config(100, (15, 10), 1.2)
    # etd.heatmap_plot(df=df, file_name="heatmap_empty.png", to_save=True)
    # etd.join_plot(df=df, x_var="parameter", y_var="num_sched_switches", file_name="joinplot_empty.png", to_save=True)


if __name__ == "__main__":
    main()
