#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// ----- Defines -----

// Ranges are inclusive
#define R_MIN 3
#define R_MAX 10
#define C_MIN 3
#define C_MAX 10

// ----- Typedefs -----

typedef enum { E_MEMALLOC = 100, E_INVALIDINP = 101, E_INPOUTOFRANGE = 102 } Error;

typedef enum { BLUE = 1, GREEN = 2, ORANGE = 3 } Color;

typedef struct {
  Color *data;
  int R;
  int C;
  int score;
} Matrix;

typedef struct {
  int u_row, u_col;
  int v_row, v_col;
  int score;
} Segment;

typedef struct {
  Segment *data;
  int size;
  int capacity;
} SegmentList;

// ----- Global vars -----

Matrix *matrix = NULL;
SegmentList *segments = NULL;
int max_score = 0;

// ----- Declarations -----

void Error__throw(Error e);
void cleanup();
//
Matrix *Matrix__init(int R, int C);
void Matrix__free(Matrix *m);
void Matrix__read(Matrix *m);
void Matrix__solve();
void Matrix__solve_backtrack(SegmentList *sl, int idx, int *removed, int removed_count);
//
SegmentList *SegmentList__init();
void SegmentList__free(SegmentList *sl);
void SegmentList__add(SegmentList *sl, Segment seg);
void SegmentList__findAll(Matrix *m, SegmentList *sl);
int SegmentList__getScore(SegmentList *sl, int *removed, int removed_count);
int SegmentList__scoreUpperBound(SegmentList *sl, int start_idx, int *used, int used_count);
//
int Segment__overlaps(Segment *s1, Segment *s2);
int Segment__canAdd(SegmentList *sl, int seg_idx, int *removed, int removed_count);
int Segment__cmp(const void *a, const void *b);
int SegmentList__scoreUpperBound(SegmentList *sl, int start_idx, int *used, int used_count);

// ----- Definitions -----

int main() {
  int R, C;
  if (scanf("%d %d\n", &R, &C) != 2) Error__throw(E_INVALIDINP);
  if (R < R_MIN || R > R_MAX || C < C_MIN || C > C_MAX) Error__throw(E_INPOUTOFRANGE);

  matrix = Matrix__init(R, C);
  Matrix__read(matrix);

  segments = SegmentList__init();
  if (!segments) {
    Error__throw(E_MEMALLOC);
  }

  SegmentList__findAll(matrix, segments);
  Matrix__solve(matrix);

  printf("%d\n", max_score);

  cleanup();
  return 0;
}

void Error__throw(Error e) {
  switch (e) {
  case E_MEMALLOC: fprintf(stderr, "Error: Memory allocation!\n"); break;
  case E_INVALIDINP: fprintf(stderr, "Error: Invalid argument!\n"); break;
  case E_INPOUTOFRANGE: fprintf(stderr, "Error: Input is out of range!\n"); break;
  }

  cleanup();
  exit(e);
}

void cleanup() {
  if (matrix != NULL) {
    Matrix__free(matrix);
    matrix = NULL;
  }
  if (segments != NULL) {
    SegmentList__free(segments);
    segments = NULL;
  }
}

Matrix *Matrix__init(int R, int C) {
  Matrix *m = malloc(sizeof(Matrix));
  if (!m) Error__throw(E_MEMALLOC);

  m->R = R;
  m->C = C;
  m->score = 0;
  m->data = malloc(R * C * sizeof(Color));

  if (!m->data) {
    Matrix__free(m);
    Error__throw(E_MEMALLOC);
  }

  return m;
}

void Matrix__free(Matrix *m) {
  if (!m) return;

  if (m->data) free(m->data);
  free(m);
}

void Matrix__read(Matrix *m) {
  for (int i = 0; i < m->R * m->C; i++) {
    int n;
    if (scanf("%d", &n) != 1) Error__throw(E_INVALIDINP);
    if (n != BLUE && n != GREEN && n != ORANGE) Error__throw(E_INPOUTOFRANGE);

    m->data[i] = n;
  }
}

void Matrix__solve() {
  int removed[1000] = {0};
  Matrix__solve_backtrack(segments, 0, removed, 0);
}

void Matrix__solve_backtrack(SegmentList *sl, int idx, int *removed, int removed_count) {
  int score = SegmentList__getScore(sl, removed, removed_count);
  if (score > max_score) {
    max_score = score;
  }

  if (idx >= sl->size) {
    return;
  }

  int upper_bound = SegmentList__scoreUpperBound(sl, idx, removed, removed_count);
  if (score + upper_bound <= max_score) {
    return;
  }

  Matrix__solve_backtrack(sl, idx + 1, removed, removed_count);

  if (Segment__canAdd(sl, idx, removed, removed_count)) {
    removed[removed_count] = idx;
    Matrix__solve_backtrack(sl, idx + 1, removed, removed_count + 1);
  }
}

SegmentList *SegmentList__init() {
  SegmentList *sl = malloc(sizeof(SegmentList));
  if (!sl) Error__throw(E_MEMALLOC);

  sl->data = NULL;
  sl->size = 0;
  sl->capacity = 0;

  return sl;
}

void SegmentList__free(SegmentList *sl) {
  if (!sl) return;

  if (sl->data) free(sl->data);
  free(sl);
}

void SegmentList__add(SegmentList *sl, Segment seg) {
  if (sl->size == sl->capacity) {
    sl->capacity = sl->capacity ? sl->capacity * 2 : 4;
    sl->data = realloc(sl->data, sl->capacity * sizeof(Segment));

    if (!sl->data) Error__throw(E_MEMALLOC);
  }

  sl->data[sl->size++] = seg;
}

void SegmentList__findAll(Matrix *m, SegmentList *sl) {
  // Horizontal
  for (int r = 0; r < m->R; r++) {
    for (int c = 0; c + 2 < m->C; c++) {
      Color c1 = m->data[r * m->C + c];
      Color c2 = m->data[r * m->C + c + 1];
      Color c3 = m->data[r * m->C + c + 2];

      int score = 0;
      if (c1 == c2 && c2 == c3) {
        score = 3;
      } else if (c1 == c2 || c2 == c3 || c1 == c3) {
        score = 1;
      }

      Segment seg = {.u_row = r, .u_col = c, .v_row = r, .v_col = c + 2, .score = score};
      SegmentList__add(sl, seg);
    }
  }

  // Vertical
  for (int c = 0; c < m->C; c++) {
    for (int r = 0; r + 2 < m->R; r++) {
      Color c1 = m->data[r * m->C + c];
      Color c2 = m->data[(r + 1) * m->C + c];
      Color c3 = m->data[(r + 2) * m->C + c];

      int score = 0;
      if (c1 == c2 && c2 == c3) {
        score = 3;
      } else if (c1 == c2 || c2 == c3 || c1 == c3) {
        score = 1;
      }

      Segment seg = {.u_row = r, .u_col = c, .v_row = r + 2, .v_col = c, .score = score};
      SegmentList__add(sl, seg);
    }
  }

  // Sort in descending order
  qsort(sl->data, sl->size, sizeof(Segment), Segment__cmp);
}

int SegmentList__getScore(SegmentList *sl, int *removed, int removed_count) {
  int total_score = 0;
  for (int i = 0; i < removed_count; i++) {
    total_score += sl->data[removed[i]].score;
  }
  return total_score;
}

int SegmentList__scoreUpperBound(SegmentList *sl, int start_idx, int *used, int used_count) {
  int additional_score = 0;

  for (int i = start_idx; i < sl->size; i++) {
    int can_add = 1;
    for (int j = 0; j < used_count; j++) {
      if (Segment__overlaps(&sl->data[i], &sl->data[used[j]])) {
        can_add = 0;
        break;
      }
    }

    if (can_add) {
      additional_score += sl->data[i].score;
    }
  }

  return additional_score;
}

int Segment__overlaps(Segment *s1, Segment *s2) {
  int s1_min_row = s1->u_row;
  int s1_max_row = s1->v_row;
  int s1_min_col = s1->u_col;
  int s1_max_col = s1->v_col;

  int s2_min_row = s2->u_row;
  int s2_max_row = s2->v_row;
  int s2_min_col = s2->u_col;
  int s2_max_col = s2->v_col;

  if (s1_min_row <= s2_max_row && s1_max_row >= s2_min_row && s1_min_col <= s2_max_col && s1_max_col >= s2_min_col) {
    return 1;
  }
  return 0;
}

int Segment__canAdd(SegmentList *sl, int seg_idx, int *removed, int removed_count) {
  for (int i = 0; i < removed_count; i++) {
    if (Segment__overlaps(&sl->data[seg_idx], &sl->data[removed[i]])) {
      return 0;
    }
  }
  return 1;
}

int Segment__cmp(const void *a, const void *b) {
  const Segment *s1 = (const Segment *)a;
  const Segment *s2 = (const Segment *)b;
  return s2->score - s1->score;
}
