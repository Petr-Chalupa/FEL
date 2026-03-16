#include "bst_tree.h"

void bst_tree::insert(const int64_t data) {
  const auto new_node = new node(data);

  auto *current_ptr = &root;
  while (true) {
    auto *expected = current_ptr->load();

    if (expected == nullptr) {
      if (current_ptr->compare_exchange_strong(expected, new_node)) {
        break;
      }
    }

    if (data < expected->data) {
      current_ptr = &expected->left;
    } else if (data > expected->data) {
      current_ptr = &expected->right;
    } else {
      delete new_node;
      break;
    }
  }
}

void delete_node(const bst_tree::node *node) {
  if (node == nullptr) {
    return;
  }

  delete_node(node->left);
  delete_node(node->right);
  delete node;
}

bst_tree::~bst_tree() { delete_node(root); }
