
#include <iostream>
#include <tuple>
#include "../lib/inih/cpp/INIReader.h"
#include "config_reader.h"

using namespace std;

Configuration read_config(string filename)
{
    if (filename == "")
    {
        cerr << "No file name was passed";
        throw invalid_argument("No config file passed");
    }
    INIReader reader(filename);
    if (reader.ParseError() < 0)
    {
        cerr << "Can't load" << filename << "\n";
        throw invalid_argument("Can't load configuration file");
    }

    Configuration config;
    config.matrix_dimention = reader.GetInteger("matrix", "matrix_dimention", 1);
    config.max_matrix_value = reader.GetReal("matrix", "max_matrix_value", 1.0);

    config.max_iteration_count = reader.GetInteger("calculation", "max_iteration_count", 1);
    config.delta = reader.GetReal("calculation", "delta", 0.1);

    config.draw_frequency = reader.GetInteger("drawing", "draw_frequency", 10);
    config.use_abs_scale = reader.GetBoolean("drawing", "use_abs_scale", false);

    config.thread_info = reader.GetBoolean("debug", "thread_info", false);
    config.iteration_time = reader.GetBoolean("debug", "iteration_time", false);
    config.only_main_core = reader.GetBoolean("debug", "only_main_core", false);

    return config;
}