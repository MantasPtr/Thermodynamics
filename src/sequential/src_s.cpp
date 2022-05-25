#include <stdio.h>
#include <stdlib.h>
#include <tuple>
#include <algorithm>

#include "../common/utils_flat.h"
#include "../common/display_utils_flat.h"
#include "../common/config_reader.h"
#include "../common/time_utils.h"

double termodynamics(double *matrix, int matrix_dimention, double **result_matrix)
{
    double *next_matrix = *result_matrix;
    double delta = 0;
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
                delta = max(abs(next_matrix[i * matrix_dimention + j] - matrix[i * matrix_dimention + j]), delta);
            }
        }
    }
    return delta;
}

int main(int argc, char **argv)
{

    double start_time = getTime();
    string config_location = "config.ini";
    if (argc == 2)
    {
        // argv[0] - program name
        config_location = argv[1];
    }

    Configuration config = read_config(config_location);
    printf("config read time: %.6f\n", getTime() - start_time);

    auto MATRIX_DIMENTION = config.matrix.dimention + 2; // add side values
    auto MAX_MATRIX_VALUE = config.matrix.max_value;
    auto MAX_ITERATION_COUNT = config.calculation.max_iteration_count;
    auto MAX_DELTA = config.calculation.delta;
    auto DRAW_FREQUENCY = config.drawing.draw_frequency;
    auto USE_ABS_SCALE = config.drawing.use_abs_scale;

    double *matrix = generate_matrix(MATRIX_DIMENTION, MAX_MATRIX_VALUE);
    double *result_matrix = new double[MATRIX_DIMENTION * MATRIX_DIMENTION];

    if (DRAW_FREQUENCY > 0)
    {
        save_to_file(matrix, MATRIX_DIMENTION, MAX_MATRIX_VALUE, 0, USE_ABS_SCALE);
    }
    double delta = MAX_MATRIX_VALUE;
    int i = 1;
    while (i < MAX_ITERATION_COUNT && delta > MAX_DELTA)
    {
        delta = termodynamics(matrix, MATRIX_DIMENTION, &result_matrix);
        swap(matrix, result_matrix);
        if (DRAW_FREQUENCY > 0 && i % DRAW_FREQUENCY == 0)
        {
            printf("iteraction count: %d, max_delta: %.6f\n", i, delta);

            save_to_file(matrix, MATRIX_DIMENTION, MAX_MATRIX_VALUE, i, USE_ABS_SCALE);
        }
        i++;
    }

    double end_time = getTime();
    printf("iteraction count %d\n", i);
    printf("execution time: %.3f\n", end_time - start_time);
}
