#include <stdio.h>
#include <stdlib.h>
#include <tuple>

#include "../common/utils_flat.h"
#include "../common/display_utils_flat.h"
#include "../common/config_reader.h"
#include "../common/time_utils.h"

// #define MATRIX_DIMENTION 10
// #define MAX_MATRIX_VALUE 1.0
// #define MAX_ITERATION_COUNT 300
// #define DRAW_FREQUENCY 10
// #define USE_ABS_SCALE false

void termodynamics(double *matrix, int matrix_dimention, double **result_matrix)
{
    // double t1 = getTime();
    double *next_matrix = *result_matrix;
    for (int i = 0; i < matrix_dimention; i++)
    {
        for (int j = 0; j < matrix_dimention; j++)
        {
            if (i == 0 || i == matrix_dimention - 1 || j == 0 || j == matrix_dimention - 1)
            {
                // assuming sides has constant value
                next_matrix[i * matrix_dimention + j] = matrix[i * matrix_dimention + j];
            }
            else
            {
                next_matrix[i * matrix_dimention + j] =
                    (matrix[(i - 1) * matrix_dimention + j] +
                     matrix[(i + 1) * matrix_dimention + j] +
                     matrix[i * matrix_dimention + j - 1] +
                     matrix[i * matrix_dimention + j + 1]) /
                    4;
            }
        }
    }
    // double t2 = getTime();
    // printf("update time: %.3f\n", t2 - t1);
}

int main(int argc, char **argv)
{
    string config_location = "config.ini";
    if (argc == 2)
    {
        // argv[0] - program name
        config_location = argv[1];
    }
    double start_time = getTime();

    Configuration config = read_config(config_location);

    auto MATRIX_DIMENTION = config.matrix.dimention;
    auto MAX_MATRIX_VALUE = config.matrix.max_value;
    auto MAX_ITERATION_COUNT = config.calculation.max_iteration_count;
    auto DRAW_FREQUENCY = config.drawing.draw_frequency;
    auto USE_ABS_SCALE = config.drawing.use_abs_scale;

    double *matrix = generate_matrix(MATRIX_DIMENTION, MAX_MATRIX_VALUE);
    double *result_matrix = new double[MATRIX_DIMENTION * MATRIX_DIMENTION];

    if (DRAW_FREQUENCY > 0)
    {
        save_to_file(matrix, MATRIX_DIMENTION, MAX_MATRIX_VALUE, 0, USE_ABS_SCALE);
    }
    for (int i = 1; i < MAX_ITERATION_COUNT; i++)
    {
        termodynamics(matrix, MATRIX_DIMENTION, &result_matrix);
        swap(matrix, result_matrix);
        if (DRAW_FREQUENCY > 0 && i % DRAW_FREQUENCY == 0)
        {
            save_to_file(matrix, MATRIX_DIMENTION, MAX_MATRIX_VALUE, i, USE_ABS_SCALE);
        }
    }
    double end_time = getTime();
    printf("execution time: %.3f\n", end_time - start_time);
}
