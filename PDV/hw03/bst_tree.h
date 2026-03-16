#pragma once

#include <atomic>
#include <cstdint>

class bst_tree {
public:
  class node {
  public:
    std::atomic<node *> left{nullptr};
    std::atomic<node *> right{nullptr};
    int64_t data;

    explicit node(const int64_t data) : data(data) {}
  };

  std::atomic<node *> root{nullptr};

  ~bst_tree();

  void insert(int64_t data);
};
