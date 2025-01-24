#!/bin/bash


# Get the hostname
hostname=$(hostname)
# Define the log file path using the hostname


color_log_file_1="c1.log"
# Run the program
color_log_file_2="c2.log"

rm c1.log
rm c2.log


../../redis/src/redis-cli -h localhost FLUSHALL


../../redis/src/redis-cli -h localhost add_graph /home/abhattar/Desktop/project_fall_sem/redis/graph/simple_graph.txt

gcc -o color color.c -lhiredis


./color 0 2 localhost $color_log_file_1 &

./color 3 4 localhost $color_log_file_2 &



wait 

echo "Execution of all ./color commands is complete."
# Get the hostname of the computer (used as the log file name)


