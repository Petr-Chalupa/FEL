#include "dijkstra.h"
#include "graph.h"
#include "my_malloc.h"
#include "pq_heap.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  int edge_start; // index to the first edge in the array of edges (-1 if does not exist)
  int edge_count; // number of edges (may be 0)
  int parent;     // index to the parent node on the shortest path from the given node
  int cost;       // cost of the shortest path from the starting node to this node
} node_t;

typedef struct {
  graph_t *graph;
  node_t *nodes;
  int num_nodes;  // number of nodes;
  int start_node; //
} dijkstra_t;

void *dijkstra_init(void) {
  dijkstra_t *dij = myMalloc(sizeof(dijkstra_t));
  dij->nodes = NULL;
  dij->num_nodes = 0;
  dij->start_node = -1;
  dij->graph = allocate_graph();
  if (!dij->graph) {
    free(dij);
    dij = NULL;
  }
  return dij;
}

_Bool dijkstra_load_graph(const char *filename, void *dijkstra) {
  _Bool ret = false;
  dijkstra_t *dij = (dijkstra_t *)dijkstra;
  if (dij && dij->graph) {
    load_txt(filename, dij->graph);

    // 1st get the maximal number of nodes
    int m = -1;
    for (size_t i = 0; i < dij->graph->size; i += 3) {
      int *e = &dij->graph->edges[i];
      m = m < e[0] ? e[0] : m;
      m = m < e[1] ? e[1] : m;
    }
    m += 1; // m is the index therefore we need +1 for label 0
    dij->nodes = myMalloc(sizeof(node_t) * m);
    dij->num_nodes = m;

    // 2nd initialization of the nodes
    for (int i = 0; i < m; i++) {
      dij->nodes[i].edge_start = -1;
      dij->nodes[i].edge_count = 0;
      dij->nodes[i].parent = -1;
      dij->nodes[i].cost = -1;
    }

    // 3nd add edges to the nodes
    for (size_t i = 0; i < dij->graph->size; i += 3) {
      int cur = dij->graph->edges[i];
      if (dij->nodes[cur].edge_start == -1) { // first edge
        dij->nodes[cur].edge_start = i;       // mark the first edge in the array of edges
      }
      dij->nodes[cur].edge_count += 1; // increase number of edges
    }
    ret = true;
  }
  return ret;
}

_Bool dijkstra_set_graph(int e, int edges[][3], void *dijkstra) {
  dijkstra_t *dij = (dijkstra_t *)dijkstra;
  if (dij && dij->graph) {
    for (int i = 0; i < e; i++) {
      if (dij->graph->size + 1 > dij->graph->capacity) {
        enlarge_graph(dij->graph);
      }
      int *edge = dij->graph->edges + dij->graph->size;
      edge[0] = edges[i][0];
      edge[1] = edges[i][1];
      edge[2] = edges[i][2];
      dij->graph->size += 3;
    }

    // 1st get the maximal number of nodes
    int m = -1;
    for (size_t i = 0; i < dij->graph->size; i += 3) {
      int *e = &dij->graph->edges[i];
      m = m < e[0] ? e[0] : m;
      m = m < e[1] ? e[1] : m;
    }
    m += 1; // m is the index therefore we need +1 for label 0
    dij->nodes = myMalloc(sizeof(node_t) * m);
    dij->num_nodes = m;

    // 2nd initialization of the nodes
    for (int i = 0; i < m; i++) {
      dij->nodes[i].edge_start = -1;
      dij->nodes[i].edge_count = 0;
      dij->nodes[i].parent = -1;
      dij->nodes[i].cost = -1;
    }

    // 3nd add edges to the nodes
    for (size_t i = 0; i < dij->graph->size; i += 3) {
      int cur = dij->graph->edges[i];
      if (dij->nodes[cur].edge_start == -1) { // first edge
        dij->nodes[cur].edge_start = i;       // mark the first edge in the array of edges
      }
      dij->nodes[cur].edge_count += 1; // increase number of edges
    }

    return true;
  }
  return false;
}

_Bool dijkstra_solve(void *dijkstra, int label) {
  dijkstra_t *dij = (dijkstra_t *)dijkstra;
  if (!dij || label < 0 || label >= dij->num_nodes) {
    return false;
  }
  dij->start_node = label;

  void *pq = pq_alloc(dij->num_nodes);

  dij->nodes[label].cost = 0; // initialize the starting node
  pq_push(pq, label, 0);

  int cur_label;
  while (!pq_is_empty(pq) && pq_pop(pq, &cur_label)) {
    node_t *cur = &(dij->nodes[cur_label]);
    for (int i = 0; i < cur->edge_count; i++) { // relax all children
      int *edge = &dij->graph->edges[cur->edge_start + i * 3];
      node_t *to = &(dij->nodes[edge[1]]);
      const int cost = cur->cost + edge[2];
      if (to->cost == -1) { // the node to has not been visited yet
        to->cost = cost;
        to->parent = cur_label;
        pq_push(pq, edge[1], cost);
      } else if (cost <
                 to->cost) { // already relaxed check if we can make a shortcut to child node via the current node
        to->cost = cost;
        to->parent = cur_label;
        pq_update(pq, edge[1], cost);
      }
    } // end all children of the cur node;
  } // end pq_is_empty

  pq_free(pq); // release the memory

  return true;
}

_Bool dijkstra_get_solution(const void *dijkstra, int n, int solution[][3]) {
  const dijkstra_t *const dij = (dijkstra_t *)dijkstra;
  if (dij) {
    for (int i = 0; i < n; i++) {
      const node_t *const node = &(dij->nodes[i]);
      solution[i][0] = i;
      solution[i][1] = node->cost;
      solution[i][2] = node->parent;
    }
    return true;
  }
  return false;
}

_Bool dijkstra_save_path(const void *dijkstra, const char *filename) {
  _Bool ret = false;
  const dijkstra_t *const dij = (dijkstra_t *)dijkstra;
  if (dij) {
    FILE *f = fopen(filename, "w");
    if (f) {
      for (int i = 0; i < dij->num_nodes; i++) {
        const node_t *const node = &(dij->nodes[i]);
        fprintf(f, "%i %i %i\n", i, node->cost, node->parent);
      } // end all nodes
      ret = fclose(f) == 0;
    }
  }
  return ret;
}

void dijkstra_free(void *dijkstra) {
  dijkstra_t *dij = (dijkstra_t *)dijkstra;
  if (dij) {
    if (dij->graph) {
      free_graph(&(dij->graph));
    }
    if (dij->nodes) {
      free(dij->nodes);
    }
    free(dij);
  }
}
