default: cleanup gcc
gcc: compile run
view: gif preview

cleanup:
	rm -rf ./images/*

ubuntu_install:
	sudo apt-get install cmake

compile:
	g++ src_s.cpp ../common/utils_flat.cpp ../common/display_utils_flat.cpp ../common/time_utils.cpp ../common/config_reader.cpp ../lib/inih/cpp/INIReader.cpp ../lib/inih/ini.c -o program.sh -std=c++17
run:
	./program.sh ../common/config.ini


test: cleanup
	./program.sh ../test_files/config.ini
	diff ./images ../test_files/images

gif:
	convert -scale 1000x  -delay 10 -loop 0 images/*.pgm images/result.gif 
preview:
	eog images/result.gif


