#include "utils_flat.h"
#include <stdio.h>
#include <math.h>

#define PI 3.14159265

int sqr(int x)
{
    return x * x;
}

double *empty_partial_matrix(int dim_x, int dim_y)
{
    // printf("EMPTY PARTIAL MATRIX = %d %d\n", dim_x, dim_y);
    // Innitiate
    double *matrix = new double[dim_x * dim_y];
    // Clean matrix
    for (int i = 0; i < dim_x; i++)
    {
        for (int j = 0; j < dim_y; j++)
            matrix[i * dim_y + j] = 0.0;
    }
    return matrix;
}

double *empty_matrix(int matrix_dimention)
{
    return empty_partial_matrix(matrix_dimention, matrix_dimention);
}

void delete_matrix(double *matrix)
{
    delete matrix;
}

double *generate_matrix(int matrix_dimention, double max_value)
{
    return generate_part_of_the_matrix(matrix_dimention, max_value, 0, matrix_dimention, 0, matrix_dimention);
}

double *generate_part_of_the_matrix(int matrix_dimention, double max_value, int row_start, int row_end, int column_start, int column_end)
{
    // printf("EMPTY MATRIX = %d %d %d %d %d\n", matrix_dimention, row_end, row_start, column_end, column_start);
    double *matrix = empty_partial_matrix(row_end - row_start, column_end - column_start);
    // printf("FILLING WAVES\n");
    int NUMBER_OF_WAVES = 5;

    for (int i = 0; i < matrix_dimention; i++)
    {
        double value = abs(sin((double)i / matrix_dimention * PI * NUMBER_OF_WAVES)) * max_value;
        if (column_start == 0)
            matrix[i] = value;
        if (matrix_dimention == column_end)
            matrix[matrix_dimention * (matrix_dimention - 1) + i] = value;
    }

    //

    // // fill sequence
    // for (int i = 0; i < matrix_dimention; i++)
    // {
    //     for (int j = 0; j < matrix_dimention; j++)
    //     {
    //         matrix[i * matrix_dimention + j] = (double)max_value / ((matrix_dimention - 1) * 2) * (i + j);
    //     }
    // }

    // fill donut

    // int center_x = (int)(matrix_dimention / 2);
    // int center_y = (int)(matrix_dimention / 2);
    // int radius1 = (int)((center_x + matrix_dimention) / 5);
    // int radius2 = (int)((center_x + matrix_dimention) / 6);

    // for (int i = column_start; i < column_end; i++)
    // {
    //     for (int j = row_start; j < row_end; j++)
    //     {
    //         bool is_inside_outer = sqr(i - center_x) + sqr(j - center_y) <= sqr(radius1);
    //         bool is_inside_inner = sqr(i - center_x) + sqr(j - center_y) <= sqr(radius2);
    //         matrix[i * matrix_dimention + j] = is_inside_outer && !is_inside_inner ? max_value : 0.0;
    //     }
    // }

    // fill corner

    // int center = (int)(matrix_dimention / 2);
    // for (int i = 0; i < center; i++)
    // {
    //     for (int j = center; j < matrix_dimention; j++)
    //     {
    //         matrix[i * matrix_dimention, +j] = max_value;
    //     }
    // }

    // fill corner L shape

    // int center = (int)(matrix_dimention / 2);
    // // for (int i = 0; i < center; i++)
    // for (int i = 0; i < matrix_dimention; i++)
    // {
    //     if (i % 4 >= 2)
    //         matrix[i * matrix_dimention + matrix_dimention - 1] = max_value;
    // }
    // // for (int j = center; j < MATRIX_DIMENTION; j++)
    // for (int i = 0; i < matrix_dimention; i++)
    // {
    //     if (i % 4 <= 1)
    //         matrix[i] = max_value;
    // }

    // // other corner L shape

    // // for (int i = center; i < MATRIX_DIMENTION; i++)
    // for (int i = 0; i < matrix_dimention; i++)
    // {
    //     if (i % 4 <= 1)
    //         matrix[i * matrix_dimention] = max_value;
    // }
    // // for (int j = 0; j < center; j++)
    // for (int i = 0; i < matrix_dimention; i++)
    // {
    //     if (i % 4 >= 2)
    //         matrix[(matrix_dimention - 1) * matrix_dimention + i] = max_value;
    // }

    // cross
    // for (int i = 0; i < matrix_dimention; i++)
    // {
    //     matrix[i * matrix_dimention + matrix_dimention / 2] = max_value;
    //     matrix[matrix_dimention / 2 * matrix_dimention + i] = max_value;
    // }

    return matrix;
}
