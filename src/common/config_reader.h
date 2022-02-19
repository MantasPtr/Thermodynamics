#include <iostream>
#include <tuple>

using namespace std;

struct Configuration
{
    int matrix_dimention;
    double max_matrix_value;

    int max_iteration_count;
    double delta;

    int draw_frequency;
    bool use_abs_scale;

    bool communication_info;
    bool time_info;
    bool only_main_core;
};

Configuration read_config(string filename);
