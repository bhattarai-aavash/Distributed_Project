import re
import matplotlib.pyplot as plt
import datetime
from collections import defaultdict
import os

# Server log files
server_log_files = [
    "/home/abhattar/Desktop/results/sync/3server20clients/yangra4.log",
    "/home/abhattar/Desktop/results/sync/3server20clients/yangra5.log",
    "/home/abhattar/Desktop/results/sync/3server20clients/yangra6.log"
]

# Client log directory
client_log_dir = "/home/abhattar/Desktop/results/sync/3server20clients/client_throughput"  # Update this path
output_file = "throughput_comparison.png"

# Time range (optional)
start_time_str = ""  # Example: "19 Mar 2025 12:10:00"
end_time_str = ""    # Example: "19 Mar 2025 12:50:00"

# Regex for server logs
server_pattern = re.compile(r"(\d{1,2} \w{3} \d{4} \d{2}:\d{2}:\d{2})\.?\d* - .*throughput: ([\d.]+)")

# Regex for client logs
client_pattern = re.compile(r"(Read|Write) command at (\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})")

plt.figure(figsize=(12, 6))

# Convert time strings to datetime objects if provided
start_time = datetime.datetime.strptime(start_time_str, "%d %b %Y %H:%M:%S") if start_time_str else None
end_time = datetime.datetime.strptime(end_time_str, "%d %b %Y %H:%M:%S") if end_time_str else None

# # Process server logs
# for log_file in server_log_files:
#     throughput_per_second = defaultdict(list)
    
#     with open(log_file, "r") as file:
#         for line in file:
#             match = server_pattern.search(line)
#             if match:
#                 time_str = match.group(1)
#                 throughput = float(match.group(2))
#                 time_obj = datetime.datetime.strptime(time_str, "%d %b %Y %H:%M:%S")
                
#                 # Apply time filtering if specified
#                 if start_time and end_time and not (start_time <= time_obj <= end_time):
#                     continue
                
#                 throughput_per_second[time_obj].append(throughput)

#     # Aggregate data
#     time_objs = sorted(throughput_per_second.keys())
#     if not time_objs:
#         print(f"No data found in {log_file}. Skipping...")
#         continue

#     base_time = time_objs[0]
#     elapsed_times = [(t - base_time).total_seconds() for t in time_objs]
#     avg_throughput_values = [sum(throughput_per_second[t]) / len(throughput_per_second[t]) for t in time_objs]

#     server_name = os.path.basename(log_file).replace(".log", "")
#     plt.plot(elapsed_times, avg_throughput_values, linestyle='-', label=f"Server {server_name}")

# Process client logs
client_logs = [f for f in os.listdir(client_log_dir) if f.endswith(".txt")]  # Adjust file extension if needed

for client_log in client_logs:
    client_log_path = os.path.join(client_log_dir, client_log)
    client_throughput = defaultdict(int)
    
    with open(client_log_path, "r") as file:
        for line in file:
            match = client_pattern.search(line)
            if match:
                time_str = match.group(2)
                time_obj = datetime.datetime.strptime(time_str, "%Y-%m-%d %H:%M:%S")
                
                if start_time and end_time and not (start_time <= time_obj <= end_time):
                    continue
                
                client_throughput[time_obj] += 1  # Count the number of commands per second

    # Convert client throughput data to elapsed time format
    if client_throughput:
        time_objs = sorted(client_throughput.keys())
        base_time = time_objs[0]
        elapsed_times = [(t - base_time).total_seconds() for t in time_objs]
        throughput_values = [client_throughput[t] for t in time_objs]

        plt.plot(elapsed_times, throughput_values, linestyle='--', label=f"Client {client_log.replace('.txt', '')}")

# Configure plot
plt.xlabel("Elapsed Time (seconds)")
plt.ylabel("Throughput (requests/sec)")
plt.title("Server vs. Client Throughput Over Time")
plt.grid(True)
plt.legend()
plt.savefig(output_file
