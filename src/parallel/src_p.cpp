#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <algorithm>
#include <tuple>
#include <iterator>

#include "../common/utils_flat.h"
#include "../common/display_utils_flat.h"
#include "../common/config_reader.h"
#include "../common/time_utils.h"

using namespace std;

struct ProcConfig
{
    int proc_id = -1;
    int proc_count = -1;
};

class Communicator
{
public:
    ProcConfig proc_config;
    DebugConfig debug_config;
    MatrixConfig matrix_config;
    int work_row_count;
    int send_blocksize;
    int recv_blocksize;
    MPI_Status com_status;
    int mpi_com_tag;

    Communicator(ProcConfig arg_proc_config,
                 DebugConfig arg_debug_config,
                 MatrixConfig arg_matrix_config) : proc_config(arg_proc_config), debug_config(arg_debug_config), matrix_config(arg_matrix_config)
    {
        // -2 is for first and last row - they are constants, so no computation
        work_row_count = (arg_matrix_config.dimention - 2) / arg_proc_config.proc_count;
        // +2 is padding (extra line on top and bellow), used in computation, but not modified
        int block_row_count = work_row_count + 2;
        send_blocksize = block_row_count * arg_matrix_config.dimention;
        recv_blocksize = work_row_count * arg_matrix_config.dimention;
        mpi_com_tag = 1;
    }

    void spread(double *matrix, double *work_matrix)
    {
        if (proc_config.proc_id == 0)
        {
            for (int proc_id = 1; proc_id < proc_config.proc_count; proc_id++)
            {
                int offset = proc_id * work_row_count * matrix_config.dimention;

                if (debug_config.communication_info)
                {
                    printf("Sending data from %d (total of %d, usable %d, offset %d) to process %d\n", proc_config.proc_id, send_blocksize, recv_blocksize, offset, proc_id);
                }
                MPI_Send(
                    matrix + offset,
                    send_blocksize,
                    MPI_DOUBLE,
                    proc_id,
                    mpi_com_tag,
                    MPI_COMM_WORLD);
            }
            printf("--\n");
            copy(matrix, matrix + send_blocksize, work_matrix);
        }
        else
        {
            int sender_proc_id = 0;
            if (debug_config.communication_info)
            {
                printf("Process %d Waiting to receive data from %d (total of %d)\n", proc_config.proc_id, sender_proc_id, send_blocksize);
            }
            MPI_Recv(
                work_matrix,
                send_blocksize,
                MPI_DOUBLE,
                sender_proc_id,
                mpi_com_tag,
                MPI_COMM_WORLD,
                &com_status);
            if (debug_config.communication_info)
            {
                printf("Process %d reveived message from process %d.  Error code %d\n", proc_config.proc_id, com_status.MPI_SOURCE, com_status.MPI_ERROR);
            }
        }
    }

    void collect(double *matrix, double *work_matrix)
    {
        if (proc_config.proc_id == 0)
        {
            // processor 0 did everything locally, so just coping directly to
            // skip row 1
            copy(work_matrix + matrix_config.dimention, work_matrix + matrix_config.dimention + recv_blocksize, matrix + matrix_config.dimention);
            // for others do sending
            for (int proc_id = 1; proc_id < proc_config.proc_count; proc_id++)
            {

                int offset = (proc_id * work_row_count + 1) * matrix_config.dimention;
                MPI_Recv(
                    matrix + offset,
                    recv_blocksize,
                    MPI_DOUBLE,
                    proc_id,
                    0,
                    MPI_COMM_WORLD,
                    &com_status);

                if (debug_config.communication_info)
                {
                    printf("Primary process reveived %d message from process %d, saved to matrix [%d to %d] \n", 0, com_status.MPI_SOURCE, (proc_id * work_row_count + 1) * matrix_config.dimention, (proc_id * work_row_count + 1) * matrix_config.dimention + recv_blocksize);
                }
            }
        }
        else
        {
            if (debug_config.communication_info)
            {
                printf("Sending data back to master process from %d (total of %d)\n", proc_config.proc_id, recv_blocksize);
            }
            MPI_Send(
                work_matrix + matrix_config.dimention, // skip first line since its all the same
                recv_blocksize,
                MPI_DOUBLE,
                0,
                0,
                MPI_COMM_WORLD);
        }
    }
};

void termodynamics(double *matrix, int dimention_x, int dimention_y, double **result_matrix)
{
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
}

int main(int argc, char *argv[])
{
    double start_time = getTime();
    double t1, t2;
    string config_location = "";
    if (argc == 2)
    {
        // argv[0] - program name
        config_location = argv[1];
    }

    t1 = getTime();
    Configuration config = read_config(config_location);
    t2 = getTime();

    auto MATRIX_DIMENTION = config.matrix.dimention;
    auto MAX_MATRIX_VALUE = config.matrix.max_value;
    auto MAX_ITERATION_COUNT = config.calculation.max_iteration_count;
    auto DRAW_FREQUENCY = config.drawing.draw_frequency;
    auto USE_ABS_SCALE = config.drawing.use_abs_scale;

    bool debug_communication_info = config.debug.communication_info;
    bool debug_only_main_core = config.debug.only_main_core;

    double *matrix = generate_matrix(MATRIX_DIMENTION, MAX_MATRIX_VALUE);
    ProcConfig proc_config;
    MPI_Status com_status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_config.proc_id);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_config.proc_count);

    bool debug_time = (proc_config.proc_id == 0 || !debug_only_main_core) && config.debug.time_info;
    if (debug_time)
    {
        printf("config read time: %.3f\n", t2 - t1);
    }
    // TODO: handle uneven division errors

    Communicator communicator = Communicator(proc_config, config.debug, config.matrix);

    // -2 is for first and last row - they are constants, so no computation
    int work_row_count = (MATRIX_DIMENTION - 2) / proc_config.proc_count;
    // +2 is padding (extra line on top and bellow), used in computation, but not modified
    int block_row_count = work_row_count + 2;
    int send_blocksize = block_row_count * MATRIX_DIMENTION;
    int recv_blocksize = work_row_count * MATRIX_DIMENTION;

    double *work_matrix = new double[communicator.send_blocksize];
    double *result_matrix = new double[communicator.send_blocksize];
    if (proc_config.proc_id == 0)
    {
        if (debug_only_main_core)
        {
            printf(" - Matrix dimention %d \n - Iteration count %d \n - Draw interval %d \n - Will generate %d images \n", MATRIX_DIMENTION, MAX_ITERATION_COUNT, DRAW_FREQUENCY, (DRAW_FREQUENCY == 0) ? 0 : (MAX_ITERATION_COUNT / DRAW_FREQUENCY));
            printf(" - Each thread will process %d rows \n - Each thread will receive %d rows \n - Each thread will receive %d numbers \n - Each thread will send back %d numbers \n", work_row_count, block_row_count, send_blocksize, recv_blocksize);
        }
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
        // TODO: remove matrix as it is unnecessary
        if (debug_time)
        {
            t1 = getTime();
        }
        communicator.spread(matrix, work_matrix);
        if (debug_time)
        {
            t2 = getTime();
            printf("Spreading data on proc %d took: %.3f s\n", proc_config.proc_id, t2 - t1);
            t1 = getTime();
        }
        termodynamics(work_matrix, block_row_count, MATRIX_DIMENTION, &result_matrix);
        swap(work_matrix, result_matrix);
        if (debug_time)
        {
            t2 = getTime();
            printf("%d iteration - process %d - time to calculate: %.3f s\n", i, proc_config.proc_id, t2 - t1);
            t1 = getTime();
        }
        communicator.collect(matrix, work_matrix);
        if (debug_time)
        {
            t2 = getTime();
            printf("Collecting data from process %d: %.3f s\n", proc_config.proc_id, t2 - t1);
        }
        if (DRAW_FREQUENCY > 0 && proc_config.proc_id == 0 && i % DRAW_FREQUENCY == 0)
        {
            printf("%d iterations has passed\n", i);
            save_to_file(matrix, MATRIX_DIMENTION, MAX_MATRIX_VALUE, i, USE_ABS_SCALE);
        }
    }
    if (proc_config.proc_id == 0)
    {
        double end_time = getTime();
        printf("Total execution time: %.3f\n", end_time - start_time);
    }
    delete matrix;
    delete work_matrix;
    delete result_matrix;
    MPI_Finalize();
}
