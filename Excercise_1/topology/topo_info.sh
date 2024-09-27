#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks-per-node=24
#SBATCH --time=00:05:00
#SBATCH --partition=THIN
#SBATCH --job-name=HPC_ex01
#SBATCH --exclusive
#SBATCH -A dssc

module load openMPI/4.1.6/gnu/14.2.1

module load hwloc/2.10.0

rm -f out.svg

srun -n1 -map-by node lstopo --whole-system out.svg