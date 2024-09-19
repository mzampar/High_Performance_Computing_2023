#!/bin/bash

#SBATCH --nodes=2
#SBATCH --ntasks-per-node=24
#SBATCH --time=02:00:00
#SBATCH --partition=THIN
#SBATCH --job-name=HPC_ex01_bcast_default
#SBATCH --error=bcast_default_%j.err
#SBATCH --output=bcast_default_%j.out
#SBATCH --nodelist=thin009,thin010
#SBATCH --exclusive
#SBATCH -A dssc

# Load the openMPI module
module load openMPI/4.1.6/gnu/14.2.1

# Define the range of np values
np_values=$(seq 2 2 48)
# Define the range of map values
map_values="core socket node"

# Define filepaths
src_path="../osu-micro-benchmarks-7.3/c/mpi/collective/blocking/"
out_csv="/u/dssc/mzampar/High_Performance_Computing_2023/Excercise_1/bcast/results/bcast_default.csv"

# Create the CSV file with header
echo "Algorithm,Allocation,Processes,MessageSize,Latency, MinLatency, MaxLatency" > $out_csv

# Iterate over map and np values
for map in $map_values; do
  for np in $np_values; do
    # Run the mpirun command
    echo "   Benchmarking Default with map=$map and np=$np"
    mpirun -np $np -map-by $map --mca coll_tuned_use_dynamic_rules true --mca coll_tuned_bcast_algorithm 0 ${src_path}osu_bcast -x 1000 -i 10000 -f | tail -n 21 \
    | awk -v np="$np" -v map="$map" '{printf "Default,%s,%s,%s,%s,%s,%s\n",map,np,$1, $2, $3, $4}' | sed 's/,$//' >> $out_csv
  done
done
