#!/bin/sh
#SBATCH -p gpu
#SBATCH --gres gpu:1
#SBATCH -n1
#SBATCH -D /scratch/lustre/home/mape3071/Thermodynamics/src/cuda

make