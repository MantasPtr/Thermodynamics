#!/bin/sh
#SBATCH -p gpu
#SBATCH -n1
#SBATCH -D /scratch/lustre/home/mape3071/Thermodynamics/src/cuda

nvidia-smi