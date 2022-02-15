#include "utils_flat.h"
#include <stdio.h>
int sqr(int x)
{
    return x * x;
}

double *empty_matrix(int matrix_dimention)
{
    // Innitiate
    double *matrix = new double[matrix_dimention * matrix_dimention];
    for (int i = 0; i < matrix_dimention; i++)
    {
        // Clean matrix
        for (int j = 0; j < matrix_dimention; j++)
            matrix[i * matrix_dimention + j] = 0.0;
    }
    return matrix;
}

void delete_matrix(double *matrix)
{
    delete matrix;
}

double *generate_matrix(int matrix_dimention, int max_value)
{
    double *matrix = empty_matrix(matrix_dimention);

    // // fill sequence
    // for (int i = 0; i < matrix_dimention; i++)
    // {
    //     for (int j = 0; j < matrix_dimention; j++)
    //     {
    //         matrix[i * matrix_dimention + j] = (double)max_value / ((matrix_dimention - 1) * 2) * (i + j);
    //     }
    // }

    // fill donut

    int center_x = (int)(matrix_dimention / 2);
    int center_y = (int)(matrix_dimention / 2);
    int radius1 = (int)((center_x + matrix_dimention) / 5);
    int radius2 = (int)((center_x + matrix_dimention) / 6);

    for (int i = 0; i < matrix_dimention; i++)
    {
        for (int j = 0; j < matrix_dimention; j++)
        {
            bool is_inside_outer = sqr(i - center_x) + sqr(j - center_y) <= sqr(radius1);
            bool is_inside_inner = sqr(i - center_x) + sqr(j - center_y) <= sqr(radius2);
            matrix[i * matrix_dimention + j] = is_inside_outer && !is_inside_inner ? max_value : 0.0;
        }
    }

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
