import sys
from pandas import read_csv, DataFrame
import matplotlib.pyplot as plt

# set font of all elements to size 15
plt.rc("font", size=15)

# Plot the times in log-log scale
def plot_points(df, label):
    ax = df.plot.line(marker="o", figsize=(20, 10))
    ax.set_ylabel(label)
    ax.set_xticks(df.index)
    return ax.figure


def main(filename, plots_directory):
    # Reads the CSV file
    df = read_csv(filename, index_col=["nw"])
    # Takes the sequential time, then drops the row
    t_seq = df["Ideal"].iloc[0]
    df.drop(labels=0, inplace=True)
    # Plots the latency
    plot_points(df, "Latency (ms)").savefig(f"{plots_directory}/latency.png")
    # Plots the speedup
    speedup = t_seq / df
    plot_points(speedup, "Speedup").savefig(f"{plots_directory}/speedup.png")
    # Plots the efficiency
    efficiency = DataFrame()
    for column in df.columns:
        efficiency[column] = speedup[column] / df.index
    plot_points(efficiency, "Efficiency").savefig(f"{plots_directory}/efficiency.png")
    # Plots the scalability
    scalability = DataFrame(index=df.index)
    for column in df.columns:
        scalability[column] = df[column].loc[1] / df[column]
    plot_points(scalability, "Scalability").savefig(
        f"{plots_directory}/scalability.png"
    )


if __name__ == "__main__":
    # Filename of the input
    filename = sys.argv[1]
    # Directory to save the plots
    plots_directory = sys.argv[2]
    main(filename, plots_directory)
