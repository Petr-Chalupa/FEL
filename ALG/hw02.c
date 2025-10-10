#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define N_MIN 0
#define N_MAX 30
#define M_MIN 0
#define M_MAX 180
#define A_MIN 1
#define B_MIN 1

typedef enum {
  ERR_INVALID_INPUT = 100,
  ERR_MEM_ALLOC = 101,
} Error;

typedef enum {
  NONE = 0,
  TYPE_A = 1, // Score is defined as the sum of occupied neighbours
  TYPE_B = 2  // Score is defined as the sum of unoccupied neighbours
} AgentType;

typedef struct {
  size_t N;
  size_t M;
  size_t A;
  size_t B;
  bool *edges;
  AgentType *nodes;
  size_t *degrees;
  size_t *nodes_order;
} Graph;

void Error__throw(Error e);
Graph *Graph__init(size_t N, size_t M, size_t A, size_t B);
Graph *Graph__read(FILE *fp);
size_t Graph__solve(Graph *g);
void Graph__solve_backtrack(Graph *g, size_t agents_A, size_t agents_B, size_t current_score, size_t *best_score, size_t order_idx);
int Graph__node_gain(Graph *g, size_t node);
size_t Graph__score_upper_bound(Graph *g, size_t agents_A, size_t agents_B, size_t order_idx);
void Graph__free(Graph *g);

int main() {
  Graph *p = Graph__read(stdin);

  size_t solution = Graph__solve(p);
  printf("%zu\n", solution);

  Graph__free(p);

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

Graph *Graph__init(size_t N, size_t M, size_t A, size_t B) {
  Graph *g = malloc(sizeof(Graph));
  if (g == NULL) {
    Error__throw(ERR_MEM_ALLOC);
  }

  g->N = N;
  g->M = M;
  g->A = A;
  g->B = B;

  g->edges = calloc(N * N, sizeof(*g->edges)); // Init as false
  g->nodes = calloc(N, sizeof(*g->nodes));     // Init as NONE
  g->degrees = calloc(N, sizeof(*g->degrees)); // Init as 0
  g->nodes_order = malloc(N * sizeof(*g->nodes_order));
  if (!g->edges || !g->nodes || !g->degrees || !g->nodes_order) {
    Graph__free(g);
    Error__throw(ERR_MEM_ALLOC);
  }

  return g;
}

void Graph__free(Graph *g) {
  free(g->edges);
  free(g->nodes);
  free(g->degrees);
  free(g->nodes_order);
  free(g);
}

Graph *Graph__read(FILE *fp) {
  // Read graph dimensions
  int n, m, a, b;
  if (fscanf(fp, "%d %d %d %d", &n, &m, &a, &b) != 4) {
    Error__throw(ERR_INVALID_INPUT);
  }
  if (n < N_MIN || n > N_MAX || m < M_MIN || m > M_MAX || a < A_MIN || b < B_MIN || a + b > n) {
    Error__throw(ERR_INVALID_INPUT);
  }

  size_t N = (size_t)n;
  size_t M = (size_t)m;
  size_t A = (size_t)a;
  size_t B = (size_t)b;
  Graph *g = Graph__init(N, M, A, B);

  // Fill the graph edges
  for (size_t i = 0; i < M; i++) {
    int u, v;
    if (fscanf(fp, "%d %d", &u, &v) != 2 || (u < 1 || u > (int)N) || (v < 1 || v > (int)N)) {
      Graph__free(g);
      Error__throw(ERR_INVALID_INPUT);
    }

    u--;
    v--;
    g->edges[u * N + v] = true;
    g->edges[v * N + u] = true;
    g->degrees[u]++;
    g->degrees[v]++;
  }

  // Sort nodes by degree (descending)
  for (size_t i = 0; i < N; i++) {
    size_t j = i;
    while (j > 0 && g->degrees[g->nodes_order[j - 1]] < g->degrees[i]) {
      g->nodes_order[j] = g->nodes_order[j - 1];
      j--;
    }
    g->nodes_order[j] = i;
  }

  return g;
}

size_t Graph__solve(Graph *g) {
  size_t score = 0;

  Graph__solve_backtrack(g, g->A, g->B, 0, &score, 0);

  return score;
}

void Graph__solve_backtrack(Graph *g, size_t agents_A, size_t agents_B, size_t current_score, size_t *best_score, size_t order_idx) {
  if (agents_A + agents_B == 0) {
    if (current_score > *best_score) {
      *best_score = current_score;
    }
    return;
  }

  // Not enough nodes - pruning
  if (g->N - order_idx < agents_A + agents_B) return;

  // Score upper bound - pruning
  size_t max_possible_score = current_score + Graph__score_upper_bound(g, agents_A, agents_B, order_idx);
  if (max_possible_score <= *best_score) return;

  // Placement branching
  size_t node = g->nodes_order[order_idx];
  if (g->nodes[node] == NONE) {
    // Try TYPE_A
    if (agents_A > 0) {
      g->nodes[node] = TYPE_A;
      int gain = Graph__node_gain(g, node);
      Graph__solve_backtrack(g, agents_A - 1, agents_B, current_score + gain, best_score, order_idx + 1);
      g->nodes[node] = NONE;
    }

    // Try TYPE_B
    if (agents_B > 0) {
      g->nodes[node] = TYPE_B;
      int gain = Graph__node_gain(g, node);
      Graph__solve_backtrack(g, agents_A, agents_B - 1, current_score + gain, best_score, order_idx + 1);
      g->nodes[node] = NONE;
    }
  }
  // Try NONE
  Graph__solve_backtrack(g, agents_A, agents_B, current_score, best_score, order_idx + 1);
}

int Graph__node_gain(Graph *g, size_t node) {
  AgentType type = g->nodes[node];
  int gain = 0;

  for (size_t i = 0; i < g->N; i++) {
    if (!g->edges[node * g->N + i]) continue;

    if (type == TYPE_A && g->nodes[i] != NONE) {
      gain++;
    } else if (type == TYPE_B && g->nodes[i] == NONE) {
      gain++;
    }
    if (g->nodes[i] == TYPE_A) gain++; // Already occupied neighbour of TYPE_A gains 1
    if (g->nodes[i] == TYPE_B) gain--; // Already occupied neighbour of TYPE_B loses 1
  }

  return gain;
}

size_t Graph__score_upper_bound(Graph *g, size_t agents_A, size_t agents_B, size_t order_idx) {
  if (agents_A == 0 && agents_B == 0) return 0;

  size_t remaining = agents_A + agents_B;
  size_t sum_degrees = 0;

  for (size_t i = order_idx; i < g->N && remaining > 0; i++) {
    size_t node = g->nodes_order[i];
    if (g->nodes[node] != NONE) continue;

    sum_degrees += g->degrees[node];
    remaining--;
  }

  return (2 * agents_A + agents_B) * ((sum_degrees + agents_A + agents_B - 1) / (agents_A + agents_B));
}
