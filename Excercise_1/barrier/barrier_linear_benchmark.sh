#!/bin/bash

#SBATCH --nodes=2
#SBATCH --ntasks-per-node=24
#SBATCH --time=02:00:00
#SBATCH --partition=THIN
#SBATCH --job-name=HPC_ex01_barrier_linear
#SBATCH --error=barrier_linear_%j.err
#SBATCH --output=barrier_linear_%j.out
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
out_csv="/u/dssc/mzampar/High_Performance_Computing_2023/Excercise_1/barrier/results/barrier_linear.csv"

# Create the CSV file with header
echo "Algorithm,Allocation,Processes,Latency, MinLatency, MaxLatency" > $out_csv

# Iterate over map and np values
for map in $map_values; do
  for np in $np_values; do
    # Run the mpirun command
    echo "   Benchmarking Linear with map=$map and np=$np"
    mpirun -np $np -map-by $map --mca coll_tuned_use_dynamic_rules true --mca coll_tuned_bcast_algorithm 1 ${src_path}osu_barrier -x 1000 -i 10000 -f | tail -n 1 \
    | awk -v np="$np" -v map="$map" '{printf "Linear,%s,%s,%s\n",map,np,$1}' >> $out_csv
  done
done
