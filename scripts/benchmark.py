import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np
import pandas as pd
import seaborn as sns
import math
import ipdb

def process(results):
    database = []
    for dataset in results.keys():
        if len(results[dataset]) > 0:
            for scheme in results[dataset].keys():
                entry = {
                    'dataset': results[dataset][scheme]["dataset"],
                    'name': results[dataset][scheme]["name"],
                    'scheme': results[dataset][scheme]["scheme"],
                    'num_procs': results[dataset][scheme]["num_procs"],
                    'num_runs': results[dataset][scheme]["num_runs"],
                    'nr_states': results[dataset][scheme]["nr_states"],
                    'nr_actions': results[dataset][scheme]["nr_actions"],
                    'frequency': results[dataset][scheme]["frequency"],
                    'mean_time': results[dataset][scheme]["mean_time"],
                    'comm_mean_time': results[dataset][scheme]["comm_mean_time"],
                    'min_time': results[dataset][scheme]["min_time"],
                    'var_time': results[dataset][scheme]["var_time"],
                    'weighted_mean_execution_times': results[dataset][scheme]["weighted_mean_execution_times"]
                }
                database.append(entry)


    df = pd.DataFrame(database)
    df_debug = df[df.dataset == "debug"]
    df_small = df[df.dataset == "small"]
    df_normal = df[df.dataset == "normal"]

    df["mean_time_dataset"] = 0.0

    if len(df_debug) > 0:
        for p in df_debug["num_procs"].unique():
            mean_debug = df_debug[df_debug.num_procs == p]["mean_time"].mean()
            df.loc[((df.dataset == "debug") & (df.num_procs == p)),'mean_time_dataset'] = mean_debug

    if len(df_small) > 0:
        for p in df_small["num_procs"].unique():
            mean_small = df_small[df_small.num_procs == p]["mean_time"].mean()
            df.loc[((df.dataset == "small") & (df.num_procs == p)),'mean_time_dataset'] = mean_small

    if len(df_normal) > 0:
        for p in df_normal["num_procs"].unique():
            mean_normal = df_normal[df_normal.num_procs == p]["mean_time"].mean()
            df.loc[((df.dataset == "normal") & (df.num_procs == p)),'mean_time_dataset'] = mean_normal

    return df

def increment(results):
    for dataset in results.keys():
        if len(results[dataset]) > 0:
            for scheme in results[dataset].keys():
                results[dataset][scheme]["mean_time"] = np.mean(results[dataset][scheme]["execution_times"])
                results[dataset][scheme]["comm_mean_time"] = np.mean(results[dataset][scheme]["comm_times"])
                results[dataset][scheme]['log_times'] = np.log2(results[dataset][scheme]['execution_times'])
                results[dataset][scheme]['min_time'] = np.min(results[dataset][scheme]['execution_times'])
                results[dataset][scheme]['var_time'] = np.var(results[dataset][scheme]['execution_times'])
                results[dataset][scheme]['nr_states'] = np.min(results[dataset][scheme]['nr_states'])
                results[dataset][scheme]['nr_actions'] = np.min(results[dataset][scheme]['nr_actions'])
                results[dataset][scheme]['weighted_mean_execution_times'] = np.mean(results[dataset][scheme]['weighted_mean_execution_times'])
                

def plots(df):
    def line(df, x="", y="", hue="", title="", xlabel="", ylabel="", key=""):
        if len(df) == 0:
            return
        plt.figure(figsize=(7, 6))
        g = sns.lineplot(x=x, y=y, hue=hue, data=df)
        g.set_xticks(df[x].unique())

        plt.title(f"{title}")
        plt.xlabel(f'{xlabel}')
        plt.ylabel(f'{ylabel}')

        plt.savefig(f".imgs/line_{key}.png")
        plt.close()

    def bar(df, x="", y="", hue="", title="", xlabel="", ylabel="", key=""):
        if len(df) == 0:
            return
        plt.figure()
        g = sns.catplot(x=x, y=y, hue=hue, data=df, kind="bar", ci=None, legend_out=True)
        g.fig.set_size_inches(16, 8)
        g.fig.subplots_adjust(top=0.81, right=0.86)
        ax = g.facet_axis(0, 0)
        for p in ax.patches:
             ax.text(p.get_x() - 0.01,
                     p.get_height() * 1.02,
                     '{0:.3f}'.format(p.get_height()),
                     color='black',
                     rotation='horizontal',
                     size='medium')

        plt.title(f"{title}")
        plt.xlabel(f'{xlabel}')
        plt.ylabel(f'{ylabel}')

        plt.grid()
        plt.savefig(f".imgs/bar_{key}.png")
        plt.close()
    
    def pie_charts(df):
        if len(df) == 0:
            return
        unique_names = df["name"].unique()

        procs = sorted(df['num_procs'].unique())

        freqs = sorted(df['frequency'].unique())

        schemes = df['scheme'].unique()

        num_plots = len(unique_names)

        important_columns = ['dataset', 
                            'name',
                            'scheme',
                            'mean_time',
                            'comm_mean_time',
                            'frequency',
                            'num_procs']
        
        df = df[important_columns]

        df['total_time'] = df['mean_time'] + df['comm_mean_time']

        df['perc_mean_time'] = df['mean_time'] / df['total_time']
        df['perc_comm_time'] = df['comm_mean_time'] / df['total_time']

        df_slice = df.copy()

        for scheme in schemes:
            fig, axs = plt.subplots(len(procs), len(freqs), figsize=(16,16))


            df_scheme = df_slice[ df['scheme'] == scheme]

            for ind_freq in range(len(freqs)):

                freq = freqs[ind_freq]

                df_freq = df_scheme[df['frequency'] == freq]

                for ind_proc in range(len(procs)):

                    proc = procs[ind_proc]

                    df_s_f_p = df_freq[df['num_procs'] == proc]
                    df_name = df_s_f_p.copy()

                    dataset = list(df_s_f_p['dataset'])[0]

                    labels = ['mean_time', 'mean_comm_time']

                    perc_means = list(df_s_f_p['perc_mean_time'])[0]

                    perc_comms = list(df_s_f_p['perc_comm_time'])[0]

                    percentages = [perc_means, perc_comms]

                    axs[ind_proc, ind_freq].pie(percentages, labels=labels, autopct='%.0f%%', shadow=True, startangle=90)

                    axs[ind_proc, ind_freq].legend(bbox_to_anchor=(0.9, 0.1))

                    axs[ind_proc, ind_freq].set_title(f"{scheme.replace('ValueIterator','')} f:{freq} p:{proc}")    

            img_path = f".imgs/pies_{dataset}_{scheme}.png"

            fig.savefig(img_path)
            

    def stacked(df, x="", y1="", y2="", title="", xlabel="", ylabel="", key=""):
        if len(df) == 0:
            return
        
        plt.figure()

        g1 = sns.barplot(x=x, y=y1, data=df, color="blue")
        g2 = sns.barplot(x=x, y=y2, data=df, color="red")

        plt.title(f"{title}")
        plt.xlabel(f'{xlabel}')
        plt.ylabel(f'{ylabel}')

        plt.grid()
        plt.savefig(f".imgs/stacked_{key}.png")
        plt.close()

    df_debug = df[df.dataset == "debug"]
    df_small = df[df.dataset == "small"]
    df_normal = df[df.dataset == "normal"]

    for dataframe in (df_debug, df_small, df_normal):
        if len(dataframe) == 0:
            continue

        dataset = dataframe["dataset"].unique()[0]
        dataset_title = dataset.capitalize()
        for p in dataframe["num_procs"].unique():
            bar(dataframe[dataframe.num_procs == p], 
                x="frequency", y="mean_time", hue="scheme", 
                title=f"Mean Time - {p} Processors - {dataset_title} Dataset", 
                xlabel="Frequency (in epochs)", 
                ylabel="Mean Execution Time (in seconds)", 
                key=f"{dataset}_num_procs_{p}")

            line(dataframe, 
                 x="frequency", y="mean_time", hue="scheme", 
                 title=f"Mean Time - {p} Processors - {dataset_title} Dataset", 
                 xlabel="Frequency (in epochs)", 
                 ylabel="Mean Execution Time (in seconds)", 
                 key=f"{dataset}_num_procs_{p}")

            # Visualization done to be able to forecast the execution times for larger nr_states and nr_actions
            line(dataframe, 
                 x="frequency", y="weighted_mean_execution_times", hue="scheme", 
                 title=f"Weighted Mean Time - {p} Processors - {dataset_title} Dataset", 
                 xlabel="Frequency (in epochs)", 
                 ylabel="Weighted Mean Execution Time (in seconds)", 
                 key=f"{dataset}_weighted_num_procs_{p}")
            
            pie_charts(dataframe)

            for s in dataframe["scheme"].unique():
                stacked(dataframe[((dataframe.scheme == s) & (dataframe.num_procs == p))], 
                        x="frequency", y1="mean_time", y2="comm_mean_time", 
                        title=f"{s} - {p} Processors - {dataset_title} Dataset", 
                        xlabel="Frequency (in epochs)", 
                        ylabel="Mean Execution Time (in seconds)", 
                        key=f"{dataset}_{s}_num_procs_{p}")


        for f in dataframe["frequency"].unique():
            bar(dataframe[dataframe.frequency == f], 
                x="num_procs", y="mean_time", hue="scheme", 
                title=f"Mean Time - Frequency {f} - {dataset_title} Dataset", 
                xlabel="Number of Processors", 
                ylabel="Mean Execution Time (in seconds)", 
                key=f"{dataset}_frequency_{f}")

            line(dataframe, 
                 x="num_procs", y="mean_time", hue="scheme", 
                 title=f"Mean Time - Frequency {f} - {dataset_title} Dataset", 
                 xlabel="Number of Processors", 
                 ylabel="Mean Execution Time (in seconds)", 
                 key=f"{dataset}_frequency_{f}")

            # Visualization done to be able to forecast the execution times for larger nr_states and nr_actions
            line(dataframe, 
                 x="frequency", y="weighted_mean_execution_times", hue="scheme", 
                 title=f"Weighted Mean Time - Frequency {f} - {dataset_title} Dataset", 
                 xlabel="Frequency (in epochs)", 
                 ylabel="Weighted Mean Execution Time (in seconds)", 
                 key=f"{dataset}_weighted_frequency_{f}")

            for s in dataframe["scheme"].unique():
                stacked(dataframe[((dataframe.scheme == s) & (dataframe.frequency == f))], 
                        x="num_procs", y1="mean_time", y2="comm_mean_time", 
                        title=f"{s} - {f} Frequency - {dataset_title} Dataset", 
                        xlabel="Number of Processors", 
                        ylabel="Mean Execution Time (in seconds)", 
                        key=f"{dataset}_{s}_frequency_{f}")

    line(df, 
         x="num_procs", y="mean_time_dataset", hue="dataset", 
         title="Mean Execution Time - All Datasets", 
         xlabel="Number of Processors", 
         ylabel="Mean Execution Time (in seconds)", 
         key="datasets_mean")


def profile_results(results):
    increment(results)
    df = process(results)

    plots(df)
