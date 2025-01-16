#!/bin/bash

# Assign the first four arguments to variables
username=$1            # Username (single string)
dataset=$2             # Dataset name
servers_string=$3      # Servers array (comma-separated string)
clients_string=$4      # Clients array (comma-separated string)

# Remaining arguments are the partitions
shift 4                # Remove the first four arguments
partitions=("$@")      # Store all remaining arguments as an array

# Parse the clients and servers into arrays
IFS=',' read -r -a clients <<< "$clients_string"
IFS=',' read -r -a servers <<< "$servers_string"

# Initialize associative arrays
declare -A machine_server_map
declare -A machine_client_map

# Function to display the maps
display_map(){
    for key in "${!machine_server_map[@]}"; do
        echo " server $key : ${machine_server_map[$key]}"
    done

    for key in "${!machine_client_map[@]}"; do
        echo " client $key : ${machine_client_map[$key]}"
    done
}

# Function to generate the server and client maps
generate_map(){
    for server in "${servers[@]}"; do
        machine_server_map["$server"]="${username}@$server.uwyo.edu"
    done

    for client in "${clients[@]}"; do
        machine_client_map["$client"]="${username}@$client.uwyo.edu"
    done
    display_map
}





echo "Partitions:"
for partition in "${partitions[@]}"; do
    echo "$partition"
done



# Generate the maps
generate_map


# Implement your logic here
echo "---------------------------------------------------------------------------------------------------"
echo "                  Executing Code in Clients                                                        "
echo "---------------------------------------------------------------------------------------------------"

for client in "${!machine_client_map[@]}"; do
    echo "Connecting to ${machine_client_map[$client]}..."
    ssh "${machine_client_map[$client]}" "cd ~/fall_2024; ./process_graph.sh '${partitions[@]}' '$dataset'"
done

echo "---------------------------------------------------------------------------------------------------"
echo "                  Execution Complete                                                               "
echo "---------------------------------------------------------------------------------------------------"
