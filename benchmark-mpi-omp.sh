#!/bin/bash

# Fixed seed value
SEED=1

SRC_FILE="./main.c"
BIN_FILE="./main"
HOSTS_FILE="./hosts.txt"
CFLAGS="-std=c11 -Ofast -lm -fopenmp -march=native"

# Compile the application
mpicc $SRC_FILE -o $BIN_FILE $CFLAGS

# Execution command
EXEC_CMD="mpirun -np 10 --hostfile $HOSTS_FILE $BIN_FILE"

# Define arrays for sample values of n and thread_count
declare -a n_values=(10 20 50 100 200 400)
declare -a thread_counts=(1 2 4 8)

# Function to convert time format to total seconds
convert_to_seconds() {
    local time_string=$1
    if [[ $time_string == *m* ]]; then
        local minutes=$(echo $time_string | awk -F'm' '{print $1}')
        local seconds=$(echo $time_string | awk -F'm' '{print $2}' | sed 's/s//')
        echo "$minutes * 60 + $seconds" | bc
    else
        echo $time_string | sed 's/s//'
    fi
}

# Loop over sample n values and thread counts
for n in "${n_values[@]}"; do
    for thread_count in "${thread_counts[@]}"; do
        echo "Running test for n=$n, seed=$SEED, thread_count=$thread_count"
        
        # Initialize an array to store execution times
        times=()

        # Run the application 10 times for each combination of n and thread_count
        for i in {1..10}; do
            # Measure the time and suppress the application output
            exec_time=$( { time $EXEC_CMD $n $SEED $thread_count > /dev/null 2>&1; } 2>&1 )
            # Extract real time in seconds and store in array
            real_time=$(echo $exec_time | grep real | awk '{print $2}')
            total_seconds=$(convert_to_seconds $real_time)
            times+=("$total_seconds")
        done

        # Process the times array to compute max, min, avg, median

        echo "Results for n=$n, thread_count=$thread_count:"
        echo "Max time: $(printf "%s\n" "${times[@]}" | sort -nr | head -n 1)"
        echo "Min time: $(printf "%s\n" "${times[@]}" | sort -n | head -n 1)"
        echo "Avg time: $(awk '{ total += $1; count++ } END { print total/count }' <<< "${times[@]}")"
        # Calculate median
        num_elements=${#times[@]}
        middle_index=$((num_elements / 2))
        if ((num_elements % 2 == 0)); then
            median=$(printf "%s\n" "${times[@]}" | sort -n | awk -v mid=$middle_index 'NR == mid || NR == mid+1 { total += $1; count++ } END { print total/count }')
        else
            median=$(printf "%s\n" "${times[@]}" | sort -n | awk -v mid=$middle_index 'NR == mid+1 { print $1 }')
        fi
        echo "Median time: $median"

        echo "--------------------------------------------------"
    done
done
