default: thermo

mif_compile:
	nvcc -o thermo.sh -ccbin /usr/bin/gcc -gencode arch=compute_70,code=sm_70  ../common/utils_flat.cpp ../common/display_utils_flat.cpp ../common/time_utils.cpp ../common/config_reader.cpp ../lib/inih/cpp/INIReader.cpp ../lib/inih/ini.c thermo.cu
	# nvcc -o thermo.sh -ccbin /usr/bin/gcc   -gencode arch=compute_70,code=sm_70  ../common/utils_flat_float.cpp ../common/display_utils_flat_float.cpp ../common/time_utils.cpp ../common/config_reader.cpp ../lib/inih/cpp/INIReader.cpp ../lib/inih/ini.c thermo_float.cu

mif_test:
	./thermo.sh ../common/config_cs1.ini
	./thermo.sh ../common/config_cs2.ini
	./thermo.sh ../common/config_cs4.ini
	./thermo.sh ../common/config_cs64.ini
	# ./thermo.sh ../common/config_cs1024.ini
	# ./thermo.sh ../common/config_cs512.ini
	# ./thermo.sh ../common/config_cs256.ini
	# ./thermo.sh ../common/config_cs128.ini
	# ./thermo.sh ../common/config_cs64.ini
	# ./thermo.sh ../common/config_cs32.ini
	# ./thermo.sh ../common/config_cs16.ini
	# ./thermo.sh ../common/config_cs8.ini
	# ./thermo.sh ../common/config_cs512_md128.ini
	# ./thermo.sh ../common/config_cs512_md256.ini
	# ./thermo.sh ../common/config_cs512_md512.ini
	# ./thermo.sh ../common/config_cs512_md1024.ini
	# ./thermo.sh ../common/config_cs512_md2048.ini
	# ./thermo.sh ../common/config_cs512_md4192.ini
	# ./thermo.sh ../common/config_cs512_md8384.ini

tests:
	nvcc -o cuda_test.sh -gencode arch=compute_61,code=sm_61 test.cu 
	./cuda_test.sh

thermo: cleanup
	nvcc -o thermo.sh -gencode arch=compute_61,code=sm_61  ../common/utils_flat.cpp ../common/display_utils_flat.cpp ../common/time_utils.cpp ../common/config_reader.cpp ../lib/inih/cpp/INIReader.cpp ../lib/inih/ini.c thermo.cu
	./thermo.sh ../common/config.ini


thermo_float: cleanup
	nvcc -o thermo_float.sh  -gencode arch=compute_61,code=sm_61  ../common/utils_flat_float.cpp ../common/display_utils_flat_float.cpp ../common/time_utils.cpp ../common/config_reader.cpp ../lib/inih/cpp/INIReader.cpp ../lib/inih/ini.c thermo_float.cu
	./thermo_float.sh ../common/config.ini

memdebug:
	nvcc -o thermo_float.sh -lineinfo -gencode arch=compute_61,code=sm_61  ../common/utils_flat.cpp ../common/display_utils_flat.cpp ../common/time_utils.cpp ../common/config_reader.cpp ../lib/inih/cpp/INIReader.cpp ../lib/inih/ini.c thermo.cu
	cuda-memcheck ./thermo.sh ../common/config.ini


memdebug_float:
	nvcc -o thermo_float.sh -lineinfo -gencode arch=compute_61,code=sm_61  ../common/utils_flat_float.cpp ../common/display_utils_flat_float.cpp ../common/time_utils.cpp ../common/config_reader.cpp ../lib/inih/cpp/INIReader.cpp ../lib/inih/ini.c thermo_float.cu
	cuda-memcheck ./thermo_float.sh ../common/config.ini


cleanup:
	rm -rf ./images/*
