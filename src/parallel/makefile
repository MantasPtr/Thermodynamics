default: cleanup mpi
mpi: compile mpi_run_2
view: gif preview
m4: cleanup compile mpi_run_4 

cleanup:
	rm -rf ./images/*

ubuntu_install:
	sudo apt-get update
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

run:
	mpirun ./program.sh ../common/config.ini
mpi_run_2:
	mpirun -n 2 ./program.sh ../common/config.ini
mpi_run_4:
	mpirun -n 4 ./program.sh ../common/config.ini

