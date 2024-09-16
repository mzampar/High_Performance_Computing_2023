#!/bin/bash

#SBATCH --nodes=2
#SBATCH --ntasks-per-node=24
#SBATCH --cpus-per-task=1
#SBATCH --time=00:30:00
#SBATCH --partition=THIN
#SBATCH --nodelist=thin001,thin002
#SBATCH --job-name=HPC_ex01
#SBATCH --exclusive
#SBATCH -A dssc

module load openMPI/4.1.6/gnu/14.2.1

CORES_PER_NODE=24
NODE1="thin001"
NODE2="thin002"

OSU_LATENCY_EXEC="/u/dssc/mzampar/.local/libexec/osu-micro-benchmarks/mpi/pt2pt/osu_latency"


OUTFILE="latency_results.csv"
echo "Core1,Core2,Latency(us)" > $OUTFILE

# Measure latencies within a single node
echo "Measuring latencies within $NODE1..." >> $OUTFILE

for ((i=0; i<$CORES_PER_NODE; i++)); do
    for ((j=i+1; j<$CORES_PER_NODE; j++)); do
        echo "Running latency between core $i and core $j on $NODE1..." >> $OUTFILE
        LATENCY=$(mpirun -np 2 --host thin001 --cpu-list $i,$j $OSU_LATENCY_EXEC)
    done
done

echo "Measuring latencies between $NODE1 and $NODE2..." >> $OUTFILE

for ((i=0; i<$CORES_PER_NODE; i++)); do
    for ((j=i; j<$CORES_PER_NODE; j++)); do
        echo "Running latency between core $i on $NODE1 and core $j on $NODE2..." >> $OUTFILE
        LATENCY=$(mpirun -np 2 --host $NODE1,$NODE2 --cpu-list $i,$j $OSU_LATENCY_EXEC)
    done
done

echo "Latency measurements completed. Results saved to $OUTFILE."
