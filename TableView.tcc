#ifndef VOTABLE_TEST__TABLEVIEW_TCC_
#define VOTABLE_TEST__TABLEVIEW_TCC_

#include <execution>

namespace carta {

template<class T>
bool TableView::SortByNumericColumn(const Column* column, bool ascending) {
    auto numeric_column = dynamic_cast<const NumericColumn<T>*>(column);
    if (!numeric_column) {
        return false;
    }

    // If we're sorting an entire column, we first need to populate the indices
    if (!_is_subset) {
        _subset_indices.resize(_table.NumRows());
        std::iota(_subset_indices.begin(), _subset_indices.end(), 0);
        _is_subset = true;
    }

    // Perform ascending or descending sort
    if (ascending) {
        std::sort(std::execution::par_unseq, _subset_indices.begin(), _subset_indices.end(), [numeric_column](int64_t a, int64_t b) {
            return numeric_column->entries[a] < numeric_column->entries[b];
        });
    } else {
        std::sort(std::execution::par_unseq, _subset_indices.begin(), _subset_indices.end(), [numeric_column](int64_t a, int64_t b) {
            return numeric_column->entries[a] > numeric_column->entries[b];
        });
    }

    // After sorting by a specific column, the table view is no longer ordered by index
    _ordered = false;
    return true;
}

template<class T>
std::vector<T> TableView::NumericValues(const Column* column, int64_t start, int64_t end) const {
    auto numeric_column = dynamic_cast<const NumericColumn<T>*>(column);
    if (!numeric_column || numeric_column->entries.empty()) {
        return std::vector<T>();
    }

    if (_is_subset) {
        int64_t N = _subset_indices.size();
        int64_t begin_index = std::clamp(start, 0L, N - 1);
        int64_t end_index = std::clamp(end, begin_index, N - 1);
        if (end < 0) {
            end_index = _subset_indices.size() - 1;
        }

        auto begin_it = _subset_indices.begin() + begin_index;
        auto end_it = _subset_indices.begin() + end_index;
        std::vector<T> values;
        values.reserve(std::distance(begin_it, end_it));

        auto& entries = numeric_column->entries;
        for (auto it = begin_it; it != end_it; it++) {
            values.push_back(entries[*it]);
        }
        return values;
    } else {
        int64_t N = numeric_column->entries.size();
        int64_t begin_index = std::clamp(start, 0L, N - 1);
        int64_t end_index = std::clamp(end, begin_index, N - 1);
        if (end < 0) {
            end_index = N - 1;
        }

        auto begin_it = numeric_column->entries.begin() + begin_index;
        auto end_it = numeric_column->entries.end() + end_index;
        return std::vector<T>(begin_it, end_it);
    }
}

}

#endif // VOTABLE_TEST__TABLEVIEW_TCC_