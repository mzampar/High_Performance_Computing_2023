#!/bin/bash

#SBATCH --nodes=4 
#SBATCH --ntasks-per-node=1
#SBATCH --time=02:00:00
#SBATCH --partition=THIN
#SBATCH --job-name=HPC_ex02_mandelbrot
#SBATCH --exclusive
#SBATCH -A dssc
#SBATCH --exclude=fat[001-002]

# Load necessary modules
module load openMPI/4.1.5/gnu/12.2.1

# Compile the program
mpicc -o ./build/mandelbrot mandelbrot.c -lm -fopenmp

# Output file for storing results
out_csv="./results/mandelbrot_mpi_execution_times.csv"
echo "Nodes,Threads,Time(s)\n" >> "$out_csv"

# Number of repetitions
repetitions=10


echo "Iteration,Total Tasks,Elapsed Time(s)" > "$output_file"  # Clear and set header

# Number of OpenMP threads
export OMP_NUM_THREADS=1

tasks_list=({2..24..2})

for ((i=1; i<=$repetitions; i++)); do
    for total_tasks in "${tasks_list[@]}"; do

        echo "Running iteration $i with $total_tasks MPI tasks."

        elapsed_time=$(mpirun -np $total_tasks --map-by core ./build/mandelbrot 1000 1000 -2 -2 2 2 1000 $OMP_NUM_THREADS | grep "Elapsed time:" | awk '{print $3}')
        
        # If elapsed_time is empty, skip this iteration
        if [ -z "$elapsed_time" ]; then
            continue
        fi

        echo "$i,$total_tasks,$elapsed_time" >> "$output_file"
    done
done

echo "Execution completed. Results saved to $output_file"