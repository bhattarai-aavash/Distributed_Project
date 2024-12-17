#!/bin/bash

# Capture the start time
start_time=$(date +%s)
 

gcc -o color color.c -lhiredis
# Run the program
./color 0 317080


#211387
# Capture the end time
end_time=$(date +%s)

# Calculate the elapsed time
elapsed_time=$((end_time - start_time))

echo "Execution time: $elapsed_time seconds"