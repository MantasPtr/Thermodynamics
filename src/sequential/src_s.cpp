#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#include "../common/utils_flat.h"
#include "../common/display_utils_flat.h"
#include "../common/config_reader.h"

// #define MATRIX_DIMENTION 10
// #define MAX_MATRIX_VALUE 1.0
// #define MAX_ITERATION_COUNT 300
// #define DRAW_FREQUENCY 10
// #define USE_ABS_SCALE false

double *termodynamics(double *matrix, int matrix_dimention)
{
    double *next_matrix = empty_matrix(matrix_dimention);
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
    delete_matrix(matrix);
    return next_matrix;
}

int main()
{
    auto config = read_config("config.ini");

    auto MATRIX_DIMENTION = get<0>(config);
    auto MAX_MATRIX_VALUE = get<1>(config);
    auto MAX_ITERATION_COUNT = get<2>(config);
    auto DRAW_FREQUENCY = get<3>(config);
    auto USE_ABS_SCALE = get<4>(config);

    double *matrix = generate_matrix(MATRIX_DIMENTION, MAX_MATRIX_VALUE);
    save_to_file(matrix, MATRIX_DIMENTION, MAX_MATRIX_VALUE, 0, USE_ABS_SCALE);
    for (int i = 1; i < MAX_ITERATION_COUNT; i++)
    {
        matrix = termodynamics(matrix, MATRIX_DIMENTION);
        if (i % DRAW_FREQUENCY == 0)
        {
            save_to_file(matrix, MATRIX_DIMENTION, MAX_MATRIX_VALUE, i, USE_ABS_SCALE);
        }
    }
}
