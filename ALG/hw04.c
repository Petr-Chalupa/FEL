#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

// ----- Defines -----

// Ranges are inclusive
#define N_MIN 10
#define N_MAX 200000
#define H_MIN 1
#define M_MIN 1
#define M_MAX 50
#define ExM_MIN 20
#define ExM_MAX 10000000

// ----- Typedefs -----

typedef enum { E_MEMALLOC = 100, E_INVALIDINP = 101, E_INPOUTOFRANGE = 102, E_QOVERFLOW = 103 } Error;

typedef struct {
  int N;
  int H;
  int E;
  int *degree;
  int **adj;
  char *is_noisy;
} Graph;

typedef struct {
  int node;
  int dist;
  int noisy_used;
} State;

typedef struct {
  State *data;
  int head;
  int tail;
  int capacity;
} Queue;

typedef struct {
  State *states;
  int size;
  int capacity;
} NodeStates;

// ----- Global vars -----

Graph *graph = NULL;
Queue *queue = NULL;
int N, H, E;                    // Number of nodes, number of noisy nodes, number of edges
int S, C, K, M;                 // Start node of mouse, end node of mouse, start node of cat, max jumps of cat
int *dist_cat = NULL;           // [node] => min dist from node to the start node of cat
NodeStates *node_states = NULL; // [node] => reachable states for this node

// ----- Declarations -----

void Error__throw(Error e);
void cleanup();
//
Graph *Graph__init(int N, int H, int E);
void Graph__free(Graph *g);
void Graph__load(Graph *g);
//
Queue *Queue__init(int capacity);
void Queue__free(Queue *q);
void Queue__clear(Queue *q);
int Queue__empty(Queue *q);
void Queue__push(Queue *q, State s);
State Queue__pop(Queue *q);
//
NodeStates *NodeStates__init(int N);
void NodeStates__free(NodeStates *ns, int N);
int NodeStates__update(NodeStates *ns, State s);
//
void BFS_cat(Graph *g, int K, int *dist_cat);
void BFS_mouse(Graph *g, int S, int C, int M, int *dist_cat, NodeStates *node_states);

// ----- Definitions -----

int main() {
  // Read input arguments
  if (scanf("%d %d %d\n%d %d %d %d\n", &N, &H, &E, &S, &C, &K, &M) != 7) {
    Error__throw(E_INVALIDINP);
  }
  if (S == C || S == K || C == K) {
    Error__throw(E_INVALIDINP);
  }
  if (!(N_MIN <= N && N <= N_MAX && H_MIN <= H && H < N - 3 && M_MIN <= M && M <= M_MAX && ExM_MIN <= E * M && E * M <= ExM_MAX)) {
    Error__throw(E_INPOUTOFRANGE);
  }

  // Allocate global vars
  graph = Graph__init(N, H, E);
  Graph__load(graph);

  queue = Queue__init(N * (M + 1));

  dist_cat = malloc(N * sizeof(int));
  if (dist_cat == NULL) {
    Error__throw(E_MEMALLOC);
  }

  node_states = NodeStates__init(N);
  if (!node_states) {
    Error__throw(E_MEMALLOC);
  }

  // Precompute nodes dist from cat
  BFS_cat(graph, K, dist_cat);

  // Compute shortest safe paths
  BFS_mouse(graph, S, C, M, dist_cat, node_states);

  // Find and print optimal path
  int opt_path_length = INT_MAX;
  int opt_path_noisy = INT_MAX;
  for (int i = 0; i < node_states[C].size; i++) {
    State s = node_states[C].states[i];
    if (s.dist < opt_path_length || (s.dist == opt_path_length && s.noisy_used < opt_path_noisy)) {
      opt_path_length = s.dist;
      opt_path_noisy = s.noisy_used;
    }
  }
  printf("%d %d\n", opt_path_length, opt_path_noisy);

  cleanup();
  return 0;
}

void Error__throw(Error e) {
  switch (e) {
  case E_MEMALLOC:
    fprintf(stderr, "Error: Memory allocation!\n");
    break;
  case E_INVALIDINP:
    fprintf(stderr, "Error: Invalid argument!\n");
    break;
  case E_INPOUTOFRANGE:
    fprintf(stderr, "Error: Input is out of range!\n");
    break;
  case E_QOVERFLOW:
    fprintf(stderr, "Error: Pushing to a full queue!\n");
    break;
  }

  cleanup();

  exit(e);
}

void cleanup() {
  if (graph != NULL) {
    Graph__free(graph);
    graph = NULL;
  }

  if (queue != NULL) {
    Queue__free(queue);
    queue = NULL;
  }

  if (dist_cat != NULL) {
    free(dist_cat);
    dist_cat = NULL;
  }

  if (node_states != NULL) {
    NodeStates__free(node_states, N);
    node_states = NULL;
  }
}

Graph *Graph__init(int N, int H, int E) {
  Graph *g = malloc(sizeof(Graph));
  if (g == NULL) {
    Error__throw(E_MEMALLOC);
  }

  g->N = N;
  g->H = H;
  g->E = E;
  g->degree = NULL;
  g->adj = NULL;
  g->is_noisy = NULL;

  g->degree = calloc(N, sizeof(int));
  if (g->degree == NULL) {
    Graph__free(g);
    Error__throw(E_MEMALLOC);
  }

  g->adj = malloc(N * sizeof(int *));
  if (g->adj == NULL) {
    Graph__free(g);
    Error__throw(E_MEMALLOC);
  } else {
    for (int i = 0; i < N; i++) {
      g->adj[i] = NULL;
    }
  }

  g->is_noisy = calloc(N, sizeof(char));
  if (g->is_noisy == NULL) {
    Graph__free(g);
    Error__throw(E_MEMALLOC);
  }

  return g;
}

void Graph__free(Graph *g) {
  if (g == NULL) return;

  if (g->degree != NULL) {
    free(g->degree);
  }

  if (g->adj != NULL) {
    for (int i = 0; i < g->N; i++) {
      if (g->adj[i] != NULL) free(g->adj[i]);
    }
    free(g->adj);
  }

  if (g->is_noisy != NULL) {
    free(g->is_noisy);
  }

  free(g);
}

void Graph__load(Graph *g) {
  // Load noisy nodes
  for (int i = 0; i < g->H; i++) {
    int noisy_idx;
    if (scanf("%d", &noisy_idx) != 1) {
      Error__throw(E_INVALIDINP);
    }

    g->is_noisy[noisy_idx] = 1;
  }

  // Temporary arrays for edges
  int *edge_u = malloc(g->E * sizeof(int));
  int *edge_v = malloc(g->E * sizeof(int));
  if (edge_u == NULL || edge_v == NULL) {
    free(edge_u);
    free(edge_v);
    Graph__free(g);
    Error__throw(E_MEMALLOC);
  }

  // Read edges and count degrees
  for (int i = 0; i < g->E; i++) {
    if (scanf("%d %d", &edge_u[i], &edge_v[i]) != 2) {
      free(edge_u);
      free(edge_v);
      Graph__free(g);
      Error__throw(E_INVALIDINP);
    }

    g->degree[edge_u[i]]++;
    g->degree[edge_v[i]]++;
  }

  // Allocate adjacency lists
  for (int i = 0; i < g->N; i++) {
    g->adj[i] = malloc(g->degree[i] * sizeof(int));
    if (g->adj[i] == NULL) {
      free(edge_u);
      free(edge_v);
      Graph__free(g);
      Error__throw(E_MEMALLOC);
    }
    g->degree[i] = 0; // Reuse degree[] as write cursor
  }

  // Fill adjacency lists
  for (int i = 0; i < g->E; i++) {
    int u = edge_u[i];
    int v = edge_v[i];

    g->adj[u][g->degree[u]++] = v;
    g->adj[v][g->degree[v]++] = u;
  }

  free(edge_u);
  free(edge_v);
}

Queue *Queue__init(int capacity) {
  Queue *q = malloc(sizeof(Queue));
  if (q == NULL) {
    Error__throw(E_MEMALLOC);
  }

  q->data = NULL;
  q->head = 0;
  q->tail = 0;
  q->capacity = capacity;

  q->data = malloc(capacity * sizeof(State));
  if (q->data == NULL) {
    free(q);
    Error__throw(E_MEMALLOC);
  }

  return q;
}

void Queue__free(Queue *q) {
  if (q == NULL) return;

  free(q->data);
  free(q);
}

void Queue__clear(Queue *q) {
  q->head = 0;
  q->tail = 0;
}

int Queue__empty(Queue *q) { return q->head == q->tail; }

void Queue__push(Queue *q, State s) {
  int next = (q->tail + 1) % q->capacity;

  if (next == q->head) {
    Error__throw(E_QOVERFLOW);
  }

  q->data[q->tail] = s;
  q->tail = next;
}

State Queue__pop(Queue *q) {
  if (q->head == q->tail) {
    State empty = {.node = -1, .noisy_used = 0};
    return empty;
  }

  State s = q->data[q->head];
  q->head = (q->head + 1) % q->capacity;
  return s;
}

NodeStates *NodeStates__init(int N) {
  NodeStates *ns = malloc(N * sizeof(NodeStates));
  if (!ns) {
    Error__throw(E_MEMALLOC);
  }

  for (int i = 0; i < N; i++) {
    ns[i].states = NULL;
    ns[i].size = 0;
    ns[i].capacity = 0;
  }

  return ns;
}

void NodeStates__free(NodeStates *ns, int N) {
  if (!ns) return;

  for (int i = 0; i < N; i++) {
    free(ns[i].states);
  }
  free(ns);
}

int NodeStates__update(NodeStates *ns, State s) {
  // Update existing state with same noisy_used
  for (int i = 0; i < ns->size; i++) {
    State *old = &ns->states[i];
    if (old->noisy_used <= s.noisy_used && old->dist <= s.dist) {
      return 0;
    }
    if (ns->states[i].noisy_used == s.noisy_used) {
      if (s.dist < ns->states[i].dist) {
        ns->states[i].dist = s.dist;
        return 1;
      }
      return 0;
    }
  }

  // Add new state
  if (ns->size == ns->capacity) {
    ns->capacity = ns->capacity ? ns->capacity * 2 : 4;
    ns->states = realloc(ns->states, ns->capacity * sizeof(State));
    if (!ns->states) {
      Error__throw(E_MEMALLOC);
    }
  }

  ns->states[ns->size++] = s;
  return 1;
}

void BFS_cat(Graph *g, int K, int *dist_cat) {
  // Initialize
  for (int i = 0; i < g->N; i++) {
    dist_cat[i] = INT_MAX;
  }

  dist_cat[K] = 0;

  Queue__clear(queue);
  Queue__push(queue, (State){.node = K});

  // Compute
  while (!Queue__empty(queue)) {
    State s = Queue__pop(queue);
    int u = s.node;
    int du = dist_cat[u];

    for (int k = 0; k < g->degree[u]; k++) {
      int v = g->adj[u][k];

      if (dist_cat[v] == INT_MAX) {
        dist_cat[v] = du + 1;
        Queue__push(queue, (State){.node = v});
      }
    }
  }
}

void BFS_mouse(Graph *g, int S, int C, int M, int *dist_cat, NodeStates *node_states) {
  // Initialize
  State start_state = (State){.node = S, .noisy_used = 0, .dist = 0};
  Queue__clear(queue);
  Queue__push(queue, start_state);
  NodeStates__update(&node_states[S], start_state);

  // Compute
  while (!Queue__empty(queue)) {
    State s = Queue__pop(queue);
    int u = s.node;

    if (u == C) continue;

    for (int k = 0; k < g->degree[u]; k++) {
      int v = g->adj[u][k];
      int new_dist = s.dist + 1;
      int new_noisy = s.noisy_used + g->is_noisy[v];
      int cat_jumps_used = new_noisy <= M ? new_noisy : M;

      if (new_noisy >= g->H) {
        continue; // Noisy nodes out of range
      }

      if (dist_cat[v] <= cat_jumps_used) {
        continue; // Reachable by cat
      }

      State new_state = (State){.node = v, .noisy_used = new_noisy, .dist = new_dist};
      if (NodeStates__update(&node_states[v], new_state)) {
        Queue__push(queue, new_state);
      }
    }
  }
}
