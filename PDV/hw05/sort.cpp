#include "sort.h"

#include <stdexcept>


void radix_par_task(std::vector<std::string *> &vector_to_sort, MappingFunction mapping_function, size_t alphabet_size,
                    size_t str_size,
                    size_t depth = 0) {
    if (vector_to_sort.size() <= 1 || depth >= str_size) return;

    std::vector<std::vector<std::string *> > buckets(alphabet_size);

    for (auto str: vector_to_sort) {
        const size_t idx = mapping_function(str->at(depth));
        buckets[idx].push_back(str);
    }

    for (size_t i = 0; i < alphabet_size; i++) {
        #pragma omp task shared(buckets) firstprivate(i)
        radix_par_task(buckets[i], mapping_function, alphabet_size, str_size, depth + 1);
    }

    #pragma omp taskwait

    size_t pos = 0;
    for (size_t i = 0; i < alphabet_size; i++) {
        for (const auto str: buckets[i]) {
            vector_to_sort[pos++] = str;
        }
    }
}

void radix_par(std::vector<std::string *> &vector_to_sort, MappingFunction mapping_function, size_t alphabet_size,
               size_t str_size) {
    #pragma omp parallel
    #pragma omp single
    radix_par_task(vector_to_sort, mapping_function, alphabet_size, str_size);
}


