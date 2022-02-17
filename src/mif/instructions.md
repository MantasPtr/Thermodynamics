

https://mif.vu.lt/cluster/
 
```
 ssh mape3071@uosis.mif.vu.lt 
 kinit
 ssh cluster
 cd /scratch/lustre/users/mape3071/Thermodynamics/src/mif
 sbatch mpi-thd-par.sh
 squeue -j 982659
 cat /scratch/lustre/users/mape3071/Thermodynamics/src/parallel slurm-982659.out
```


https://www.dsi.unive.it/~calpar/New_HPC_course/AA16-17/Project-Jacobi.pdf
