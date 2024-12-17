#!/bin/bash

# List of remote nodes and directories
NODES=("mteverest2" "mteverest3" "mteverest4")
REMOTE_PATH="/home/abhattar/redis/1server/main.sh"


for i in "${!NODES[@]}"; do
  NODE="${NODES[$i]}"
  
  
  echo "Executing $SCRIPT on $NODE..."
  
  ssh "abhattar@$NODE" "nohup bash $REMOTE_PATH > main_script_output.log 2>&1 &"
done

echo "All scripts started in the background."