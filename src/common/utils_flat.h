
int sqr(int x);

double *empty_matrix(int matrix_dimention);
void delete_matrix(double *matrix);
double *generate_matrix(int matrix_dimention, double max_value);
double *generate_part_of_the_matrix(int matrix_dimention, double max_value, int row_start, int row_end, int column_start, int column_end);

enum matrix_prefil
{
    donut,
    corner
}; // TODO: use
