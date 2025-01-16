#!/bin/bash

clients=("mteverest1" "mteverest2" "mteverest3" )
servers=("yangra1" "mteverest4")
username="abhattar"
dataset="simple_graph.txt"

number_of_node=5
number_of_edges=10
start_node=0
end_node=4
partition=2

# Convert arrays to comma-separated strings
clients_string=$(IFS=','; echo "${clients[*]}")
servers_string=$(IFS=','; echo "${servers[*]}")

# Debug: Print variables being passed
echo "$number_of_node"
echo "$number_of_edges"
echo "$servers_string"
echo "$clients_string"

# Pass variables and arrays to master_run_experiment.sh
bash master_run_experiment.sh $number_of_node $start_node $end_node $partition "$clients_string" "$servers_string" "$username" "$dataset"
