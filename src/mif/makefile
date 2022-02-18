ssh:
	ssh mape3071@uosis.mif.vu.lt

cd:
	cd /scratch/lustre/home/mape3071/Thermodynamics/src/mif

clean:
	rm status.txt
	rm id.txt

submit:
	sbatch mpi-thd-par.sh > status.txt
	cat status.txt
	cat status.txt | awk '{print $4}' > id.txt

status:
	squeue -j $(cat id.txt)

output:
	cat /scratch/lustre/home/mape3071/Thermodynamics/src/parallel slurm-{cat id.txt}.out

