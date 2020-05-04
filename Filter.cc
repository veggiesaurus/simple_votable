#include <string>
#include <utility>
#include <numeric>
#include "Filter.h"

using namespace std;

namespace carta {
IndexList StringFilter(Column* column, string search_string, bool case_insensitive) {
    auto string_column = dynamic_cast<StringColumn*>(column);
    if (!string_column) {
        return IndexList();
    }
    size_t num_entries = string_column->entries.size();
    IndexList matching_indices;

    if (case_insensitive) {
        // If case-insensitive, must transform strings to lower-case while iterating
        transform(search_string.begin(), search_string.end(), search_string.begin(), ::tolower);
        for (auto i = 0; i < num_entries; i++) {
            auto val = string_column->entries[i];
            transform(val.begin(), val.end(), val.begin(), ::tolower);
            if (val.find(search_string) != string::npos) {
                matching_indices.push_back(i);
            }
        }
    } else {
        for (auto i = 0; i < num_entries; i++) {
            auto& val = string_column->entries[i];
            if (val.find(search_string) != string::npos) {
                matching_indices.push_back(i);
            }
        }
    }
    return matching_indices;
}

template<class T>
IndexList NumericFilterTemplated(Column* column, double min_value, double max_value) {
    IndexList matching_indices;

    auto numeric_column = dynamic_cast<NumericColumn<T>*>(column);
    if (!numeric_column) {
        return matching_indices;
    }

    size_t num_entries = numeric_column->entries.size();
    T typed_min = min_value;
    T typed_max = max_value;

    if (!isfinite(min_value)) {
        typed_min = numeric_limits<T>::lowest();
    }

    if (!isfinite(max_value)) {
        typed_max = numeric_limits<T>::max();
    }

    for (auto i = 0; i < num_entries; i++) {
        T val = numeric_column->entries[i];
        if (val >= typed_min && val <= typed_max) {
            matching_indices.push_back(i);
        }
    }

    return matching_indices;
}

IndexList NumericFilter(Column* column, double min_value, double max_value) {
    if (!column) {
        return IndexList();
    }

    switch (column->data_type) {
        case FLOAT: return NumericFilterTemplated<float>(column, min_value, max_value);
        case DOUBLE: return NumericFilterTemplated<double>(column, min_value, max_value);
        case INT: return NumericFilterTemplated<int>(column, min_value, max_value);
        case LONG: return NumericFilterTemplated<long>(column, min_value, max_value);
        default: return IndexList();
    }
}

IndexList LogicalFilter(LogicalOperator op, const IndexList& A, const IndexList& B) {
    IndexList combined_indices;
    if (op == AND) {
        set_intersection(A.begin(), A.end(), B.begin(), B.end(), back_inserter(combined_indices));
    } else {
        set_union(A.begin(), A.end(), B.begin(), B.end(), back_inserter(combined_indices));
    }
    return combined_indices;
}

IndexList InvertIndices(const IndexList& indices, int64_t total_row_count) {
    IndexList inverted_indices;
    auto inverted_row_count = total_row_count - indices.size();

    // after inversion, all indices are included
    if (indices.empty()) {
        inverted_indices.resize(indices.size());
        iota(inverted_indices.begin(), inverted_indices.end(), 0);
    } else if (inverted_row_count > 0) {
        inverted_indices.reserve(inverted_row_count);
        auto it = indices.begin();
        auto next_val = *it;
        for (auto i = 0; i < total_row_count; i++) {
            // index is in the existing set
            if (i == next_val) {
                it++;

                if (it == indices.end()) {
                    next_val = -1;
                } else {
                    next_val = *it;
                }
            } else {
                inverted_indices.push_back(i);
            }
        }
    }

    return inverted_indices;
}

}