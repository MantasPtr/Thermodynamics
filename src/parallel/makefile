default: full
mpi: mpi_compile mpi_run
view: gif preview
full: cleanup mpi view
m4: cleanup compile mpi_run_4 view 

cleanup:
	rm -rf ./images/*

ubuntu_install:
	sudo apt-get install mpich

gif:
	convert -scale 1000x  -delay 10 -loop 0 images/*.pgm images/result.gif 
preview:
	eog images/result.gif

test_1: cleanup
	mpirun -n 1 ./program.sh ../test_files/config.ini
	diff ./images ../test_files/images

test_2: cleanup
	mpirun -n 2 ./program.sh ../test_files/config.ini
	diff ./images ../test_files/images

test_4: cleanup
	mpirun -n 4 ./program.sh ../test_files/config.ini
	diff ./images ../test_files/images


compile:
	mpic++ src_p.cpp ../common/utils_flat.cpp ../common/display_utils_flat.cpp ../common/config_reader.cpp ../common/time_utils.cpp ../lib/inih/cpp/INIReader.cpp ../lib/inih/ini.c -o program.sh -std=c++17
mpi_run:
	mpirun -n 2 ./program.sh
mpi_run_4:
	mpirun -n 4 ./program.sh

