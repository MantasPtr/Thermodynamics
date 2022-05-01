#!/bin/sh
#SBATCH -p short
#SBATCH -n2
#SBATCH -C beta
#SBATCH -D /scratch/lustre/home/mape3071/Thermodynamics/src/parallel

make compile
make run
