#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <algorithm>

#include "../common/utils_flat.h"
#include "../common/display_utils_flat.h"
#include "../common/config_reader.h"
#include "../common/time_utils.h"

double *termodynamics(double *matrix, int dimention_x, int dimention_y)
{
    double *next_matrix = empty_matrix(dimention_x * dimention_y);
    double t1 = getTime();
    for (int i = 0; i < dimention_x; i++)
    {
        for (int j = 0; j < dimention_y; j++)
        {
            // first or last column or row
            if (i == 0 || i == dimention_x - 1 || j == 0 || j == dimention_y - 1)
            {
                // assuming sides has constant value
                next_matrix[i * dimention_y + j] = matrix[i * dimention_y + j];
            }
            else
            {
                next_matrix[i * dimention_y + j] = (matrix[(i - 1) * dimention_y + j] +
                                                    matrix[(i + 1) * dimention_y + j] +
                                                    matrix[i * dimention_y + j - 1] +
                                                    matrix[i * dimention_y + j + 1]) /
                                                   4;
            }
        }
    }
    double t2 = getTime();
    delete_matrix(matrix);
    double t3 = getTime();
    printf(" update time: %.3f\n delete time: %.3f\n", t2 - t1, t3 - t2);

    return next_matrix;
}

void termodynamics_2(double *matrix, int dimention_x, int dimention_y, double **result_matrix)
{
    double *next_matrix = *result_matrix;
    double t1 = getTime();
    for (int i = 0; i < dimention_x; i++)
    {
        for (int j = 0; j < dimention_y; j++)
        {
            // first or last column or row
            if (i == 0 || i == dimention_x - 1 || j == 0 || j == dimention_y - 1)
            {
                // assuming sides has constant value
                next_matrix[i * dimention_y + j] = matrix[i * dimention_y + j];
            }
            else
            {
                next_matrix[i * dimention_y + j] = (matrix[(i - 1) * dimention_y + j] +
                                                    matrix[(i + 1) * dimention_y + j] +
                                                    matrix[i * dimention_y + j - 1] +
                                                    matrix[i * dimention_y + j + 1]) /
                                                   4;
            }
        }
    }
    double t2 = getTime();
    printf("update time: %.3f\n", t2 - t1);
}

int main(int argc, char *argv[])
{
    auto config = read_config("config.ini");

    auto MATRIX_DIMENTION = get<0>(config);
    auto MAX_MATRIX_VALUE = get<1>(config);
    auto MAX_ITERATION_COUNT = get<2>(config);
    auto DRAW_FREQUENCY = get<3>(config);
    auto USE_ABS_SCALE = get<4>(config);

    double *matrix = generate_matrix(MATRIX_DIMENTION, MAX_MATRIX_VALUE);
    int id, proc_count;
    double start_time;
    MPI_Status com_status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_count);
    if (id == 0)
    {
        start_time = getTime();
    }
    // TODO: handle uneven division errors

    // -2 is for first and last row - they are constants, so no computation
    int work_row_count = (MATRIX_DIMENTION - 2) / proc_count;
    // +2 is padding (extra line on top and bellow), used in computation, but not modified
    int block_row_count = work_row_count + 2;
    int send_blocksize = block_row_count * MATRIX_DIMENTION;
    int recv_blocksize = work_row_count * MATRIX_DIMENTION;
    double *work_matrix = new double[send_blocksize];
    double *result_matrix = new double[send_blocksize];
    if (id == 0)
    {
        printf(" - Matrix dimention %d \n - Iteration count %d \n - Draw interval %d \n - Will generate %d images \n", MATRIX_DIMENTION, MAX_ITERATION_COUNT, DRAW_FREQUENCY, (DRAW_FREQUENCY == 0) ? 0 : (MAX_ITERATION_COUNT / DRAW_FREQUENCY));
        printf(" - Each thread will process %d rows \n - Each thread will receive %d rows \n - Each thread will receive %d numbers \n - Each thread will send back %d numbers \n", work_row_count, block_row_count, send_blocksize, recv_blocksize);
        double *matrix = generate_matrix(MATRIX_DIMENTION, MAX_MATRIX_VALUE);
        // save inital value
        save_to_file(matrix, MATRIX_DIMENTION, MAX_MATRIX_VALUE, 0, USE_ABS_SCALE);
    }

    // going throught termodinamic iterations
    for (int i = 1; i < MAX_ITERATION_COUNT; i++)
    {
        // primary process sends data
        if (id == 0)
        {
            for (int proc_id = 1; proc_id < proc_count; proc_id++) // TODO: handle 1 processor
            {
                int offset = proc_id * work_row_count * MATRIX_DIMENTION;
                printf("send data with offset of %d to processor %i\n", offset, id);
                printf("Sending data from %d (total of %d, usable %d) to process %d\n", id, send_blocksize, recv_blocksize, proc_id);

                MPI_Send(
                    matrix + offset, // TODO: check math for this thing
                    send_blocksize,
                    MPI_DOUBLE,
                    proc_id,
                    i,
                    MPI_COMM_WORLD);
            }
            std::copy(matrix, matrix + send_blocksize, work_matrix);
        }
        // others processes receive data
        else
        {
            MPI_Recv(
                work_matrix,
                send_blocksize,
                MPI_DOUBLE,
                0,
                i,
                MPI_COMM_WORLD,
                &com_status);
            printf("Process %d reveived %d message from process %d.  Error code %d\n", id, i, com_status.MPI_SOURCE, com_status.MPI_ERROR);
        }

        // MPI_Scatter(matrix + N * M / blockCount * i, M * oneBlockSize, MPI_INT, workMatrix, M * oneBlockSize, MPI_INT, 0, MPI_COMM_WORLD);
        // printf("Staring termodynamics iteration");
        // if (id == 2)
        // {
        //     printf("work matrix for process %d:\n", id);
        //     print_matrix(work_matrix, block_row_count, MATRIX_DIMENTION);
        // }
        // work_matrix = termodynamics(work_matrix, block_row_count, MATRIX_DIMENTION);
        termodynamics_2(work_matrix, block_row_count, MATRIX_DIMENTION, &result_matrix);
        swap(work_matrix, result_matrix);

        // printf("Ending termodynamics");

        if (id == 0)
        {
            // processor 0 did everything locally, so just coping directly to
            // skip row 1
            std::copy(work_matrix + MATRIX_DIMENTION, work_matrix + MATRIX_DIMENTION + recv_blocksize, matrix + MATRIX_DIMENTION);
            for (int proc_id = 1; proc_id < proc_count; proc_id++)
            {

                int offset = (proc_id * work_row_count + 1) * MATRIX_DIMENTION;
                MPI_Recv(
                    matrix + offset,
                    recv_blocksize,
                    MPI_DOUBLE,
                    proc_id,
                    i,
                    MPI_COMM_WORLD,
                    &com_status);
                // printf("Primary process reveived %d message from process %d, saved to matrix [%d to %d] \n", i, com_status.MPI_SOURCE, (proc_id * work_row_count + 1) * MATRIX_DIMENTION, (proc_id * work_row_count + 1) * MATRIX_DIMENTION + recv_blocksize);
            }
        }
        else
        {
            // printf("Sending data back to master process from %d (total of %d)\n", id, recv_blocksize);
            MPI_Send(
                work_matrix + MATRIX_DIMENTION, // skip first line since its all the same
                recv_blocksize,
                MPI_DOUBLE,
                0,
                i,
                MPI_COMM_WORLD);
        }

        if (DRAW_FREQUENCY > 0 && id == 0 && i % DRAW_FREQUENCY == 0)
        {
            printf("%d iterations has passed\n", i);
            save_to_file(matrix, MATRIX_DIMENTION, MAX_MATRIX_VALUE, i, USE_ABS_SCALE);
        }
    }
    if (id == 0)
    {
        double end_time = getTime();
        printf("execution time: %.3f\n", end_time - start_time);
    }
    MPI_Finalize();
}
