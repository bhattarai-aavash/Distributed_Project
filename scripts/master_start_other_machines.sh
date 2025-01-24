#!/bin/bash



get_ip_normal(){ 
    ip address | grep -Eo 'inet (addr:)?([0-9]*\.){3}[0-9]*' | grep -Eo '([0-9]*\.){3}[0-9]*' | grep -v '127.*.*.*' | grep -v '192.168.*.*' | grep -v '172.*.*.*'
}

  # Clients array (3 elements)
username=$1 # Username (single string)
dataset=$2             # The last argument is the username
servers_string=$3  # Servers array (just 1 element)
clients_string=$4 



IFS=',' read -r -a clients <<< "$clients_string"
IFS=',' read -r -a servers <<< "$servers_string"




echo "------------------------------------------------------------------"
echo "              INSIDE START OTHER MACHINES                   "
echo "-------------------------------------------------------------------"
echo "Servers: ${servers[@]}"
echo "Clients: ${clients[@]}"
echo "Username: $username"
echo "Dataset: $dataset"

base_target_directory="$(pwd)"
base_target_directory="${base_target_directory%/*}"   
base_target_directory="$(basename $base_target_directory)"
echo "  base target directory =$base_target_directory"

compileTarFilename="redis.tar.gz"

# Declare an associative array for machine_map
declare -A machine_server_map
declare -A machine_client_map

display_map(){
    for key in "${!machine_server_map[@]}"; do
        echo " server $key : ${machine_server_map[$key]}"
    done

    for key in "${!machine_client_map[@]}"; do
        echo " client $key : ${machine_client_map[$key]}"
    done
}

generate_map(){
    server_node_id_counter=0
    for server in "${servers[@]}"; do
        machine_server_map["$server"]="${username}@$server.uwyo.edu"
    done

    client_node_id_counter=0
    for client in "${clients[@]}"; do
        machine_client_map["$client"]="${username}@$client.uwyo.edu"
    done

    # display_map
}

# Call generate_map to populate and display the mappings



# test_ssh(){
#     generate_map
# echo "Attempting to SSH into each server..."
#     for key in "${!machine_client_map[@]}"; do
#         echo "Connecting to ${machine_client_map[$key]}..."
#         ssh "${machine_client_map[$key]}" "ip addr"  
#     done
# }

# test_ssh



copy_code_to_machines(){

    generate_map
    
    cd ../redis
    make clean
    cd ../scripts
  


    COPY_START=$(date +%s)
    echo
    echo
    echo "compressing code"
    cd ../../$base_target_directory
    echo
    echo "  compressing $base_target_directory"

    subdirlist="$(ls --ignore=results* --ignore=config_repository --ignore=config_data_repository --ignore=all_config_data_repository --ignore=graph_dataset --ignore=*.tar.xz --ignore=*.tar.gz .)"
    echo "  subdirlist:"
    for sd in $subdirlist
    do
        echo "      $sd"
    done

    echo
    echo "  + removing tar file $compileTarFilename"
    rm -f $compileTarFilename
    tar -zcf $compileTarFilename $subdirlist
    
    COMPRESS_END=$(date +%s)
    COMPRESS_DURATION=$(( $COMPRESS_END - $COPY_START ))
    echo "      ... done ($COMPRESS_DURATION seconds)"
    echo


    echo
    echo "----------------------------------------------------------------------------------------------------------------"
    echo $(date) " COPYING CODE TO Client"
    echo "----------------------------------------------------------------------------------------------------------------"
    echo

    for key in "${!machine_client_map[@]}"
    do
        value=${machine_map[$key]}
        destination=$key
       
        role_list=${value#*@}
        target_directory="fall_2024"
        echo "        target_directory = $target_directory"
        echo " here here  $username@$destination"

        ssh $username@$destination "mkdir -p $target_directory; cd $target_directory; rm -rf *"


        local_time_start=$(date +%s)


        #copying file to the nodes
        echo "        rsync-ing $compileTarFilename to $username@$destination ..."
        rsync -arzSH $compileTarFilename $username@$destination:~/$target_directory/  
        local_time_end=$(date +%s)
        local_time_duration=$(( $local_time_end - $local_time_start ))
        echo "        ... done in $local_time_duration seconds"
        
        #uncompressing files in the node
        local_time_start=$(date +%s)
        echo "        uncompressing ..."
        ssh $username@$destination "cd $target_directory; tar -zxf $compileTarFilename;"

        local_time_end=$(date +%s)
        local_time_duration=$(( $local_time_end - $local_time_start ))
        echo "        ... done in $local_time_duration seconds"
        echo
    done
    echo "--------------------------------------------------------------------------"
    echo "                        Copying into Servers                              "    
    echo "--------------------------------------------------------------------------"
    
    for key in "${!machine_server_map[@]}"
    do
        value=${machine_map[$key]}
        destination=$key
        
        role_list=${value#*@}
        target_directory="fall_2024"
        echo "        target_directory = $target_directory"
        

        ssh $username@$destination "mkdir -p $target_directory; cd $target_directory; rm -rf *"


        local_time_start=$(date +%s)


        #copying file to the nodes
        echo "        rsync-ing $compileTarFilename to $username@$destination ..."
        rsync -arzSH $compileTarFilename $username@$destination:~/$target_directory/  
        local_time_end=$(date +%s)
        local_time_duration=$(( $local_time_end - $local_time_start ))
        echo "        ... done in $local_time_duration seconds"
        
        #uncompressing files in the node
        local_time_start=$(date +%s)
        echo "        uncompressing ..."
        ssh $username@$destination "cd $target_directory; tar -zxf $compileTarFilename; cd redis; make all"

        local_time_end=$(date +%s)
        local_time_duration=$(( $local_time_end - $local_time_start ))
        echo "        ... done in $local_time_duration seconds"
        echo
    done
  
    echo




}

start_servers() {
    generate_map
    echo
    echo "------------------------------------"
    echo "$(date) STARTING SERVERS:"
    echo "------------------------------------"
    echo

    STARTING_SERVERS_START=$(date +%s)

    server_node_id_counter=0


    
    for key in "${!machine_server_map[@]}"
    do
        destination=$key
         # Extract username from the value
         # Command to execute

        echo "Connecting to Server:$username@$destination to run redis server..."
        
        # Run SSH command with a single block of shell commands
        ssh -n "$username@$destination" "pkill redis;  ./fall_2024/redis/src/redis-server fall_2024/redis/redis.conf; " 
        
        # Check if the SSH command was successful and print output
        if [ $? -eq 0 ]; then
            echo "Server $destination responded with: $output"
        else
            echo "Failed to start server on $destination."
        fi
    done

   
}



store_graph_in_servers() {
    generate_map
    echo
    echo "---------------------------------------------------------------------------------"
    echo "$(date) STORE GRAPH DATASET ON  SERVERS:"
    echo "-----------------------------------------------------------------------------------"
    echo

    STARTING_SERVERS_START=$(date +%s)

    server_node_id_counter=0
    path_to_graph="fall_2024/redis/graph"
    path=$path_to_graph/$dataset
    echo "$path"
    
    for key in "${!machine_server_map[@]}"
    do
        destination=$key
         # Extract username from the value
         # Command to execute

        echo "Connecting to Server:$username@$destination to run redis server..."
        
        # Run SSH command with a single block of shell commands
        ssh -n "$username@$destination" "./fall_2024/redis/src/redis-cli add_graph $path  " 
        
        # Check if the SSH command was successful and print output
        if [ $? -eq 0 ]; then
            echo "Server $destination responded with: $output"
        else
            echo "Failed to start server on $destination."
        fi
    done

    
}

copy_code_to_machines