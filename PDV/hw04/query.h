#pragma once

#include <cstdint>
#include <vector>
#include <functional>

template<typename T>
using Predicate = std::function<bool(T)>;

bool is_satisfied_for_all(const std::vector<Predicate<uint32_t> > &predicates, const std::vector<uint32_t> &data);

bool is_satisfied_for_any(const std::vector<Predicate<uint32_t> > &predicates, const std::vector<uint32_t> &data);
