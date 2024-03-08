#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks-per-node=24
#SBATCH --time=02:00:00
#SBATCH --partition=THIN
#SBATCH --job-name=HPC_ex02_mandelbrot
#SBATCH --exclusive
#SBATCH -A dssc
#SBATCH --exclude=fat[001-002]

# Load necessary modules
module load openMPI/4.1.5/icc/2021.7.1

# Set environment variables
export OMP_NUM_THREADS=24

# Compile the program
gcc -o ./build/mandelbrot_mpi mandelbrot_mpi.c -lm -fopenmp

# Define pixel and iteration parameters
pixel_values=(5000 7500 10000 15000)   # Example pixel values
iteration_values=(5000 7500 10000 15000)   # Example iteration values

# Output file for storing results
out_csv="./results/mandelbrot_mpi_execution_times.csv"

# Write header to CSV file
echo "Pixel_Value,Iteration_Value,Execution_Time" > "$out_csv"

# Loop over pixel values
for pixels in "${pixel_values[@]}"; do
    # Loop over iteration values
    for iterations in "${iteration_values[@]}"; do
        # Run the program and capture execution time
        execution_time=$( { time -p mpirun -np 2 ./build/mandelbrot_mpi $pixels $pixels -2.25 -1.5 0.75 1.5 $iterations 2>&1; } 2>&1 | grep real | awk '{print $2}' )

        # Append results to CSV file
        echo "$pixels,$iterations,$execution_time" >> "$out_csv"
    done
done

echo "Execution completed. Results saved to $out_csv"
