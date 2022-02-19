
#include <iostream>
#include <tuple>
#include "../lib/inih/cpp/INIReader.h"
#include "config_reader.h"

using namespace std;

Configuration read_config(string filename)
{

    INIReader reader(filename);
    if (reader.ParseError() < 0)
    {
        cerr << "Can't load" << filename << "\n";
        throw invalid_argument("Can't load 'configuration file");
    }

    /*
    [matrix]
    matrix_dimention=10
    max_matrix_value=1.0

    [calculation]
    max_iteration_count=300

    [drawing]
    draw_frequency=10
    use_abs_scale=false
    */
    Configuration config;
    config.draw_frequency = reader.GetInteger("drawing", "draw_frequency", 10);
    config.matrix_dimention = reader.GetInteger("matrix", "matrix_dimention", 1);
    config.max_matrix_value = reader.GetReal("matrix", "max_matrix_value", 1.0);
    config.max_iteration_count = reader.GetInteger("calculation", "max_iteration_count", 1);
    config.use_abs_scale = reader.GetBoolean("drawing", "use_abs_scale", false);
    return config;
}