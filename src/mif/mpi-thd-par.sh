#!/bin/sh
#SBATCH -p main
#SBATCH -n2
#SBATCH -D /scratch/lustre/home/mape3071/Thermodynamics/src/parallel
module load openmpi
# module load mpich
which mpicc
which mpic++
make compile
make run
