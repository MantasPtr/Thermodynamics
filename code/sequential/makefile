default: full
gcc: compile run
view: gif preview
full: cleanup gcc view
cleanup:
	rm -rf ./images/*

compile:
	g++ src_s.cpp ../common/utils_flat.cpp ../common/display_utils_flat.cpp ../common/config_reader.cpp ../lib/inih/cpp/INIReader.cpp ../lib/inih/ini.c -o program.sh
run:
	./program.sh
scale1:
	convert -scale 1000x  -delay 10 -loop 0 image_0.pgm $(for i in $(seq 0 50 9950); do echo image_${i}.pgm; done) images/result.gif 
gif:
	convert -scale 1000x  -delay 10 -loop 0 images/*.pgm images/result.gif 
preview:
	eog images/result.gif


