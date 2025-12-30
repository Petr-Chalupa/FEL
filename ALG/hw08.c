#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// ----- Defines -----

// Ranges are inclusive
#define N_MIN 1
#define N_MAX 800
#define P_MIN 1
#define P_MAX 10
#define L_MIN 1
#define L_MAX 10
#define WARRIORS_MIN 1
#define WARRIORS_MAX 20

// ----- Typedefs -----

typedef enum { E_MEMALLOC = 100, E_INVALIDINP = 101, E_INPOUTOFRANGE = 102 } Error;

typedef struct {
  int profit;   // Maximum profit for this interval
  int warriors; // Total warriors in merged villages
} DPState;

typedef struct {
  int N, P, L;
  int *villages;
  DPState *dp;
  int *best;
} River;

// ----- Global vars -----

River *river = NULL;

// ----- Declarations -----

void Error__throw(Error e);
//
River *River__init(int N, int P, int L);
void River__free(River *r);
void River__read(River *r);
void River__solve(River *r);

// ----- Definitions -----

int main() {
  int N, P, L;
  if (scanf("%d %d %d\n", &N, &P, &L) != 3) Error__throw(E_INVALIDINP);
  if (N < N_MIN || N > N_MAX || P < P_MIN || P > P_MAX || L < L_MIN || L > L_MAX) Error__throw(E_INPOUTOFRANGE);

  river = River__init(N, P, L);
  River__read(river);
  River__solve(river);

  printf("%d\n", river->best[river->N - 1]);

  River__free(river);

  return 0;
}

void Error__throw(Error e) {
  switch (e) {
  case E_MEMALLOC: fprintf(stderr, "Error: Memory allocation!\n"); break;
  case E_INVALIDINP: fprintf(stderr, "Error: Invalid argument!\n"); break;
  case E_INPOUTOFRANGE: fprintf(stderr, "Error: Input is out of range!\n"); break;
  }

  River__free(river);
  exit(e);
}

River *River__init(int N, int P, int L) {
  River *r = malloc(sizeof(River));
  if (!r) Error__throw(E_MEMALLOC);

  r->N = N;
  r->P = P;
  r->L = L;
  r->villages = calloc(N, sizeof(int));
  r->dp = malloc(N * N * sizeof(DPState));
  r->best = calloc(N, sizeof(int));

  if (!r->villages || !r->dp || !r->best) {
    River__free(r);
    Error__throw(E_MEMALLOC);
  }

  return r;
}

void River__free(River *r) {
  if (!r) return;

  if (r->villages) free(r->villages);
  if (r->dp) free(r->dp);
  if (r->best) free(r->best);

  free(r);
}

void River__read(River *r) {
  for (int i = 0; i < r->N; i++) {
    int n;
    if (scanf("%d", &n) != 1) Error__throw(E_INVALIDINP);
    if (n < WARRIORS_MIN || n > WARRIORS_MAX) Error__throw(E_INPOUTOFRANGE);

    r->villages[i] = n;
  }
}

void River__solve(River *r) {
  int N = r->N;
  int P = r->P;
  int L = r->L;

  // Initialize DP states
  for (int i = 0; i < N; i++) {
    r->dp[i * N + i].profit = 0;
    r->dp[i * N + i].warriors = r->villages[i];
  }

  for (int len = 2; len <= N; len++) {
    for (int i = 0; i + len - 1 < N; i++) {
      int j = i + len - 1;

      r->dp[i * N + j].warriors = r->dp[i * N + (j - 1)].warriors + r->villages[j];
      r->dp[i * N + j].profit = INT_MIN;

      for (int k = i; k < j; k++) {
        int left_warriors = r->dp[i * N + k].warriors;
        int right_warriors = r->dp[(k + 1) * N + j].warriors;
        int cost = L * abs(left_warriors - right_warriors);

        int candidate = r->dp[i * N + k].profit + r->dp[(k + 1) * N + j].profit + P - cost;
        if (candidate > r->dp[i * N + j].profit) {
          r->dp[i * N + j].profit = candidate;
        }
      }
    }
  }

  for (int i = 0; i < N; i++) {
    for (int j = 0; j <= i; j++) {
      int profit_from_merge = r->dp[j * N + i].profit;
      int profit_before = (j > 0) ? r->best[j - 1] : 0;
      int total = profit_before + profit_from_merge;

      if (total > r->best[i]) {
        r->best[i] = total;
      }
    }
  }
}