

https://mif.vu.lt/cluster/
 

Example run:
```
 ssh mape3071@uosis.mif.vu.lt 
 kinit
 ssh hpc
 cd /scratch/lustre/home/mape3071/Thermodynamics/src/mif
 sbatch mpi-thd-par.sh
 squeue -j 982659
 cat /scratch/lustre/home/mape3071/Thermodynamics/src/parallel slurm-982659.out
```

Using makefile in cluster:
```
make cd
make submit
make status
make output
```

https://www.dsi.unive.it/~calpar/New_HPC_course/AA16-17/Project-Jacobi.pdf
