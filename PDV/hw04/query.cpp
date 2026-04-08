#include "query.h"
#include <atomic>


bool is_satisfied_for_all(const std::vector<Predicate<uint32_t> > &predicates, const std::vector<uint32_t> &data) {
    std::atomic<bool> res(true);

    #pragma omp parallel for
    for (size_t i = 0; i < predicates.size(); i++) {
        bool satisfied = false;
        for (size_t j = 0; j < data.size() && res; j++) {
            if (predicates[i](data[j])) {
                satisfied = true;
                break;
            }
        }

        if (!satisfied) {
            res = false;
            #pragma omp cancel for
        }
    }

    return res;
}

bool is_satisfied_for_any(const std::vector<Predicate<uint32_t> > &predicates, const std::vector<uint32_t> &data) {
    std::atomic<bool> res(false);

    #pragma omp parallel for
    for (size_t i = 0; i < predicates.size(); i++) {
        for (size_t j = 0; j < data.size() && !res; j++) {
            if (predicates[i](data[j])) {
                res = true;
                #pragma omp cancel for
                break;
            }
        }
    }

    return res;
}
