#include "hw09.h"
#include <stdio.h>
#include <stdlib.h>

graph_t *allocate_graph() {
  graph_t *graph = malloc(sizeof(graph_t));
  if (graph == NULL) return NULL;

  graph->edges = malloc(3 * sizeof(int)); // 3 ints corresponds to 1 edge
  if (graph->edges == NULL) {
    free(graph);
    return NULL;
  }

  graph->capacity = 3;
  graph->size = 0;
  return graph;
}

void free_graph(graph_t **graph) {
  free((*graph)->edges);
  free(*graph);
  *graph = NULL;
}

void enlarge_graph(graph_t *graph) {
  graph->edges = realloc(graph->edges, 2 * graph->capacity * sizeof(int));
  if (graph->edges == NULL) exit(EXIT_FAILURE);

  graph->capacity *= 2;
}

void load_txt(const char *fname, graph_t *graph) {
  FILE *file = fopen(fname, "r");
  if (file == NULL) exit(EXIT_FAILURE);

  // --- Commented out, because the specification required speed ---
  // int start, end, cost;
  // while (fscanf(file, "%d %d %d\n", &start, &end, &cost) == 3) {
  //   if (graph->size + 3 > graph->capacity) enlarge_graph(graph);
  //   graph->edges[graph->size++] = start;
  //   graph->edges[graph->size++] = end;
  //   graph->edges[graph->size++] = cost;
  // }

  int n = 0;
  while (1) {
    char ch = fgetc(file);
    if (ch == EOF) {
      if (feof(file))
        break;
      else
        exit(EXIT_FAILURE);
    }

    if (ch == ' ' || ch == '\n') {
      if (graph->size + 1 > graph->capacity) enlarge_graph(graph);
      graph->edges[graph->size++] = n;
      n = 0;
      continue;
    }

    n = (10 * n) + (ch - '0');
  }

  fclose(file);
}

void save_txt(const graph_t *const graph, const char *fname) {
  FILE *file = fopen(fname, "w");
  if (file == NULL) exit(EXIT_FAILURE);

  // --- Commented out, because the specification required speed ---
  // for (size_t i = 0; i < graph->size; i += 3) {
  //   fprintf(file, "%d %d %d\n", graph->edges[i], graph->edges[i + 1], graph->edges[i + 2]);
  // }

  char buffer[11];
  for (size_t i = 0; i < graph->size; i += 3) {
    sprintf(buffer, "%d", graph->edges[i]);
    fputs(buffer, file);
    fputc(' ', file);
    sprintf(buffer, "%d", graph->edges[i + 1]);
    fputs(buffer, file);
    fputc(' ', file);
    sprintf(buffer, "%d", graph->edges[i + 2]);
    fputs(buffer, file);
    fputc('\n', file);
  }

  fclose(file);
}

void load_bin(const char *fname, graph_t *graph) {
  FILE *file = fopen(fname, "r");
  if (file == NULL) exit(EXIT_FAILURE);

  while (fread(&graph->edges[graph->size], sizeof(int), 3, file) == 3) {
    graph->size += 3;
    if (graph->size + 3 > graph->capacity) enlarge_graph(graph);
  }

  fclose(file);
}

void save_bin(const graph_t *const graph, const char *fname) {
  FILE *file = fopen(fname, "w");
  if (file == NULL) exit(EXIT_FAILURE);

  fwrite(graph->edges, sizeof(int), graph->size, file);

  fclose(file);
}
