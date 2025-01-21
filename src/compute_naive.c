#include "compute.h"

// Computes the convolution of two matrices
int convolve(matrix_t *a_matrix, matrix_t *b_matrix, matrix_t **output_matrix) {
  // TODO: convolve matrix a and matrix b, and store the resulting matrix in
  // output_matrix
  if (a_matrix == NULL || b_matrix == NULL || output_matrix == NULL) {
    return -1;
  }
  uint32_t m = a_matrix->rows;
  uint32_t n = a_matrix->cols;
  uint32_t k = b_matrix->rows;
  uint32_t l = b_matrix->cols;
  uint32_t output_rows = m - k + 1;
  uint32_t output_cols = n - l + 1;
  *output_matrix = malloc(sizeof(matrix_t));
  if (*output_matrix == NULL) {
    return -1;
  }
  (*output_matrix)->rows = output_rows;
  (*output_matrix)->cols = output_cols;
  (*output_matrix)->data = malloc(output_rows * output_cols * sizeof(int32_t));
  if ((*output_matrix)->data == NULL) {
    free(*output_matrix);
    return -1;
  }
  for (uint32_t i = 0; i < output_rows; i++) {
    for (uint32_t j = 0; j < output_cols; j++) {
        int32_t sum = 0;
        for (uint32_t x = 0; x < k; x++) {
            for (uint32_t y = 0; y < l; y++) {
                sum += a_matrix->data[(i + x) * n + (j + y)] * b_matrix->data[(k - x - 1) * l + (l - y - 1)];
            }
        }
        (*output_matrix)->data[i * output_cols + j] = sum;
    }
  }
  return 0;
  }

// Executes a task
int execute_task(task_t *task) {
  matrix_t *a_matrix, *b_matrix, *output_matrix;

  char *a_matrix_path = get_a_matrix_path(task);
  if (read_matrix(a_matrix_path, &a_matrix)) {
    printf("Error reading matrix from %s\n", a_matrix_path);
    return -1;
  }
  free(a_matrix_path);

  char *b_matrix_path = get_b_matrix_path(task);
  if (read_matrix(b_matrix_path, &b_matrix)) {
    printf("Error reading matrix from %s\n", b_matrix_path);
    return -1;
  }
  free(b_matrix_path);

  if (convolve(a_matrix, b_matrix, &output_matrix)) {
    printf("convolve returned a non-zero integer\n");
    return -1;
  }

  char *output_matrix_path = get_output_matrix_path(task);
  if (write_matrix(output_matrix_path, output_matrix)) {
    printf("Error writing matrix to %s\n", output_matrix_path);
    return -1;
  }
  free(output_matrix_path);

  free(a_matrix->data);
  free(b_matrix->data);
  free(output_matrix->data);
  free(a_matrix);
  free(b_matrix);
  free(output_matrix);
  return 0;
}
