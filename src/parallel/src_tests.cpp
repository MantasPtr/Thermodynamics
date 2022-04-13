#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <algorithm>
#include <tuple>
#include <iterator>

using namespace std;

int main(int argc, char *argv[])
{
    int proc_id, proc_count;
    MPI_Status com_status;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &proc_id);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_count);
    MPI_Finalize();
}
