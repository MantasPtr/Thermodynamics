#!/bin/sh
#SBATCH -p short
#SBATCH -n1
#SBATCH -C beta
#SBATCH -D /scratch/lustre/home/mape3071/Thermodynamics/src/sequential

make compile
make run
