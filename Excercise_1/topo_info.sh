#!/bin/bash

#SBATCH --nodes=1
#SBATCH --ntasks-per-node=24
#SBATCH --time=00:01:00
#SBATCH --partition=THIN
#SBATCH --nodelist=thin001
#SBATCH --job-name=HPC_ex01
#SBATCH --exclusive
#SBATCH -A dssc

module load openMPI/4.1.6/gnu/14.2.1

module load hwloc/2.10.0

srun -n1 -map-by socket lstopo --whole-system out.svg