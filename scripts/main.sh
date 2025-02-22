#!/bin/bash

clients=("yangra4" "yangra5" "yangra6" "yangra8" "yangra9" "yangra10")
servers=("yangra4" "yangra5" "yangra6")
username="abhattar"
dataset="simple_graph.txt"

number_of_node=5
number_of_edges=10
start_node=0
end_node=317079
partition=6

# Convert arrays to comma-separated strings
clients_string=$(IFS=','; echo "${clients[*]}")
servers_string=$(IFS=','; echo "${servers[*]}")

# Debug: Print variables being passed
# echo "$number_of_node"
# echo "$number_of_edges"
# echo "$servers_string"
# echo "$clients_string"

# Pass variables and arrays to master_run_experiment.sh
bash master_run_experiment.sh $number_of_node $start_node $end_node $partition "$clients_string" "$servers_string" "$username" "$dataset"

EXPECTED_CLIENTS=6
check_completion() {
    while true; do
        local COMPLETED
        COMPLETED=$(../KeyDB/src/keydb-cli KEYS "*_status" | wc -l)
        echo "Clients completed: $COMPLETED / $EXPECTED_CLIENTS"

        if [[ "$COMPLETED" -eq "$EXPECTED_CLIENTS" ]]; then
            echo "All clients have completed coloring!"
            break
        fi

        sleep 10 # Wait before checking again
    done
}

# check_completion