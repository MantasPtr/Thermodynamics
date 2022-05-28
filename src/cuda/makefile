default: thermo

mif: cleanup
	nvcc -o thermo.sh -ccbin /usr/bin/gcc -gencode arch=compute_61,code=sm_61  ../common/utils_flat.cpp ../common/display_utils_flat.cpp ../common/time_utils.cpp ../common/config_reader.cpp ../lib/inih/cpp/INIReader.cpp ../lib/inih/ini.c thermo.cu
	./thermo.sh ../common/config.ini

tests:
	nvcc -o cuda_test.sh -gencode arch=compute_61,code=sm_61 test.cu 
	./cuda_test.sh

thermo: cleanup
	nvcc -o thermo.sh -ccbin /usr/bin/gcc -gencode arch=compute_61,code=sm_61  ../common/utils_flat.cpp ../common/display_utils_flat.cpp ../common/time_utils.cpp ../common/config_reader.cpp ../lib/inih/cpp/INIReader.cpp ../lib/inih/ini.c thermo.cu
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
