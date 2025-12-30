#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// ----- Defines -----

// Ranges are inclusive
#define M_MIN 5
#define M_MAX 200
#define N_MIN 5
#define N_MAX 200
#define VALVES_MIN 0
#define VALVES_MAX 100

// ----- Typedefs -----

typedef enum { E_MEMALLOC = 100, E_INVALIDINP = 101, E_INPOUTOFRANGE = 102 } Error;

typedef enum { HORIZONTAL = 0, VERTICAL = 1 } Direction;

typedef struct {
  int score;
  int last_weight;
} DPState;

typedef struct {
  int M, N;
  int *grid;
  int *pref_row;
  int *pref_col;
  DPState *bestH; // Best score arriving horizontally with weight w
  DPState *bestV; // Best score arriving vertically with weight w
} Field;

typedef struct {
  int u_row, u_col;
  int v_row, v_col;
  int w;
  Direction dir;
} Segment;

// ----- Global vars -----

Field *field = NULL;

// ----- Declarations -----

void Error__throw(Error e);
//
Field *Field__init(int M, int N);
void Field__free(Field *f);
void Field__read(Field *f);
int Field__sum(Field *f, int row_from, int col_from, int row_to, int col_to);
void Field__solve(Field *f);
//
int Segment__cmp(const void *a, const void *b);

// ----- Definitions -----

int main() {
  int M, N;
  if (scanf("%d %d\n", &M, &N) != 2) Error__throw(E_INVALIDINP);
  if (M < M_MIN || M > M_MAX || N < N_MIN || N > N_MAX) Error__throw(E_INPOUTOFRANGE);

  field = Field__init(M, N);
  Field__read(field);
  Field__solve(field);

  int endIdx = (field->M - 1) * field->N + field->N - 1;
  int bestH = field->bestH[endIdx].score;
  int bestV = field->bestV[endIdx].score;
  int ans = bestH > bestV ? bestH : bestV;
  printf("%d\n", ans);

  Field__free(field);

  return 0;
}

void Error__throw(Error e) {
  switch (e) {
  case E_MEMALLOC: fprintf(stderr, "Error: Memory allocation!\n"); break;
  case E_INVALIDINP: fprintf(stderr, "Error: Invalid argument!\n"); break;
  case E_INPOUTOFRANGE: fprintf(stderr, "Error: Input is out of range!\n"); break;
  }

  Field__free(field);
  exit(e);
}

Field *Field__init(int M, int N) {
  Field *f = malloc(sizeof(Field));
  if (!f) Error__throw(E_MEMALLOC);

  f->M = M;
  f->N = N;
  f->grid = calloc(M * N, sizeof(int));
  f->pref_row = calloc(M * N, sizeof(int));
  f->pref_col = calloc(M * N, sizeof(int));
  f->bestH = malloc(M * N * sizeof(DPState));
  f->bestV = malloc(M * N * sizeof(DPState));

  if (!f->grid || !f->pref_col || !f->pref_row || !f->bestH || !f->bestV) {
    Field__free(f);
    Error__throw(E_MEMALLOC);
  }

  // Initialize DP states
  for (int i = 0; i < M * N; i++) {
    f->bestH[i].score = f->bestV[i].score = -1;
    f->bestH[i].last_weight = f->bestV[i].last_weight = INT_MAX;
  }
  f->bestH[0].score = f->bestV[0].score = 0;

  return f;
}

void Field__free(Field *f) {
  if (!f) return;

  if (f->grid) free(f->grid);
  if (f->pref_col) free(f->pref_col);
  if (f->pref_row) free(f->pref_row);
  if (f->bestH) free(f->bestH);
  if (f->bestV) free(f->bestV);

  free(f);
}

void Field__read(Field *f) {
  for (int i = 0; i < f->M; i++) {
    for (int j = 0; j < f->N; j++) {
      int n;
      if (scanf("%d", &n) != 1) Error__throw(E_INVALIDINP);
      if (n < VALVES_MIN || n > VALVES_MAX) Error__throw(E_INPOUTOFRANGE);

      int idx = i * f->N + j;
      f->grid[idx] = n;
      f->pref_row[idx] = j == 0 ? n : f->pref_row[idx - 1] + n;
      f->pref_col[idx] = i == 0 ? n : f->pref_col[(i - 1) * f->N + j] + n;
    }
  }
}

int Field__sum(Field *f, int row_from, int col_from, int row_to, int col_to) {
  if (row_from == row_to) {
    if (col_to - col_from < 2) return 0;
    return f->pref_row[row_from * f->N + col_to - 1] - f->pref_row[row_from * f->N + col_from];
  } else {
    if (row_to - row_from < 2) return 0;
    return f->pref_col[(row_to - 1) * f->N + col_from] - f->pref_col[row_from * f->N + col_from];
  }
}

void Field__solve(Field *f) {
  int M = f->M;
  int N = f->N;

  // Allocate space for segments
  int max_segments = M * N * (M + N - 2) / 2;
  Segment *segments = malloc(max_segments * sizeof(Segment));
  if (!segments) {
    Error__throw(E_MEMALLOC);
  }

  // Generate segments
  int seg_count = 0;
  for (int r = 0; r < M; r++) {
    for (int c = 0; c < N; c++) {
      // Horizontal
      for (int c2 = c + 1; c2 < N; c2++) {
        segments[seg_count].u_row = r;
        segments[seg_count].u_col = c;
        segments[seg_count].v_row = r;
        segments[seg_count].v_col = c2;
        segments[seg_count].w = Field__sum(f, r, c, r, c2);
        segments[seg_count].dir = HORIZONTAL;
        seg_count++;
      }
      // Vertical
      for (int r2 = r + 1; r2 < M; r2++) {
        segments[seg_count].u_row = r;
        segments[seg_count].u_col = c;
        segments[seg_count].v_row = r2;
        segments[seg_count].v_col = c;
        segments[seg_count].w = Field__sum(f, r, c, r2, c);
        segments[seg_count].dir = VERTICAL;
        seg_count++;
      }
    }
  }

  // Sort segments in descending order
  qsort(segments, seg_count, sizeof(Segment), Segment__cmp);

  // Build bestH and bestV
  for (int i = 0; i < seg_count; i++) {
    Segment seg = segments[i];
    int u_idx = seg.u_row * N + seg.u_col;
    int v_idx = seg.v_row * N + seg.v_col;

    if (seg.dir == HORIZONTAL) {
      if (f->bestV[u_idx].score >= 0 && seg.w < f->bestV[u_idx].last_weight) {
        int candidate = f->bestV[u_idx].score + seg.w;
        if (candidate > f->bestH[v_idx].score) {
          f->bestH[v_idx].score = candidate;
          f->bestH[v_idx].last_weight = seg.w;
        }
      }
    } else { // VERTICAL
      if (f->bestH[u_idx].score >= 0 && seg.w < f->bestH[u_idx].last_weight) {
        int candidate = f->bestH[u_idx].score + seg.w;
        if (candidate > f->bestV[v_idx].score) {
          f->bestV[v_idx].score = candidate;
          f->bestV[v_idx].last_weight = seg.w;
        }
      }
    }
  }

  free(segments);
}

int Segment__cmp(const void *a, const void *b) {
  const Segment *s1 = (const Segment *)a;
  const Segment *s2 = (const Segment *)b;
  return s2->w - s1->w;
}