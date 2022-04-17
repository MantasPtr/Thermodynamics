#include <mpi.h>

#include "../common/utils_flat.h"

#include "../common/config_reader.h"
#include "../common/custom_structs.h"
#include "./full_comunicator.h"

using namespace std;

/**
 * Implementation v0 - the matrix is split into even strips, all data is being comunicated between
 */

FullCommunicator::FullCommunicator(ProcConfig arg_proc_config,
                                   DebugConfig arg_debug_config,
                                   MatrixConfig arg_matrix_config) : proc_config(arg_proc_config), debug_config(arg_debug_config), matrix_config(arg_matrix_config)
{
    work_row_count = (arg_matrix_config.dimention - 2) / arg_proc_config.proc_count; // -2 is for first and last row - they are constants, so no computation
    block_row_count = work_row_count + 2;                                            // +2 is padding (extra line on top and bellow), used in computation, but not modified
    send_blocksize = block_row_count * arg_matrix_config.dimention;
    recv_blocksize = work_row_count * arg_matrix_config.dimention;
    // row split
    matrix_coordinates.column_start = (arg_matrix_config.dimention - 2) / proc_config.proc_count * proc_config.proc_id;
    matrix_coordinates.column_end = matrix_coordinates.column_start + block_row_count - 1; // -1 - since 0 based
    matrix_coordinates.row_start = 0;
    matrix_coordinates.row_end = arg_matrix_config.dimention;
    printf("process %d matrix coordinates: %d %d %d %d\n", proc_config.proc_id, matrix_coordinates.column_start, matrix_coordinates.column_end, matrix_coordinates.row_start, matrix_coordinates.row_end);

    current_matrix = new double[send_blocksize];
    temp_matrix = new double[send_blocksize];

    mpi_com_tag = 1;

    if (proc_config.proc_id == 0 && debug_config.communication_info)
    {
        printf(" - Matrix size - %d rows \n - Each thread will process %d rows \n - Each thread will receive %d rows \n - Each thread will receive %d numbers \n - Each thread will send back %d numbers \n", matrix_config.dimention, work_row_count, block_row_count, send_blocksize, recv_blocksize);
    }
}

double *FullCommunicator::get_intial_matrix()
{
    printf("Generating matrix for proc %d\n", proc_config.proc_id);
    return generate_part_of_the_matrix(matrix_config.dimention, matrix_config.max_value, matrix_coordinates.row_start, matrix_coordinates.row_end, matrix_coordinates.column_start, matrix_coordinates.column_end);
}

void FullCommunicator::spread(double *full_matrix)
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
                full_matrix + offset,
                send_blocksize,
                MPI_DOUBLE,
                proc_id,
                mpi_com_tag,
                MPI_COMM_WORLD);
        }
        // proc 0 does not send data - just copies
        copy(full_matrix, full_matrix + send_blocksize, current_matrix);
    }
    else
    {
        int sender_proc_id = 0;
        if (debug_config.communication_info)
        {
            printf("Process %d Waiting to receive data from %d (total of %d)\n", proc_config.proc_id, sender_proc_id, send_blocksize);
        }
        MPI_Recv(
            current_matrix,
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

void FullCommunicator::collect(double *full_matrix)
{
    if (proc_config.proc_id == 0)
    {
        // processor 0 did everything locally, so just coping directly to
        // skip row 1
        copy(current_matrix + matrix_config.dimention, current_matrix + matrix_config.dimention + recv_blocksize, full_matrix + matrix_config.dimention);
        // for others do sending
        for (int proc_id = 1; proc_id < proc_config.proc_count; proc_id++)
        {

            int offset = (proc_id * work_row_count + 1) * matrix_config.dimention;
            MPI_Recv(
                full_matrix + offset,
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
            current_matrix + matrix_config.dimention, // skip first line since its all the same
            recv_blocksize,
            MPI_DOUBLE,
            0,
            0,
            MPI_COMM_WORLD);
    }
}

void FullCommunicator::cleanup()
{
    delete current_matrix;
    delete temp_matrix;
};