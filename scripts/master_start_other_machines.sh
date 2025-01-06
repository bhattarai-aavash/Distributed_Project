#!/bin/bash

#!/bin/bash

get_ip_normal(){ 
    ip address | grep -Eo 'inet (addr:)?([0-9]*\.){3}[0-9]*' | grep -Eo '([0-9]*\.){3}[0-9]*' | grep -v '127.*.*.*' | grep -v '192.168.*.*' | grep -v '172.*.*.*'
}


# Get the servers array and username from arguments
servers=("$@")
username="${servers[-1]}"  # The last argument is the username
unset servers[-1]          # Remove the username from the servers array
num_servers=${#servers[@]}





base_target_directory="$(pwd)"
base_target_directory="${base_target_directory%/*}"   
base_target_directory="$(basename $base_target_directory)"
echo "  base target directory =$base_target_directory"



compileTarFilename="redis.tar.gz"

# Declare an associative array for machine_map
declare -A machine_map

display_map(){
    for key in "${!machine_map[@]}"; do
        echo "  $key : ${machine_map[$key]}"
    done
}

generate_map(){
for server in "${servers[@]}"; do
    machine_map["$server"]="${username}@$server.uwyo.edu"
    
done
display_map
}

# generate_map

# test_ssh(){
# echo "Attempting to SSH into each server..."
#     for key in "${!machine_map[@]}"; do
#         echo "Connecting to ${machine_map[$key]}..."
#         ssh "${machine_map[$key]}" "ip addr"  
#     done
# }





copy_code_to_machines(){

    generate_map

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
    echo $(date) " COPYING CODE TO MACHINES:"
    echo "----------------------------------------------------------------------------------------------------------------"
    echo

    for key in "${!machine_map[@]}"
    do
        value=${machine_map[$key]}
        destination=$key
        username=${value%@*}
        role_list=${value#*@}
        target_directory="redis"
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
        ssh $username@$destination "cd $target_directory; tar -zxf $compileTarFilename"

        local_time_end=$(date +%s)
        local_time_duration=$(( $local_time_end - $local_time_start ))
        echo "        ... done in $local_time_duration seconds"
        echo
    done
    echo
    echo
    cd scripts
    pwd



}


start_servers(){
    generate_map
    echo
    echo "------------------------------------"
    echo $(date) " STARTING SERVERS:"
    echo "------------------------------------"
    echo

    STARTING_SERVERS_START=$(date +%s)

    server_node_id_counter=0
    
    for key in "${!machine_map[@]}"
    do
        echo "sup"
    done

}

start_servers