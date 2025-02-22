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





# echo "Partitions:"
# for partition in "${partitions[@]}"; do
#     echo "${partition/,/ }"
# done



pair_clients_to_servers() {
    generate_map
    declare -A server_client_map
    local server_index=0
    local num_servers=${#servers[@]}
    
    for client in "${clients[@]}"; do
        local assigned_server="${servers[$server_index]}"
        server_client_map["$assigned_server"]+="$client "

        # Move to the next server in a round-robin fashion
        server_index=$(( (server_index + 1) % num_servers ))
    done

    # Display the mapping
    for server in "${servers[@]}"; do
        echo "Server $server (${machine_server_map[$server]}) is assigned clients: ${server_client_map[$server]}"
    done
}



# run_code_1_1(){
#     # Implement your logic here
# echo "---------------------------------------------------------------------------------------------------"
# echo "                  Executing Code in Clients                                                        "
# echo "---------------------------------------------------------------------------------------------------"
#     generate_map
#     local index=0
#     for client in "${!machine_client_map[@]}"; do
#         partition="${partitions[index]}"
#         start=${partition%,*}  # Extract the first value (before comma)
#         end=${partition#*,}    # Extract the second value (after comma)

#         echo "Connecting to ${machine_client_map[$client]} with partition range $start to $end..."
        
#         ssh "${machine_client_map[$client]}" "cd /home/abhattar/code/color/1server; nohup ./color.sh $start $end 127.0.0.1 > color_${start}_${end}.log 2>&1 & echo \$! > color_${start}_${end}.pid"

#         ((index++))  # Move to the next partition
#     done
# }


run_code_1_many() {
    echo "---------------------------------------------------------------------------------------------------"
    echo "                  Executing Code in Clients                                                        "
    echo "---------------------------------------------------------------------------------------------------"
    generate_map
    
    local index=0
    declare -A server_client_map
    local server_index=0
    local num_servers=${#servers[@]}
    
    # Map clients to servers
    for client in "${clients[@]}"; do
        local assigned_server="${servers[$server_index]}"
        server_client_map["$client"]="$assigned_server"

        # Move to the next server in a round-robin fashion
        server_index=$(( (server_index + 1) % num_servers ))
    done

    # Execute the code on clients
    for client in "${!machine_client_map[@]}"; do
        partition="${partitions[index]}"
        start=${partition%,*}  # Extract the first value (before comma)
        end=${partition#*,}    # Extract the second value (after comma)

        ip="${machine_server_map[${server_client_map[$client]}]#*@}"

        echo
        echo
        echo
        echo "Connecting to ${machine_client_map[$client]} with partition range $start to $end..."
        
        ssh "${machine_client_map[$client]}" "cd /home/abhattar/code/color/1server; nohup ./color.sh $start $end $ip > color_${start}_${end}.log 2>&1 & echo \$! > color_${start}_${end}.pid"
        # echo "cd /home/abhattar/code/color/1server; nohup ./color.sh $start $end $ip > color_${start}_${end}.log 2>&1 & echo \$! > color_${start}_${end}.pid"
        ((index++))  # Move to the next partition
    done
}



wait_for_completion(){
    start_time=$(date +%s)  # Record the start time
    timeout=600  # 10 minutes in seconds

    while true; do
        all_done=true
        for client in "${!machine_client_map[@]}"; do
            if ssh "${machine_client_map[$client]}" "pgrep -f './color.sh' > /dev/null"; then
                all_done=false
                break
            fi
        done

        if $all_done; then
            echo "All experiments completed!"
            break
        fi

        current_time=$(date +%s)  # Get the current time
        elapsed_time=$((current_time - start_time))

        if (( elapsed_time >= timeout )); then
            echo "Timeout reached: Stopping wait after 10 minutes."
            break
        fi

        echo "Waiting for experiments to finish..."
        sleep 10  # Wait for 10 seconds before checking again
    done
}

close_servers() {
    generate_map
    echo
    echo "------------------------------------"
    echo "$(date) Closing SERVERS:"
    echo "------------------------------------"
    echo

    STARTING_SERVERS_START=$(date +%s)

    server_node_id_counter=0


    
    for key in "${!machine_server_map[@]}"
    do  
        echo "$key"
        destination=$key
         # Extract username from the value
         # Command to execute
        replicas=""
        for replica in ${replicas_map[$key]}; do
            replicas+="--replicaof $replica 6379 "
        done
        replicas="${replicas% }"  # Trim the trailing space
        # echo $replicas
        # echo "./fall_2024/KeyDB/src/keydb-server ./fall_2024/KeyDB/keydb.conf --multi-master yes --active-replica yes  $replicas;"
        
        # Run SSH command with a single block of shell commands
        
        ssh -n "$username@$destination" "pkill redis; pkill keydb;" 
        # ssh -n "$username@$destination" " ./fall_2024/KeyDB/src/keydb-server ./fall_2024/KeyDB/keydb.conf ; " 
        
        # # Check if the SSH command was successful and print output
        # if [ $? -eq 0 ]; then
        #     echo "Server $destination responded with: $output"
        # else
        #     echo "Failed to start server on $destination."
        # fi
    done

   
} 
# run_code
# wait_for_completion
# close_servers
# pair_clients_to_servers
run_code_1_many