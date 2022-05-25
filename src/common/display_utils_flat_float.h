void write_matrix_to_file(float *matrix, int matrix_dimention, float max_value, char *filename, bool use_abs_value);
void print_matrix(float *matrix, int matrix_dimention_y, int matrix_dimention_x);
char *generate_filename(char const *filename_postfix);
void save_to_file(float *matrix, int matrix_dimention, float max_value, int index, bool use_abs_value);