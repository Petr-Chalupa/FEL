#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ----- Defines -----

// Ranges are inclusive
#define N_MIN 1
#define N_MAX 250000
#define K_MIN 1
#define K_MAX 10000

// ----- Typedefs -----

typedef enum { E_MEMALLOC = 100, E_INVALIDINP = 101, E_INPOUTOFRANGE = 102 } Error;

typedef struct Node {
  int key;
  bool deleted;
  struct Node *left;
  struct Node *right;
} Node;

typedef struct {
  Node *root;
  int depth;
  int nodes;
  int sum_active_heights;
  int sum_deleted_heights;
  int compact_calls;
} BST;

// ----- Global vars -----

int N;
BST tree = {.root = NULL, .depth = -1, .nodes = 0, .sum_active_heights = 0, .sum_deleted_heights = 0, .compact_calls = 0};

// ----- Declarations -----

void Error__throw(Error e);
//
Node *Node__init(int key);
void Node__free(Node *n);
//
void BST__free(BST *bst);
void BST__insert(BST *bst, int K);
void BST__delete(BST *bst, int K);
void BST__update_heights(BST *bst);
void BST__compact(BST *bst);
Node *_insert(Node *n, int K, int curr_depth, int *insert_depth, bool *was_inserted, bool *was_undeleted);
Node *_delete(Node *n, int K, int curr_depth, int *delete_depth);
void _update_heights(Node *n, int *sum_active_heights, int *sum_deleted_heights, int tree_depth, int curr_depth);
void _get_active_keys(Node *n, int *keys, int *idx);

// ----- Definitions -----

int main() {
  if (scanf("%d\n", &N) != 1) Error__throw(E_INVALIDINP);
  if (!(N_MIN <= N && N <= N_MAX)) Error__throw(E_INPOUTOFRANGE);

  for (int i = 0; i < N; i++) {
    char instruction[4]; // 3 chars + null term.
    int K;
    if (scanf("%s %d", instruction, &K) != 2) Error__throw(E_INVALIDINP);
    if (!(K_MIN <= K && K <= K_MAX)) Error__throw(E_INPOUTOFRANGE);

    if (strcmp(instruction, "ins") == 0) {
      BST__insert(&tree, K);
    } else if (strcmp(instruction, "del") == 0) {
      BST__delete(&tree, K);
    } else {
      Error__throw(E_INVALIDINP);
    }
  }

  printf("%d %d\n", tree.compact_calls, tree.depth);

  BST__free(&tree);
  return 0;
}

void Error__throw(Error e) {
  switch (e) {
  case E_MEMALLOC: fprintf(stderr, "Error: Memory allocation!\n"); break;
  case E_INVALIDINP: fprintf(stderr, "Error: Invalid argument!\n"); break;
  case E_INPOUTOFRANGE: fprintf(stderr, "Error: Input is out of range!\n"); break;
  }

  BST__free(&tree);
  exit(e);
}

Node *Node__init(int key) {
  Node *n = malloc(sizeof(Node));
  if (n == NULL) Error__throw(E_MEMALLOC);

  n->key = key;
  n->deleted = false;
  n->left = NULL;
  n->right = NULL;

  return n;
}

void Node__free(Node *n) {
  if (n == NULL) return;

  Node__free(n->left);
  Node__free(n->right);
  free(n);
}

void BST__free(BST *bst) {
  if (bst == NULL) return;

  Node__free(bst->root);
  bst->root = NULL;
}

void BST__insert(BST *bst, int K) {
  if (bst == NULL) return;

  int insert_depth = -1;
  bool was_inserted = false;
  bool was_undeleted = false;
  bst->root = _insert(bst->root, K, 0, &insert_depth, &was_inserted, &was_undeleted);

  if (was_inserted) {
    bst->nodes++;
    if (insert_depth > bst->depth) {
      bst->depth = insert_depth;
      BST__update_heights(bst);
    } else {
      bst->sum_active_heights += bst->depth - insert_depth;
    }
  }
  if (was_undeleted) {
    int height = bst->depth - insert_depth;
    bst->sum_active_heights += height;
    bst->sum_deleted_heights -= height;
  }

  if (bst->sum_deleted_heights > bst->sum_active_heights) BST__compact(bst);
}

void BST__delete(BST *bst, int K) {
  if (bst == NULL) return;

  int delete_depth = -1;
  bst->root = _delete(bst->root, K, 0, &delete_depth);

  if (delete_depth != -1) {
    int height = bst->depth - delete_depth;
    bst->sum_active_heights -= height;
    bst->sum_deleted_heights += height;
  }

  if (bst->sum_deleted_heights > bst->sum_active_heights) BST__compact(bst);
}

void BST__update_heights(BST *bst) {
  if (bst == NULL) return;

  bst->sum_active_heights = 0;
  bst->sum_deleted_heights = 0;
  _update_heights(bst->root, &bst->sum_active_heights, &bst->sum_deleted_heights, bst->depth, 0);
}

void BST__compact(BST *bst) {
  if (bst == NULL) return;

  bst->compact_calls++;

  int *active_keys = malloc(bst->nodes * sizeof(int));
  if (active_keys == NULL) Error__throw(E_MEMALLOC);
  int active_index = 0;
  _get_active_keys(bst->root, active_keys, &active_index);

  // Rebuild tree preserving pre-order
  Node__free(bst->root);
  bst->root = NULL;
  bst->depth = -1;
  bst->nodes = active_index;
  for (int i = 0; i < active_index; i++) {
    int insert_depth = -1;
    bool was_inserted = false;
    bool was_undeleted = false;
    bst->root = _insert(bst->root, active_keys[i], 0, &insert_depth, &was_inserted, &was_undeleted);
    if (insert_depth > bst->depth) bst->depth = insert_depth;
  }
  BST__update_heights(bst);

  free(active_keys);
}

Node *_insert(Node *n, int K, int curr_depth, int *insert_depth, bool *was_inserted, bool *was_undeleted) {
  if (n == NULL) {
    *insert_depth = curr_depth;
    *was_inserted = true;
    return Node__init(K);
  }

  if (K < n->key) {
    n->left = _insert(n->left, K, curr_depth + 1, insert_depth, was_inserted, was_undeleted);
  } else if (K > n->key) {
    n->right = _insert(n->right, K, curr_depth + 1, insert_depth, was_inserted, was_undeleted);
  } else if (K == n->key && n->deleted) {
    n->deleted = false;
    *insert_depth = curr_depth;
    *was_undeleted = true;
  }

  return n;
}

Node *_delete(Node *n, int K, int curr_depth, int *delete_depth) {
  if (n == NULL) return NULL;

  if (K < n->key) {
    n->left = _delete(n->left, K, curr_depth + 1, delete_depth);
  } else if (K > n->key) {
    n->right = _delete(n->right, K, curr_depth + 1, delete_depth);
  } else if (K == n->key && !n->deleted) {
    n->deleted = true;
    *delete_depth = curr_depth;
  }

  return n;
}

void _update_heights(Node *n, int *sum_active_heights, int *sum_deleted_heights, int tree_depth, int curr_depth) {
  if (n == NULL) return;

  int height = tree_depth - curr_depth;
  if (n->deleted) {
    *sum_deleted_heights += height;
  } else {
    *sum_active_heights += height;
  }

  _update_heights(n->left, sum_active_heights, sum_deleted_heights, tree_depth, curr_depth + 1);
  _update_heights(n->right, sum_active_heights, sum_deleted_heights, tree_depth, curr_depth + 1);
}

void _get_active_keys(Node *n, int *keys, int *idx) {
  if (n == NULL) return;

  // Collect the keys in pre-order to preserve it
  if (!n->deleted) {
    keys[*idx] = n->key;
    (*idx)++;
  }
  _get_active_keys(n->left, keys, idx);
  _get_active_keys(n->right, keys, idx);
}
