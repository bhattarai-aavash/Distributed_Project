import re
import matplotlib.pyplot as plt
import datetime
from collections import defaultdict

# Log file name
log_file = "/home/abhattar/Desktop/project_fall_sem/monitor/yangra6.log"
output_file = "yangra6_test.png"  # Output image file

# Updated regex pattern (same as before)
pattern = re.compile(r"(\d{1,2} \w{3} \d{4} \d{2}:\d{2}:\d{2})\.?\d* - .*throughput: ([\d.]+)")

# Dictionary to store throughput values aggregated per second
throughput_per_second = defaultdict(list)

# Specify time range (set to None to disable filtering)
start_time = ""  # Change as needed
end_time = ""    # Change as needed

# Convert time range to datetime objects
start_time_obj = datetime.datetime.strptime(start_time, "%d %b %Y %H:%M:%S") if start_time else None
end_time_obj = datetime.datetime.strptime(end_time, "%d %b %Y %H:%M:%S") if end_time else None

# Read the log file and extract data
with open(log_file, "r") as file:
    for line in file:
        match = pattern.search(line)
        if match:
            time_str = match.group(1)  # Extract timestamp (without milliseconds)
            throughput = float(match.group(2))
            
            # Convert time string to datetime object
            time_obj = datetime.datetime.strptime(time_str, "%d %b %Y %H:%M:%S")

            # Filter based on time range
            if (start_time_obj and time_obj < start_time_obj) or (end_time_obj and time_obj > end_time_obj):
                continue

            # Store throughput values for each second
            throughput_per_second[time_obj].append(throughput)

# Aggregate throughput per second (average if multiple entries exist)
timestamps = []
throughput_values = []

for time_obj in sorted(throughput_per_second.keys()):
    avg_throughput = sum(throughput_per_second[time_obj]) / len(throughput_per_second[time_obj])
    timestamps.append(time_obj)
    throughput_values.append(avg_throughput)

# Ignore the initial spike (Adjustable threshold)
if len(throughput_values) > 5:
    stable_threshold = sum(throughput_values[5:]) / len(throughput_values[5:])  # Compute avg after first 5 points
    filtered_data = [(t, v) for t, v in zip(timestamps, throughput_values) if v <= stable_threshold * 1.5]
    
    timestamps, throughput_values = zip(*filtered_data) if filtered_data else ([], [])

# Create the plot
plt.figure(figsize=(10, 5))
plt.plot(timestamps, throughput_values, marker='o', linestyle='-', color='b', label="Throughput (requests/sec)")

plt.xlabel("Time")
plt.ylabel("Throughput (requests/sec)")
plt.title("KeyDB Throughput Over Time (Filtered)")
plt.xticks(rotation=45)
plt.legend()
plt.grid(True)

# Save the plot to a file
plt.savefig(output_file, bbox_inches='tight', dpi=300)

print(f"Filtered plot saved as {output_file}")
