default: full
mpi: mpi_compile mpi_run
view: gif preview
full: cleanup mpi view
m4: cleanup mpi_compile mpi_run_4 view 
cleanup:
	rm -rf ./images/*

scale:
	convert -scale 1000x  -delay 10 -loop 0 image_0.pgm $(for i in $(seq 0 50 9950); do echo image_${i}.pgm; done) images/result.gif 
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

debug: cleanup mpi
	convert -scale 1000x  images/image_00001.pgm images/image2_00001.pgm
	eog images/image2_00001.pgm

