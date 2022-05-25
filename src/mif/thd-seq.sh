#!/bin/sh
#SBATCH -p main
#SBATCH -n1
#SBATCH -D /scratch/lustre/home/mape3071/Thermodynamics/src/sequential

make compile
make run
