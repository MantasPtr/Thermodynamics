#!/bin/sh
#SBATCH -p main
#SBATCH -n2
#SBATCH -D /scratch/lustre/home/mape3071/Thermodynamics/src/parallel
module load openmpi
make compile
make run
