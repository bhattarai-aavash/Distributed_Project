#!/bin/bash
start_time=$(date +%s)
# Get the hostname
hostname=$(hostname)

# Define the log file paths
color_log_file_1="c1.log"
color_log_file_2="c2.log"

# Remove previous log files if they exist
rm -f $color_log_file_1
rm -f $color_log_file_2

if ! nc -zv localhost 6379; then
  echo "KeyDB server is down, exiting..."
  exit 1
fi




# Clear the KeyDB database
../../KeyDB/src/keydb-cli FLUSHALL

# Add the graph to KeyDB
../../KeyDB/src/keydb-cli add_graph /home/abhattar/Desktop/project_fall_sem/graph/edge_graph_dblp_coauthorship_n317080.txt



# # Activate the Python virtual environment
# source ../../monitor/monitor/bin/activate

# # Start the monitor script in the background and save its PID
# python3 /home/abhattar/Desktop/project_fall_sem/monitor/monitor.py --servers server1:localhost:6379 --interval 1 &
# monitor_pid=$!
# # Compile the color program

# sleep 2 


gcc -o color color.c -lhiredis

# Run the color commands in parallel
./color 0 158540 localhost $color_log_file_1 $hostname 0 > color_1.log 2>&1 &
color_pid_1=$!
echo "Started color process 1 with PID $color_pid_1" 0 >> color_1.log

./color 158541 317079 localhost $color_log_file_2 $hostname 0 > color_2.log 2>&1 &
color_pid_2=$!
echo "Started color process 2 with PID $color_pid_2" $hostname  0 >> color_2.log
# Wait for all ./color commands to complete

echo "Color process PID 1: $color_pid_1"
echo "Color process PID 2: $color_pid_2"
echo

wait $color_pid_1
wait $color_pid_2

../../KeyDB/src/keydb-cli set host_lhotse_1 1
# Capture the end time
end_time=$(date +%s)

if ! nc -zv localhost 6379; then
  echo "KeyDB server is down, exiting..."
  exit 1
fi

# Calculate the elapsed time
elapsed_time=$((end_time - start_time))

echo "Execution time: $elapsed_time seconds" 
echo "Execution of all ./color commands is complete."
echo "Monitor script terminated."
