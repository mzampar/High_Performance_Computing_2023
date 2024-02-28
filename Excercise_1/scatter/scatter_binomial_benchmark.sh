#!/bin/bash

#SBATCH --nodes=2
#SBATCH --ntasks-per-node=24
#SBATCH --time=02:00:00
#SBATCH --partition=THIN
#SBATCH --job-name=HPC_ex01_scatter_binomial
#SBATCH --exclusive
#SBATCH -A dssc
#SBATCH --exclude=fat[001-002]

# Load the openMPI module
module load openMPI/4.1.5/icc/2021.7.1

# Define the range of np values
np_values=$(seq 2 1 48)
# Define the range of map values
map_values="core socket node"

# Define filepaths
src_path="/u/dssc/mzampar/.local/modules/libexec/osu-micro-benchmarks/mpi/collective/"
out_csv="/u/dssc/mzampar/hpc_project/Excercise_1/scatter/results/scatter_binomial.csv"

# Create the CSV file with header
echo "Algorithm,Allocation,Processes,MessageSize,Latency" > $out_csv

# Iterate over map and np values
for map in $map_values; do
  for np in $np_values; do
    # Run the mpirun command
    echo "   Benchmarking Binomial with map=$map and np=$np"
    mpirun -np $np -map-by $map --mca coll_tuned_use_dynamic_rules true --mca coll_tuned_bcast_algorithm 2 ${src_path}osu_scatter -x 1000 -i 10000 | tail -n 21 \
    | awk -v np="$np" -v map="$map" '{printf "Binomial,%s,%s,%s,%s\n",map,np,$1, $2}' | sed 's/,$//' >> $out_csv
  done
done
