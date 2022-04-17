#ifndef __CONFIG_READER_INCLUDED__
#define __CONFIG_READER_INCLUDED__

#include <iostream>
#include <tuple>

using namespace std;

struct MatrixConfig
{
    int dimention;
    double max_value;
};

struct CalculationConfig
{
    int max_iteration_count;
    double delta;
};

struct DrawingConfig
{
    int draw_frequency;
    bool use_abs_scale;
};

struct DebugConfig
{
    bool communication_info;
    bool time_info;
    bool only_main_core;
};

struct Configuration
{
    MatrixConfig matrix;
    CalculationConfig calculation;
    DrawingConfig drawing;
    DebugConfig debug;
};

Configuration read_config(string filename);

#endif // __CONFIG_READER_INCLUDED__
