#pragma once

#include <vector>
#include <algorithm>
#include <iostream>

using MappingFunction = size_t (*)(char c);

void radix_par(std::vector<std::string *> &vector_to_sort, MappingFunction mapping_function, size_t alphabet_size,
               size_t str_size);
