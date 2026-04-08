#include "vector_sum.h"
#include <numeric>
#include <random>


void vector_sum_omp_per_vector(const InputVectors &data, OutputVector &solution, size_t min_vector_size) {
  for (size_t i = 0; i < data.size(); i++) {
    const auto &vec = data[i];
    long sum = 0;

    #pragma omp parallel for default(none) shared(vec) reduction(+ : sum)
    for (const auto j: vec) {
      sum += j;
    }

    solution[i] = sum;
  }
}

void vector_sum_omp_static(const InputVectors &data, OutputVector &solution, size_t min_vector_size) {
  #pragma omp parallel for default(none) shared(data, solution) schedule(static)
  for (size_t i = 0; i < data.size(); i++) {
    const auto &vec = data[i];
    long sum = 0;

    for (const auto j: vec) {
      sum += j;
    }

    solution[i] = sum;
  }
}

void vector_sum_omp_dynamic(const InputVectors &data, OutputVector &solution, size_t min_vector_size) {
  #pragma omp parallel for default(none) shared(data, solution) schedule(dynamic)
  for (size_t i = 0; i < data.size(); i++) {
    const auto &vec = data[i];
    long sum = 0;

    for (const auto j: vec) {
      sum += j;
    }

    solution[i] = sum;
  }
}

void vector_sum_omp_shuffle(const InputVectors &data, OutputVector &solution, size_t min_vector_size) {
  auto idxs = std::vector<size_t>(data.size());
  std::iota(idxs.begin(), idxs.end(), 0);
  std::ranges::shuffle(idxs, std::random_device{});

  #pragma omp parallel for default(none) shared(data, solution, idxs) schedule(static)
  for (size_t i = 0; i < data.size(); i++) {
    const auto idx = idxs[i];
    const auto &vec = data[idx];
    long sum = 0;

    for (const auto j: vec) {
      sum += j;
    }

    solution[idx] = sum;
  }
}
