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

struct MatrixCoordinates
{
    int row_start;
    int row_end;
    int column_start;
    int column_end;
};

/**
 * Implementation v0 - the matrix is split into even strips, all data is being comunicated between
 */
class FullCommunicator
{
public:
    ProcConfig proc_config;
    DebugConfig debug_config;
    MatrixConfig matrix_config;
    int work_row_count;
    int send_blocksize;
    int block_row_count;
    int recv_blocksize;
    MPI_Status com_status;
    int mpi_com_tag;
    MatrixCoordinates matrix_coordinates;
    double *current_matrix;
    double *temp_matrix;

    FullCommunicator(ProcConfig arg_proc_config,
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

    double *get_intial_matrix()
    {
        printf("Generating matrix for proc %d\n", proc_config.proc_id);
        return generate_part_of_the_matrix(matrix_config.dimention, matrix_config.max_value, matrix_coordinates.row_start, matrix_coordinates.row_end, matrix_coordinates.column_start, matrix_coordinates.column_end);
    }

    void spread(double *full_matrix)
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

    void collect(double *full_matrix)
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

    void cleanup()
    {
        delete current_matrix;
        delete temp_matrix;
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

class RowCommunicator
{
public:
    ProcConfig proc_config;
    DebugConfig debug_config;
    MatrixConfig matrix_config;
    int communication_size;
    int proc_id_before;
    int proc_id_after;
    int last_row_offset;
    int first_row_offset;
    int work_row_count;
    int send_blocksize;
    MPI_Status com_status;
    int block_row_count;
    MPI_Request *all_requests;
    MPI_Request request_send_before;
    MPI_Request request_send_after;
    MPI_Request request_recv_before;
    MPI_Request request_recv_after;
    double *current_matrix;
    double *temp_matrix;
    MatrixCoordinates matrix_coordinates;
    int mpi_com_tag = 1;

    RowCommunicator(ProcConfig arg_proc_config,
                    DebugConfig arg_debug_config,
                    MatrixConfig arg_matrix_config) : proc_config(arg_proc_config), debug_config(arg_debug_config), matrix_config(arg_matrix_config)
    {
        work_row_count = (arg_matrix_config.dimention - 2) / arg_proc_config.proc_count; // -2 is for first and last row - they are constants, so no computation
        block_row_count = work_row_count + 2;                                            // +2 is padding (extra line on top and bellow), used in computation, but not modified
        send_blocksize = block_row_count * arg_matrix_config.dimention;
        // row split
        matrix_coordinates.column_start = (arg_matrix_config.dimention - 2) / proc_config.proc_count * proc_config.proc_id;
        matrix_coordinates.column_end = matrix_coordinates.column_start + block_row_count - 1; // -1 - since 0 based
        matrix_coordinates.row_start = 0;
        matrix_coordinates.row_end = arg_matrix_config.dimention;
        printf("process %d matrix coordinates: %d %d %d %d\n", proc_config.proc_id, matrix_coordinates.column_start, matrix_coordinates.column_end, matrix_coordinates.row_start, matrix_coordinates.row_end);

        current_matrix = new double[send_blocksize];
        temp_matrix = new double[send_blocksize];
        all_requests = new MPI_Request[4];

        communication_size = matrix_config.dimention - 2; // SKIP 2 as they are border values
        proc_id_before = proc_config.proc_id - 1;
        proc_id_after = proc_config.proc_id + 1;
        first_row_offset = 1;                                                   // SKIP 1 as it is a border value
        last_row_offset = (matrix_config.dimention * (work_row_count + 1) + 1); // SKIPPING TO LAST ROW + SKIP 1 as it is a border value
    };

    double *get_intial_matrix()
    {
        printf("Generating matrix for proc %d\n", proc_config.proc_id);
        return generate_part_of_the_matrix(matrix_config.dimention, matrix_config.max_value, matrix_coordinates.row_start, matrix_coordinates.row_end, matrix_coordinates.column_start, matrix_coordinates.column_end);
    }

    void send()
    {
        printf("sizes: %d %d %f \n", send_blocksize, communication_size, current_matrix[communication_size]);
        if (proc_id_before >= 0)
        {
            // TODO: Double check this
            printf("core %d sending before %d \n", proc_config.proc_id, proc_id_before);
            printf("core %d first_row_offset %d com_size %d matrix size %d \n", proc_config.proc_id, first_row_offset, communication_size, send_blocksize);
            printf("core %d matrix %p %p %f\n", proc_config.proc_id, temp_matrix, temp_matrix + first_row_offset, temp_matrix[first_row_offset + communication_size]);
            MPI_Isend(
                temp_matrix + first_row_offset, // SKIP 1 as it is a border value
                communication_size,
                MPI_DOUBLE,
                proc_id_before,
                mpi_com_tag,
                MPI_COMM_WORLD,
                &request_send_before);
        }
        if (proc_id_after < proc_config.proc_count)
        {
            // TODO: Double check this
            printf("core %d sending after %d \n", proc_config.proc_id, proc_id_after);
            printf("core %d last_row_offset %d com_size %d matrix size %d \n", proc_config.proc_id, last_row_offset, communication_size, send_blocksize);
            printf("core %d matrix %p %p %f\n", proc_config.proc_id, temp_matrix, temp_matrix + last_row_offset, temp_matrix[last_row_offset + communication_size]);
            MPI_Isend(
                temp_matrix + last_row_offset,
                communication_size,
                MPI_DOUBLE,
                proc_id_after,
                mpi_com_tag,
                MPI_COMM_WORLD,
                &request_send_after);
        }
    };

    void receive()
    {
        if (proc_id_after < proc_config.proc_count)
        {
            // TODO: Double check this
            printf("core %d receiving after %d \n", proc_config.proc_id, proc_id_after);
            printf("core %d first_row_offset %d com_size %d matrix size %d \n", proc_config.proc_id, first_row_offset, communication_size, send_blocksize);
            printf("core %d matrix %p %p %f\n", proc_config.proc_id, current_matrix, current_matrix + first_row_offset, current_matrix[first_row_offset + communication_size]);
            MPI_Irecv(
                current_matrix + first_row_offset,
                communication_size,
                MPI_DOUBLE,
                proc_id_after,
                mpi_com_tag,
                MPI_COMM_WORLD,
                &request_recv_after);
        }
        if (proc_id_before >= 0)
        {
            // TODO: Double check this
            printf("core %d receiving before %d \n", proc_config.proc_id, proc_id_before);
            printf("core %d last_row_offset %d com_size %d matrix size %d \n", proc_config.proc_id, last_row_offset, communication_size, send_blocksize);
            printf("core %d matrix %p %p %f\n", proc_config.proc_id, current_matrix, current_matrix + last_row_offset, current_matrix[last_row_offset + communication_size]);
            MPI_Irecv(
                current_matrix + last_row_offset,
                communication_size,
                MPI_DOUBLE,
                proc_id_before,
                mpi_com_tag,
                MPI_COMM_WORLD,
                &request_recv_before);
        }
    };

    void wait_for_communication()
    {
        int all_request_counter = 0;
        if (proc_id_after < proc_config.proc_count)
        {
            // printf("core %d waiting send after core \n", proc_config.proc_id);
            // MPI_Wait(&request_send_after, MPI_STATUS_IGNORE);
            // printf("core %d waiting recv after core \n", proc_config.proc_id);
            // MPI_Wait(&request_recv_after, MPI_STATUS_IGNORE);
            all_requests[all_request_counter] = request_send_after;
            all_requests[all_request_counter + 1] = request_recv_after;
            all_request_counter += 2;
        }
        if (proc_id_before >= 0)
        {

            // printf("core %d waiting recv before\n", proc_config.proc_id);
            // MPI_Wait(&request_recv_before, MPI_STATUS_IGNORE);
            // printf("core %d waiting send before\n", proc_config.proc_id);
            // MPI_Wait(&request_send_before, MPI_STATUS_IGNORE);

            all_requests[all_request_counter] = request_recv_before;
            all_requests[all_request_counter + 1] = request_send_before;
            all_request_counter += 2;
        }
        printf("all_request_counter %d pointers %p %p %p %p\n", all_request_counter, all_requests[0], all_requests[1], all_requests[2], all_requests[3]);
        MPI_Waitall(all_request_counter, all_requests, MPI_STATUSES_IGNORE);
    }

    void get_all(double **full_matrix)
    {
        if (proc_config.proc_id == 0)
        {
            // processor 0 did everything locally, so just coping directly to
            // skip row 1
            copy(current_matrix, current_matrix + send_blocksize, *full_matrix);
            // for others do sending
            for (int proc_id = 1; proc_id < proc_config.proc_count; proc_id++)
            {

                int offset = (proc_id * work_row_count) * matrix_config.dimention;
                MPI_Recv(
                    full_matrix + offset,
                    send_blocksize,
                    MPI_DOUBLE,
                    proc_id,
                    0,
                    MPI_COMM_WORLD,
                    &com_status);

                if (debug_config.communication_info)
                {
                    // printf("Primary process reveived %d message from process %d, saved to matrix [%d to %d] \n", 0, com_status.MPI_SOURCE, (proc_id * work_row_count + 1) * matrix_config.dimention, (proc_id * work_row_count + 1) * matrix_config.dimention + recv_blocksize);
                }
            }
        }
        else
        {
            if (debug_config.communication_info)
            {
                printf("Sending data back to master process from %d (total of %d)\n", proc_config.proc_id, send_blocksize);
            }
            MPI_Send(
                current_matrix, // skip first line since its all the same
                send_blocksize,
                MPI_DOUBLE,
                0,
                0,
                MPI_COMM_WORLD);
        }
        // TODO - make async
    }

    void do_termodynamics()
    {
        printf("* Thermodynamics\n");
        termodynamics(current_matrix, block_row_count, matrix_config.dimention, &temp_matrix);
        printf("* Copying to antoher matrix\n");
        copy(temp_matrix, temp_matrix + send_blocksize, current_matrix);
        printf("* Communication\n");
        communicate();
    }

    void communicate()
    {
        printf("= receiving\n");
        receive();
        printf("= sending\n");
        send();
        printf("= waiting\n");
        wait_for_communication();
    };

    void cleanup()
    {
        printf("cleaning up\n");
        delete current_matrix;
        delete temp_matrix;
        delete all_requests;
    }
};

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

    bool use_full_communicator = false;

    if (use_full_communicator)
    {

        FullCommunicator communicator = FullCommunicator(proc_config, config.debug, config.matrix);

        // -2 is for first and last row - they are constants, so no computation
        int work_row_count = (MATRIX_DIMENTION - 2) / proc_config.proc_count;
        // +2 is padding (extra line on top and bellow), used in computation, but not modified
        int block_row_count = work_row_count + 2;
        int send_blocksize = block_row_count * MATRIX_DIMENTION;
        int recv_blocksize = work_row_count * MATRIX_DIMENTION;

        if (proc_config.proc_id == 0)
        {
            if (debug_only_main_core)
            {
                printf(" - Matrix dimention %d \n - Iteration count %d \n - Draw interval %d \n - Will generate %d images \n", MATRIX_DIMENTION, MAX_ITERATION_COUNT, DRAW_FREQUENCY, (DRAW_FREQUENCY == 0) ? 0 : (MAX_ITERATION_COUNT / DRAW_FREQUENCY));
                printf(" - Each thread will process %d rows \n - Each thread will receive %d rows \n - Each thread will receive %d numbers \n - Each thread will send back %d numbers \n", work_row_count, block_row_count, send_blocksize, recv_blocksize);
            }
            if (DRAW_FREQUENCY > 0)
            {
                // save initial matrix value for visualization
                save_to_file(matrix, MATRIX_DIMENTION, MAX_MATRIX_VALUE, 0, USE_ABS_SCALE);
            }
        }

        // going throught termodinamic iterations
        for (int i = 1; i < MAX_ITERATION_COUNT; i++)
        {
            if (debug_time)
            {
                t1 = getTime();
            }
            communicator.spread(matrix);
            if (debug_time)
            {
                t2 = getTime();
                printf("Spreading data on proc %d took: %.3f s\n", proc_config.proc_id, t2 - t1);
                t1 = getTime();
            }
            termodynamics(communicator.current_matrix, block_row_count, MATRIX_DIMENTION, &communicator.temp_matrix);
            swap(communicator.current_matrix, communicator.temp_matrix);
            if (debug_time)
            {
                t2 = getTime();
                printf("%d iteration - process %d - time to calculate: %.3f s\n", i, proc_config.proc_id, t2 - t1);
                t1 = getTime();
            }
            communicator.collect(matrix);
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
        communicator.cleanup();
    }
    else
    {
        // NEW way
        RowCommunicator rowCommunicator = RowCommunicator(proc_config, config.debug, config.matrix);
        printf("Generating initial matrix\n");
        rowCommunicator.current_matrix = rowCommunicator.get_intial_matrix();
        if (proc_config.proc_id == 0)
        {
            if (debug_communication_info)
            {
                printf(" - Matrix dimention %d \n - Iteration count %d \n - Draw interval %d \n - Will generate %d images \n", MATRIX_DIMENTION, MAX_ITERATION_COUNT, DRAW_FREQUENCY, (DRAW_FREQUENCY == 0) ? 0 : (MAX_ITERATION_COUNT / DRAW_FREQUENCY));
                // printf(" - Each thread will process %d rows \n - Each thread will receive %d rows \n - Each thread will receive %d numbers \n - Each thread will send back %d numbers \n", work_row_count, block_row_count, send_blocksize, recv_blocksize);
            }
            if (DRAW_FREQUENCY > 0)
            {
                // save initial matrix value for visualization
                save_to_file(matrix, MATRIX_DIMENTION, MAX_MATRIX_VALUE, 0, USE_ABS_SCALE);
            }
        }
        printf("starting looping core %d\n", proc_config.proc_id);
        for (int i = 0; i < MAX_ITERATION_COUNT; i++)
        {
            if (debug_time)
            {
                t1 = getTime();
            }
            printf(" iteration %d, core %d\n", i, proc_config.proc_id);

            rowCommunicator.do_termodynamics();
            if (debug_time)
            {
                t2 = getTime();
                printf("%d iteration %d core - time to calculate: %.3f s\n", i, proc_config.proc_id, t2 - t1);
            }
        }
        printf("finished looping core %d\n", proc_config.proc_id);
        rowCommunicator.cleanup();
    }
    MPI_Finalize();
}
