
int sqr(int x);

float *empty_matrix(int matrix_dimention);
void delete_matrix(float *matrix);
float *generate_matrix(int matrix_dimention, float max_value);
float *generate_part_of_the_matrix(int matrix_dimention, float max_value, int row_start, int row_end, int column_start, int column_end);

enum matrix_prefil
{
    donut,
    corner
}; // TODO: use
