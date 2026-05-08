#include "state.h"
#include <algorithm>
#include <limits>
#include <memory>
#include <omp.h>
#include <vector>

struct alignas(64) Context {
  state_ptr best_goal = nullptr;
  uint64_t next_limit = std::numeric_limits<uint64_t>::max();
};

void iddfs_parallel(state_ptr current, uint64_t limit, Context &ctx, int depth) {
  // --- Prune if cost is already too high ---
  if (ctx.best_goal && current->total_cost() >= ctx.best_goal->total_cost()) return;

  // --- Expand neighbours ---
  const auto &neighbors = current->next_states();
  for (size_t i = 0; i < neighbors.size(); i++) {
    const auto &neigh = neighbors[i];
    uint64_t n_cost = neigh->total_cost();

    // --- Out of bounds, update next limit ---
    if (n_cost > limit) {
      if (n_cost < ctx.next_limit) {
#pragma omp critical(limit_update)
        {
          if (n_cost < ctx.next_limit) ctx.next_limit = n_cost;
        }
      }
      continue;
    }

    // --- Update goal ---
    if (neigh->goal()) {
#pragma omp critical(goal_update)
      {
        if (!ctx.best_goal || n_cost < ctx.best_goal->total_cost() || (n_cost == ctx.best_goal->total_cost() && neigh->id() < ctx.best_goal->id())) {
          ctx.best_goal = neigh;
        }
      }
      continue;
    }

    // --- Basic cycle detection ---
    const state_ptr &parent = current->previous_state();
    if (parent && parent->id() == neigh->id()) continue;

    // --- Further expansion ---
    if (i < neighbors.size() - 1 && depth < 5 && neighbors.size() > 2) {
#pragma omp task shared(ctx) untied
      iddfs_parallel(neigh, limit, ctx, depth + 1);
    } else {
      iddfs_parallel(neigh, limit, ctx, depth + 1);
    }
  }
}

/*
 * Works with both uniform-cost and weighted graphs
 */
state_ptr iddfs(state_ptr root) {
  if (root->goal()) return root;

  Context ctx;
  uint64_t limit = root->total_cost();

  while (true) {
    ctx.next_limit = std::numeric_limits<uint64_t>::max();
    ctx.best_goal = nullptr;

#pragma omp parallel
    {
#pragma omp single
      {
        // --- Split work from start ---
        const auto &neighbors = root->next_states();
        for (const auto &neigh : neighbors) {
#pragma omp task shared(ctx) untied
          iddfs_parallel(neigh, limit, ctx, 0);
        }
      }
    }

    if (ctx.best_goal) return ctx.best_goal;
    if (ctx.next_limit == std::numeric_limits<uint64_t>::max()) break;

    limit = ctx.next_limit;
  }

  return nullptr;
}
