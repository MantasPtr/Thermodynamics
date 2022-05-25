ssh:
	ssh mape3071@uosis.mif.vu.lt

clean:
	rm status.txt
	rm id.txt

clean-result:
	find  /scratch/lustre/home/mape3071/Thermodynamics/src//parallel/ -name *.out -type f -delete

submit:
	sbatch mpi-thd-par.sh > status.txt
	cat status.txt
	cat status.txt | awk '{print $$4}' > id.txt

submit-seq:
	sbatch thd-seq.sh > status.txt
	cat status.txt
	cat status.txt | awk '{print $$4}' > id.txt

submit-gpu:
	sbatch thd-gpu.sh > status.txt
	cat status.txt
	cat status.txt | awk '{print $$4}' > id.txt

status:
	squeue -j $$(cat id.txt)

output:
	cat /scratch/lustre/home/mape3071/Thermodynamics/src/parallel/slurm-$$(cat id.txt).out


output-seq:
	cat /scratch/lustre/home/mape3071/Thermodynamics/src/sequential/slurm-$$(cat id.txt).out

