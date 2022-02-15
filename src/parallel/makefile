default: full
mpi: mpi_compile mpi_run
view: gif preview
full: cleanup mpi view
m4: cleanup mpi_compile mpi_run_4 view 

cleanup:
	rm -rf ./images/*

ubuntu_install:
	sudo apt-get install mpich

gif:
	convert -scale 1000x  -delay 10 -loop 0 images/*.pgm images/result.gif 
preview:
	eog images/result.gif

mpi_compile:
	mpic++ src_p.cpp ../common/utils_flat.cpp ../common/display_utils_flat.cpp -o program.sh
mpi_run:
	mpirun -n 2 ./p_program.sh
mpi_run_4:
	mpirun -n 4 ./p_program.sh

