#include <stdlib.h>

#ifndef __GRAPH_H__
#define __GRAPH_H__

typedef struct {
  int *edges; // 3 ints corresponds to 1 edge (start, end, cost)
  size_t size;
  size_t capacity;
} graph_t;

/* Allocate a new graph and return a reference to it. */
graph_t *allocate_graph();
/* Enlarge current graph */
void enlarge_graph(graph_t *graph);
/* Free all allocated memory and set reference to the graph to NULL. */
void free_graph(graph_t **graph);

/* Load a graph from the text file. */
void load_txt(const char *fname, graph_t *graph);
/* Load a graph from the binary file. */
void load_bin(const char *fname, graph_t *graph);

/* Save the graph to the text file. */
void save_txt(const graph_t *const graph, const char *fname);
/* Save the graph to the binary file. */
void save_bin(const graph_t *const graph, const char *fname);

#endif // __GRAPH_H__
