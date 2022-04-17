#include <mpi.h>

#include "../common/config_reader.h"
#include "../common/custom_structs.h"

using namespace std;

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
                     MatrixConfig arg_matrix_config);

    void collect(double *full_matrix);
    void spread(double *full_matrix);
    void cleanup();
    double *get_intial_matrix();
};