#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <algorithm>
#include <tuple>

#include "../common/utils_flat.h"
#include "../common/display_utils_flat.h"
#include "../common/config_reader.h"
#include "../common/time_utils.h"

using namespace std;

void termodynamics(double *matrix, int dimention_x, int dimention_y, double **result_matrix)
{
    double t1 = getTime();
    double *next_matrix = *result_matrix;

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
    double start_time = getTime();

    string config_location = "config.ini";
    if (argc == 2)
    {
        // argv[0] - program name
        config_location = argv[1];
    }

    Configuration config = read_config(config_location);

    auto MATRIX_DIMENTION = config.matrix_dimention;
    auto MAX_MATRIX_VALUE = config.max_matrix_value;
    auto MAX_ITERATION_COUNT = config.max_iteration_count;
    auto DRAW_FREQUENCY = config.draw_frequency;
    auto USE_ABS_SCALE = config.use_abs_scale;

    double *matrix = generate_matrix(MATRIX_DIMENTION, MAX_MATRIX_VALUE);
    int id, proc_count;
    MPI_Status com_status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_count);
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
        if (DRAW_FREQUENCY > 0)
        {
            // save initial matrix value for visualization
            save_to_file(matrix, MATRIX_DIMENTION, MAX_MATRIX_VALUE, 0, USE_ABS_SCALE);
        }
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
            copy(matrix, matrix + send_blocksize, work_matrix);
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

        termodynamics(work_matrix, block_row_count, MATRIX_DIMENTION, &result_matrix);
        swap(work_matrix, result_matrix);

        if (id == 0)
        {
            // processor 0 did everything locally, so just coping directly to
            // skip row 1
            copy(work_matrix + MATRIX_DIMENTION, work_matrix + MATRIX_DIMENTION + recv_blocksize, matrix + MATRIX_DIMENTION);
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
