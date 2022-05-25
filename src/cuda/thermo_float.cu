#include <stdio.h>
#include <stdlib.h>
#include <tuple>

#include "../common/utils_flat_float.h"
#include "../common/display_utils_flat_float.h"
#include "../common/config_reader.h"
#include "../common/time_utils.h"

#define GE(ans)                               \
    {                                         \
        gpuAssert((ans), __FILE__, __LINE__); \
    }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort = true)
{
    if (code != cudaSuccess)
    {
        fprintf(stderr, "GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
        if (abort)
            exit(code);
    }
}

__global__ void thermodynamics_cuda(float *matrix, float *next_matrix, int row_size)
{

    // float diff;
    int index = blockDim.x * blockIdx.x + threadIdx.x;
    int x = index / row_size;
    int y = index % row_size;
    if ((x == 0) || (x == row_size - 1) || (y == 0) || (y == row_size - 1))
    {
        next_matrix[index] = matrix[index];
    }
    else
    {

        next_matrix[index] = (matrix[index - 1] +
                              matrix[index + 1] +
                              matrix[index - row_size] +
                              matrix[index + row_size]) /
                             4;
    }
    // diff = next_matrix[loc] - matrix[loc];
}

int main(int argc, char **argv)
{

    cudaDeviceSynchronize();
    double start_time = getTime();
    string config_location = "config.ini";
    if (argc == 2)
    {
        // argv[0] - program name
        config_location = argv[1];
    }

    Configuration config = read_config(config_location);
    printf("config read time: %.6f\n", (getTime() - start_time));

    auto MATRIX_DIMENTION = config.matrix.dimention;
    auto MAX_MATRIX_VALUE = (float)config.matrix.max_value;
    auto MAX_ITERATION_COUNT = config.calculation.max_iteration_count;
    auto DRAW_FREQUENCY = config.drawing.draw_frequency;
    auto USE_ABS_SCALE = config.drawing.use_abs_scale;

    float *matrix = generate_matrix(MATRIX_DIMENTION, MAX_MATRIX_VALUE);
    float *result_matrix = new float[MATRIX_DIMENTION * MATRIX_DIMENTION];

    if (DRAW_FREQUENCY > 0)
    {
        save_to_file(matrix, MATRIX_DIMENTION, MAX_MATRIX_VALUE, 0, USE_ABS_SCALE);
    }
    float *cuda_matrix;
    float *cuda_result_matrix;
    int size = MATRIX_DIMENTION * MATRIX_DIMENTION * sizeof(float);
    GE(cudaMalloc((void **)&cuda_matrix, size));
    GE(cudaMalloc((void **)&cuda_result_matrix, size));
    GE(cudaMemcpy(cuda_matrix, matrix, size, cudaMemcpyHostToDevice));
    GE(cudaMemcpy(cuda_result_matrix, result_matrix, size, cudaMemcpyHostToDevice));

    // dim3 threads(tx, ty);
    // dim3 blocks(bx, by);
    int SPLIT = 256;
    printf("using %d %d data split \n", (MATRIX_DIMENTION * MATRIX_DIMENTION + SPLIT - 1) / SPLIT, SPLIT);

    for (int i = 1; i < MAX_ITERATION_COUNT; i++)
    {
        thermodynamics_cuda<<<(MATRIX_DIMENTION * MATRIX_DIMENTION + SPLIT - 1) / SPLIT, SPLIT>>>(cuda_matrix, cuda_result_matrix, MATRIX_DIMENTION);
        swap(cuda_matrix, cuda_result_matrix);
        if (DRAW_FREQUENCY > 0 && i % DRAW_FREQUENCY == 0)
        {
            GE(cudaMemcpy(matrix, cuda_result_matrix, size, cudaMemcpyDeviceToHost));
            save_to_file(matrix, MATRIX_DIMENTION, MAX_MATRIX_VALUE, i, USE_ABS_SCALE);
        }
    }
    GE(cudaMemcpy(matrix, cuda_result_matrix, size, cudaMemcpyDeviceToHost));
    cudaDeviceSynchronize();
    double end_time = getTime();
    printf("execution time: %.3f\n", end_time - start_time);
    cudaFree(cuda_matrix);
    cudaFree(cuda_result_matrix);
}
