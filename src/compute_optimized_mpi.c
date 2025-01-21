#include <omp.h>
#include <x86intrin.h>

#include "compute.h"





int convolve(matrix_t *a_matrix, matrix_t *b_matrix, matrix_t **output_matrix) {
  // TODO: convolve matrix a and matrix b, and store the resulting matrix in
  // output_matrix
  if (a_matrix == NULL || b_matrix == NULL) {
    return -1;
  }
  uint32_t m = a_matrix->rows;
  uint32_t n = a_matrix->cols;
  uint32_t k = b_matrix->rows;
  uint32_t l = b_matrix->cols;
  int32_t* a_data = a_matrix->data;
  int32_t* b_data = b_matrix->data;
  uint32_t output_rows = m - k + 1;
  uint32_t output_cols = n - l + 1;
  
  (*output_matrix) = malloc(sizeof(matrix_t));
  if (*output_matrix == NULL) {
    return -1;
  }
  (*output_matrix)->rows = output_rows;
  (*output_matrix)->cols = output_cols;
  (*output_matrix)->data = malloc(output_rows * output_cols * sizeof(int));
  if (*output_matrix == NULL) {
    return -1;
  }
  matrix_t* a_vector = malloc(sizeof(matrix_t));
  a_vector->data = malloc(sizeof(int) * l * k);
    #pragma omp parallel 
  {
#pragma omp for
      for (int i = 0; i < k; i++) {
        for (int j = 0; j < l; j++) {
            a_vector->data[(k - i) * l - 1 - j] = b_data[l * i + j];
        } 
      }
#pragma omp for
      for (int i = 0; i < output_rows; i++) {
        for (int j = 0; j < output_cols; j++) {
            int sum = 0;
            for (int r = 0; r < k; r++) {
                int32_t* vector1 = j + (r + i) * n + a_data;
                int32_t* vector2 = a_vector->data + r * l;
                int temp_sum = 0;
                __m256i sum_vector = _mm256_setzero_si256();
                for (int s = 0; s < l / 8 * 8; s+= 8) {
                    __m256i vector1_array = _mm256_loadu_si256((__m256i *)(vector1 + s));
                    __m256i vector2_array = _mm256_loadu_si256((__m256i *)(vector2 + s));
                    __m256i temp = _mm256_mullo_epi32(vector1_array, vector2_array);
                    sum_vector = _mm256_add_epi32(sum_vector, temp);
                }
                int32_t sum_array[8];
                _mm256_storeu_si256((__m256i *) sum_array, sum_vector);
                for (int f = 0; f < 8; f++) {
                    temp_sum += sum_array[f];
                }
                for (int g = l / 8 * 8; g < l; g++) {
                    temp_sum += vector1[g] * vector2[g];
                }
                sum += temp_sum;            
            }
            (*output_matrix)->data[i * output_cols + j] = sum;
        }
      }
    }
  free(a_vector->data);
  free(a_vector);    
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
