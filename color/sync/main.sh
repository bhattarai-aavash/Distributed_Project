#!/bin/bash

# Check if the correct number of arguments is provided
if [ $# -ne 3 ]; then
    echo "Usage: $0 <first_node_id_of_first_task> <last_node_id_of_last_task> <ip_address> <log_file_path>"
    exit 1

fi

# Capture the start time
start_time=$(date +%s)

# Get the input arguments
first_node_id_of_first_task=$1
last_node_id_of_last_task=$2
ip_address=$3

# Get the hostname of the system
hostname=$(hostname)

# Log files based on the hostname
log_file="${hostname}.log"
color_log_file="${hostname}_color.log"

# Compile the color.c program
# gcc -o color color.c -lhiredis

# Run the color program with the provided arguments
./color $first_node_id_of_first_task $last_node_id_of_last_task $ip_address $color_log_file $hostname

# Capture the end time
end_time=$(date +%s)

# Calculate the elapsed time
elapsed_time=$((end_time - start_time))

# Log the execution time to the log file
echo "Execution time: $elapsed_time seconds" >> "$log_file"

# Print a message about where the log file is saved
echo "Log saved to $log_file"
