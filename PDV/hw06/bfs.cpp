#include "bfs.h"
#include <atomic>
#include <mutex>
#include <omp.h>

class HashSet {
  static constexpr uint64_t EMPTY = 0;

  std::vector<std::atomic<uint64_t>> table;
  size_t mask;
  std::atomic<size_t> size{0};

  static size_t hash(uint64_t x) {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    return x;
  }

public:
  explicit HashSet(size_t capacity_pow2) : table(capacity_pow2), mask(capacity_pow2 - 1) {
    for (auto &slot : table) {
      slot.store(EMPTY, std::memory_order_relaxed);
    }
  }

  bool insert(uint64_t x) {
    if (x == EMPTY) x = 1;
    size_t i = hash(x) & mask;

    while (true) {
      uint64_t cur = table[i].load(std::memory_order_relaxed);

      if (cur == EMPTY) {
        if (table[i].compare_exchange_weak(cur, x, std::memory_order_release, std::memory_order_relaxed)) {
          size.fetch_add(1, std::memory_order_relaxed);
          return true; // Inserted
        }
      } else if (cur == x) {
        return false; // Already inserted
      }

      i = (i + 1) & mask;
    }
  }
};

state_ptr bfs(state_ptr root) {
  size_t max_t = omp_get_max_threads();
  HashSet visited(1 << 25); // 34M, all must fit!
  std::vector<state_ptr> opened{root};
  std::vector<std::vector<state_ptr>> local_next_opened(max_t);
  state_ptr best_goal{nullptr};
  std::mutex best_goal_m;

  while (!opened.empty()) {
    // --- Reduce currently opened layer to goal state with the lowest id, if any ---
#pragma omp parallel for
    for (size_t i = 0; i < opened.size(); i++) {
      const auto &state = opened[i];

      if (state->goal()) {
        std::lock_guard lock(best_goal_m);
        if (!best_goal || state->id() < best_goal->id()) {
          best_goal = state;
        }
      }
    }

    // --- If goal was found, return early ---
    if (best_goal) return best_goal;

    // --- No goal in this layer found, expand next layer ---
#pragma omp parallel
    {
      size_t tid = omp_get_thread_num();

#pragma omp for
      for (size_t i = 0; i < opened.size(); i++) {
        const auto &state = opened[i];
        const auto &neighbors = state->next_states();

        for (const auto &neigh : neighbors) {
          if (visited.insert(neigh->id())) local_next_opened[tid].push_back(neigh);
        }
      }
    }

    std::vector<state_ptr> next_opened;
    for (auto &v : local_next_opened) {
      next_opened.insert(next_opened.end(), std::make_move_iterator(v.begin()), std::make_move_iterator(v.end()));
      v.clear();
    }
    opened = std::move(next_opened);
  }

  return nullptr;
}