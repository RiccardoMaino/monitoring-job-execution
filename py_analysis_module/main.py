import event_tracing_module as etm


def main():
    df_path = "dataset.csv"
    dir_result = "/home/riccardo/Desktop/Stage/demo/event_tracing_library/results"

    df = etm.load_dataframe(df_path)
    etm.update_data(df=df, dir_result_path=dir_result, process_name="test_app")

    print(df)
    # etd.save_dataframe(df, df_path, sort_by=["mode", "parameter", "job_number"])

    # etd.set_sns_config(100, (15, 10), 1.2)
    # etd.heatmap_plot(df=df, file_name="heatmap_empty.png", to_save=True)
    # etd.join_plot(df=df, x_var="parameter", y_var="num_sched_switches", file_name="joinplot_empty.png", to_save=True)


if __name__ == "__main__":
    main()
