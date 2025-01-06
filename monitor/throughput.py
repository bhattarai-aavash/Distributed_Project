import matplotlib.pyplot as plt
import datetime

# Function to parse the log file and extract throughput data
def parse_log_file(log_file):
    timestamps = []
    read_throughput = []
    write_throughput = []

    with open(log_file, 'r') as file:
        for line in file:
            if "New Reads" in line and "New Writes" in line:
                # Example log line format:
                # 2024-12-19 12:41:06,185 - INFO - New Reads: 0, New Writes: 0
                parts = line.split(" - INFO - New Reads: ")
                timestamp_str = parts[0]
                reads_writes = parts[1].split(", New Writes: ")
                
                # Extract timestamp and reads/writes count
                timestamp = datetime.datetime.strptime(timestamp_str, "%Y-%m-%d %H:%M:%S,%f")
                reads = int(reads_writes[0])
                writes = int(reads_writes[1])

                # Append data
                timestamps.append(timestamp)
                read_throughput.append(reads)
                write_throughput.append(writes)
    
    return timestamps, read_throughput, write_throughput

# Function to plot throughput over time
def plot_throughput(timestamps, read_throughput, write_throughput, save_path=None):
    plt.figure(figsize=(10, 6))

    # Plot reads and writes
    plt.plot(timestamps, read_throughput, label="Reads", color='blue')
    plt.plot(timestamps, write_throughput, label="Writes", color='red')

    # Format the plot
    plt.xlabel("Timestamp")
    plt.ylabel("Throughput (operations)")
    plt.title("Redis Read and Write Throughput Over Time")
    plt.xticks(rotation=45)
    plt.tight_layout()
    plt.legend()

    # Show the plot
    if save_path:
        # Save the plot as an image file
        plt.savefig(save_path)
        print(f"Plot saved as {save_path}")
    else:
        # Display the plot
        plt.show()

if __name__ == "__main__":
    log_file = 'redis_read_write_monitor.log'  # Path to the log file
    save_path = 'redis_throughput_plot.png'  # Path to save the plot

    # Parse the log file
    timestamps, read_throughput, write_throughput = parse_log_file(log_file)

    # Plot the throughput data and save it as an image
    plot_throughput(timestamps, read_throughput, write_throughput, save_path)
