#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define N_MIN 1
#define N_MAX 3000
#define K_MIN 1 // K <= N
#define L_MIN 0 // 2L < K
#define S_MIN 1
#define S_MAX 15000
#define TILE_GRASS 0
#define TILE_FOREST 1
#define TILE_ROCK 2

typedef enum {
  ERR_INVALID_INPUT = 100,
  ERR_MEM_ALLOC = 101,
} Error;

typedef struct {
  size_t N;
  size_t K;
  size_t L;
  size_t S;
  int *array;
  int *pref_forest;
  int *pref_rock;
} Park;

void Error__throw(Error e);
Park *Park__init(size_t N, size_t K, size_t L, size_t S);
Park *Park__read(FILE *fp);
size_t Park__solve(Park *p);
void Park__free(Park *p);

int main() {
  Park *p = Park__read(stdin);

  size_t solution = Park__solve(p);
  printf("%zu\n", solution);

  Park__free(p);

  return 0;
}

void Error__throw(Error e) {
  switch (e) {
  case ERR_INVALID_INPUT:
    fprintf(stderr, "Error: The input is invalid!\n");
    break;
  case ERR_MEM_ALLOC:
    fprintf(stderr, "Error: There was an error allocating memory!\n");
    break;
  }

  exit(e);
}

Park *Park__init(size_t N, size_t K, size_t L, size_t S) {
  Park *p = malloc(sizeof(Park));
  if (p == NULL) {
    Error__throw(ERR_MEM_ALLOC);
  }

  p->N = N;
  p->K = K;
  p->L = L;
  p->S = S;

  p->array = malloc(N * N * sizeof(*p->array));
  p->pref_forest = malloc(N * N * sizeof(*p->pref_forest));
  p->pref_rock = malloc(N * N * sizeof(*p->pref_rock));
  if (!p->array || !p->pref_forest || !p->pref_rock) {
    Park__free(p);
    Error__throw(ERR_MEM_ALLOC);
  }

  return p;
}

void Park__free(Park *p) {
  free(p->array);
  free(p->pref_forest);
  free(p->pref_rock);
  free(p);
}

Park *Park__read(FILE *fp) {
  // Read park dimensions
  int n, k, l, s;
  if (fscanf(fp, "%d %d %d %d", &n, &k, &l, &s) != 4) {
    Error__throw(ERR_INVALID_INPUT);
  }
  if (n < N_MIN || n > N_MAX || k < K_MIN || k > n || l < L_MIN || 2 * l >= k || s < S_MIN || s > S_MAX) {
    Error__throw(ERR_INVALID_INPUT);
  }

  size_t N = (size_t)n;
  size_t K = (size_t)k;
  size_t L = (size_t)l;
  size_t S = (size_t)s;
  Park *p = Park__init(N, K, L, S);

  // Fill the park and compute prefixes
  for (size_t i = 0; i < N; i++) {
    for (size_t j = 0; j < N; j++) {
      size_t idx = i * N + j;

      int val;
      if (fscanf(fp, "%d", &val) != 1 || (val != TILE_GRASS && val != TILE_FOREST && val != TILE_ROCK)) {
        Park__free(p);
        Error__throw(ERR_INVALID_INPUT);
      }
      p->array[idx] = val;

      int sum_forest = (val == TILE_FOREST) ? 1 : 0;
      if (i > 0) sum_forest += p->pref_forest[(i - 1) * N + j];
      if (j > 0) sum_forest += p->pref_forest[i * N + (j - 1)];
      if (i > 0 && j > 0) sum_forest -= p->pref_forest[(i - 1) * N + (j - 1)];
      p->pref_forest[idx] = sum_forest;

      int sum_rock = (val == TILE_ROCK) ? 1 : 0;
      if (i > 0) sum_rock += p->pref_rock[(i - 1) * N + j];
      if (j > 0) sum_rock += p->pref_rock[i * N + (j - 1)];
      if (i > 0 && j > 0) sum_rock -= p->pref_rock[(i - 1) * N + (j - 1)];
      p->pref_rock[idx] = sum_rock;
    }
  }

  return p;
}

size_t Park__solve(Park *p) {
  size_t max_forests = 0;

  // Compute the minimal coordintes of the park's right bottom corner (origin in upper left corner)
  size_t min_row = p->K - 1;
  size_t min_col = p->K - 1;

  for (size_t i = 0; i < p->N; i++) {
    for (size_t j = 0; j < p->N; j++) {
      if (i < min_col || j < min_row) {
        continue; // Skip nonsensical positions
      }

      // Calculate the position of bottom-right and top-left corners of the central area
      size_t cbr_row = i - p->L;
      size_t cbr_col = j - p->L;
      size_t central_N = p->K - 2 * p->L;
      size_t ctl_row = cbr_row - central_N + 1;
      size_t ctl_col = cbr_col - central_N + 1;
      // Calculate the number of rocks in the central area
      size_t rock_count = p->pref_rock[i * p->N + j];
      if (ctl_row > 0) rock_count -= p->pref_rock[(ctl_row - 1) * p->N + j];
      if (ctl_col > 0) rock_count -= p->pref_rock[i * p->N + (ctl_col - 1)];
      if (ctl_row > 0 && ctl_col > 0) rock_count += p->pref_rock[(ctl_row - 1) * p->N + (ctl_col - 1)];

      if (rock_count >= p->S) {
        // Calculate the position of top-left corner
        size_t tl_row = i - p->K + 1;
        size_t tl_col = j - p->K + 1;
        // Calculate the number of forests
        size_t forest_count = p->pref_forest[i * p->N + j];
        if (tl_row > 0) forest_count -= p->pref_forest[(tl_row - 1) * p->N + j];
        if (tl_col > 0) forest_count -= p->pref_forest[i * p->N + (tl_col - 1)];
        if (tl_row > 0 && tl_col > 0) forest_count += p->pref_forest[(tl_row - 1) * p->N + (tl_col - 1)];
        if (forest_count > max_forests) max_forests = forest_count;
      }
    }
  }

  return max_forests;
}
