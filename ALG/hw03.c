#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Inclusive range
#define N_MIN 3
#define N_MAX 4000000

typedef enum { ERR_INVALID_INPUT = 100, ERR_MEM_ALLOC = 101 } Error;

typedef enum { WHITE = 0, RED = 1, BLUE = 2 } Color;

typedef struct Tree {
  Color clr;
  struct Tree *left;
  struct Tree *right;
} Tree;

typedef struct {
  Tree **data;
  size_t capacity;
  size_t head;
  size_t tail;
} Queue;

void Error__throw(Error e);
Tree *Tree__init(Color clr);
void Tree__free(Tree *t);
Tree *Tree__read(int N);
void Tree_solve(Tree *t, int *K);
int Tree_solve_recursive(Tree *t, Color prev_clr, int *max_K);
Queue *Queue__init(int size);
void Queue__free(Queue *q);
void Queue__push(Queue *q, Tree *t);
Tree *Queue__pop(Queue *q);

int main() {
  // Read the size of a tree
  int N;
  if (scanf("%d\n", &N) != 1 || N < N_MIN || N > N_MAX) {
    Error__throw(ERR_INVALID_INPUT);
  }

  // Read the tree
  Tree *root = Tree__read(N);

  // Solve and print the result
  int K = 0;
  Tree_solve(root, &K);
  printf("%d\n", K);

  Tree__free(root);

  return 0;
}

void Error__throw(Error e) {
  switch (e) {
  case ERR_INVALID_INPUT: fprintf(stderr, "Error: The input is invalid!\n"); break;
  case ERR_MEM_ALLOC: fprintf(stderr, "Error: There was an error allocating memory!\n"); break;
  }

  exit(e);
}

Tree *Tree__init(Color clr) {
  Tree *t = malloc(sizeof(Tree));
  if (t == NULL) {
    Error__throw(ERR_MEM_ALLOC);
  }

  t->clr = clr;
  t->left = NULL;
  t->right = NULL;

  return t;
}

void Tree__free(Tree *t) {
  if (t == NULL) return;

  if (t->left != NULL) Tree__free(t->left);
  if (t->right != NULL) Tree__free(t->right);
  free(t);
}

Tree *Tree__read(int N) {
  Queue *q = Queue__init(N);
  Tree *root = NULL;
  int clr;
  int nodes_read = 0;

  // Read the root
  if (scanf("%d", &clr) != 1 || (clr != WHITE && clr != RED && clr != BLUE)) {
    Queue__free(q);
    Error__throw(ERR_INVALID_INPUT);
  }
  root = Tree__init((Color)clr);
  Queue__push(q, root);
  nodes_read++;

  // Read the rest (every node has 0 or 2 children)
  while (nodes_read < N) {
    Tree *parent = Queue__pop(q);

    if (parent->clr == BLUE) {
      continue; // Leaf
    }

    // Left child
    if (scanf("%d", &clr) != 1 || (clr != WHITE && clr != RED && clr != BLUE)) {
      Tree__free(root);
      Queue__free(q);
      Error__throw(ERR_INVALID_INPUT);
    }
    parent->left = Tree__init((Color)clr);
    Queue__push(q, parent->left);

    // Right child
    if (scanf("%d", &clr) != 1 || (clr != WHITE && clr != RED && clr != BLUE)) {
      Tree__free(root);
      Queue__free(q);
      Error__throw(ERR_INVALID_INPUT);
    }
    parent->right = Tree__init((Color)clr);
    Queue__push(q, parent->right);

    nodes_read += 2;
  }

  Queue__free(q);

  return root;
}

void Tree_solve(Tree *root, int *K) {
  if (root == NULL) {
    *K = 0;
    return;
  }
  Tree_solve_recursive(root, (Color)-1, K); // Init with color out of range
}

int Tree_solve_recursive(Tree *t, Color prev_clr, int *max_K) {
  if (t == NULL || t->clr == BLUE) {
    return 1;
  }

  int left_path = Tree_solve_recursive(t->left, t->clr, max_K);
  int right_path = Tree_solve_recursive(t->right, t->clr, max_K);

  // Update max if paths are joinable
  if (t->left->clr != t->clr || t->right->clr != t->clr) {
    int current_max_path = left_path + right_path + 1;
    if (current_max_path > *max_K) {
      *max_K = current_max_path;
    }
  }

  // Return longer path (or at least the valid one)
  int ret_path;
  if (prev_clr == t->clr) {
    if (t->clr != t->left->clr && t->clr != t->right->clr) {
      ret_path = left_path > right_path ? left_path : right_path;
    } else if (t->clr == t->left->clr && t->clr == t->right->clr) {
      ret_path = 0;
    } else if (t->clr == t->left->clr) {
      ret_path = right_path;
    } else {
      ret_path = left_path;
    }
  } else {
    ret_path = left_path > right_path ? left_path : right_path;
  }

  return 1 + ret_path;
}

Queue *Queue__init(int size) {
  Queue *q = malloc(sizeof(Queue));
  if (q == NULL) {
    Error__throw(ERR_MEM_ALLOC);
  }

  q->data = (Tree **)malloc(size * sizeof(Tree *));
  if (q->data == NULL) {
    free(q);
    Error__throw(ERR_MEM_ALLOC);
  }

  q->capacity = size;
  q->head = 0;
  q->tail = 0;

  return q;
}

void Queue__free(Queue *q) {
  if (q == NULL) return;

  free(q->data);
  free(q);
}

void Queue__push(Queue *q, Tree *t) {
  if (q->tail >= q->capacity) {
    Error__throw(ERR_INVALID_INPUT);
  }

  q->data[q->tail++] = t;
}

Tree *Queue__pop(Queue *q) {
  if (q->head == q->tail) {
    return NULL;
  }

  return q->data[q->head++];
}
