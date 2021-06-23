#!/bin/bash

# Maximum number of workers
nw_max=$1
# Background knowledge size, h = 1, ..., 5
h=$2
# Input file path
input=$3
# Output file path
output=$4

# CSV header
echo "nw,C++ Threads (Static),C++ Threads (Dynamic),FastFlow (Static),FastFlow (Dynamic),Ideal"

# Sequential computation
seq=$(for ((i=1;i<10;i++)); do ./prudence_threads_static 0 $h $input $output; done | awk '{sum += $2} END {print sum/NR}')
# Prints the first row of the CSV
echo "0,,,$seq"
# Varies the number of workers
for ((nw=1;nw<nw_max+1;nw*=2)); do
    # Time of the C++ Threads implementation
    cts_time=$(for ((i=1;i<10;i++)); do ./prudence_threads_static $nw $h $input $output; done | awk '{sum += $2} END {print sum/NR}')
    ctd_time=$(for ((i=1;i<10;i++)); do ./prudence_threads_dynamic $nw $h $input $output; done | awk '{sum += $2} END {print sum/NR}')
    # Time of the FastFlow implementation
    ffs_time=$(for ((i=1;i<10;i++)); do ./prudence_fastflow_static $nw $h $input $output; done | awk '{sum += $2} END {print sum/NR}')
    ffd_time=$(for ((i=1;i<10;i++)); do ./prudence_fastflow_dynamic $nw $h $input $output; done | awk '{sum += $2} END {print sum/NR}')
    # Ideal time (seq / nw)
    id_time=$(bc -l <<< $seq/$nw)
    # Prints the CSV row
    echo "$nw,$cts_time,$ctd_time,$ffs_time,$ffd_time,$id_time"
done
