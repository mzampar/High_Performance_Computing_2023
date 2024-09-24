#!/bin/bash

#SBATCH --nodes=2
#SBATCH --ntasks-per-node=128
#SBATCH --time=02:00:00
#SBATCH --partition=EPYC
#SBATCH --job-name=mpi_affinity
#SBATCH --error=mpi_affinity_%j.err
#SBATCH --output=mpi_affinity_%j.out
#SBATCH --exclusive
#SBATCH -A dssc

# Load modules
module load openMPI/4.1.6/gnu/14.2.1

# Compile the program
mpicc -O3 -march=native -o ./build/mandelbrot mandelbrot.c -lm -fopenmp

# Output file for storing results
out_csv="./scaling/results/mpi_affinity.csv"

echo "Iteration,Total Tasks,Mapping,Binding,Elapsed Time(s),Computation Time(s),Gathering Time(s)" > "$out_csv"

# Number of OpenMP threads
export OMP_NUM_THREADS=1

tasks_list=(1 2 4 8)

# Define different mapping and binding strategies
mapping_list=("node" "socket" "core" "hwthread" "numa")
binding_list=("none" "core" "socket" "hwthread" "numa")

echo "Running MPI strong scaling with different mappings and bindings."

for total_tasks in "${tasks_list[@]}"; do
    for map_by in "${mapping_list[@]}"; do
        for bind_to in "${binding_list[@]}"; do
            echo "Running iteration $i with $total_tasks MPI tasks, mapping by $map_by and binding to $bind_to."
            output=$(mpirun -np $total_tasks --map-by $map_by --bind-to $bind_to ./build/mandelbrot 25000 25000 -1.5 -1.25 0.5 1.25 255)
            elapsed_time=$(echo "$output" | grep "Elapsed time:" | awk '{print $3}')
            computation_time=$(echo "$output" | grep "Computation time:" | awk '{print $3}')
            writing_time=$(echo "$output" | grep "Gathering time:" | awk '{print $3}')        
            echo "$i,$total_tasks,$map_by,$bind_to,$elapsed_time,$computation_time,$writing_time" >> "$out_csv"
        done
    done
done

echo "Execution completed. Results saved to $out_csv"