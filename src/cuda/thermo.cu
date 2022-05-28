#include <stdio.h>
#include <stdlib.h>
#include <tuple>

#include "../common/utils_flat.h"
#include "../common/display_utils_flat.h"
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

__global__ void thermodynamics_cuda(double *matrix, double *next_matrix, double *diff, int row_size, bool use_diff)
{
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
    if (use_diff)
    {
        diff[index] = next_matrix[index] - matrix[index];
    }
}

double largestElement(double *matrix, int row_size)
{
    int size = row_size * row_size;
    double max = -1;
    for (int i = 1; i < size; i++)
        if (matrix[i] > max)
            max = matrix[i];
    return max;
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
    printf("config read time: %.6f\n", getTime() - start_time);

    auto MATRIX_DIMENTION = config.matrix.dimention;
    auto MAX_MATRIX_VALUE = config.matrix.max_value;
    auto MAX_ITERATION_COUNT = config.calculation.max_iteration_count;
    auto DRAW_FREQUENCY = config.drawing.draw_frequency;
    auto USE_ABS_SCALE = config.drawing.use_abs_scale;
    auto MAX_DELTA = config.calculation.delta;
    auto CUDA_SPLIT = config.calculation.cuda_split;
    bool USE_DIFF = MAX_DELTA > 0.0;

    double *matrix = generate_matrix(MATRIX_DIMENTION, MAX_MATRIX_VALUE);
    double *result_matrix = new double[MATRIX_DIMENTION * MATRIX_DIMENTION];
    double *diff_matrix = new double[MATRIX_DIMENTION * MATRIX_DIMENTION];

    if (DRAW_FREQUENCY > 0)
    {
        save_to_file(matrix, MATRIX_DIMENTION, MAX_MATRIX_VALUE, 0, USE_ABS_SCALE);
    }
    double *cuda_matrix;
    double *cuda_result_matrix;
    double *cuda_diff_matrix;
    int size = MATRIX_DIMENTION * MATRIX_DIMENTION * sizeof(double);
    GE(cudaMalloc((void **)&cuda_matrix, size));
    GE(cudaMalloc((void **)&cuda_result_matrix, size));
    GE(cudaMemcpy(cuda_matrix, matrix, size, cudaMemcpyHostToDevice));
    GE(cudaMemcpy(cuda_result_matrix, result_matrix, size, cudaMemcpyHostToDevice));
    if (USE_DIFF)
    {
        GE(cudaMalloc((void **)&cuda_diff_matrix, size));
        GE(cudaMemcpy(cuda_diff_matrix, diff_matrix, size, cudaMemcpyHostToDevice));
    }
    int SPLIT = CUDA_SPLIT;
    printf("using %d %d data split \n", (MATRIX_DIMENTION * MATRIX_DIMENTION + SPLIT - 1) / SPLIT, SPLIT);
    int i = 0;
    double delta = MAX_MATRIX_VALUE;
    while (i < MAX_ITERATION_COUNT && delta > MAX_DELTA)
    {
        i++;
        thermodynamics_cuda<<<(MATRIX_DIMENTION * MATRIX_DIMENTION + SPLIT - 1) / SPLIT, SPLIT>>>(cuda_matrix, cuda_result_matrix, cuda_diff_matrix, MATRIX_DIMENTION, USE_DIFF);
        swap(cuda_matrix, cuda_result_matrix);
        if (DRAW_FREQUENCY > 0 && i % DRAW_FREQUENCY == 0)
        {
            GE(cudaMemcpy(matrix, cuda_result_matrix, size, cudaMemcpyDeviceToHost));
            save_to_file(matrix, MATRIX_DIMENTION, MAX_MATRIX_VALUE, i, USE_ABS_SCALE);
        }
        if ((USE_DIFF) && (i % 10000 == 0))
        {
            GE(cudaMemcpy(diff_matrix, cuda_diff_matrix, size, cudaMemcpyDeviceToHost));
            delta = largestElement(diff_matrix, MATRIX_DIMENTION);
            printf("iteration %d delta: %.7f\n", i, delta);
        }
    }
    GE(cudaMemcpy(matrix, cuda_result_matrix, size, cudaMemcpyDeviceToHost));
    GE(cudaDeviceSynchronize());
    double end_time = getTime();
    printf("iteration count: %d\n", i);
    printf("execution time: %.3f\n", end_time - start_time);
    cudaFree(cuda_matrix);
    cudaFree(cuda_result_matrix);
}
