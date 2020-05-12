#include "TableView.h"
#include "Table.h"

#include <numeric>
#include <execution>
#include <algorithm>

namespace carta {

using namespace std;

TableView::TableView(const Table* table) :
    _table(table) {
    _is_subset = false;
    _ordered = true;
}

TableView::TableView(const Table* table, const IndexList& index_list, bool ordered) :
    _table(table),
    _subset_indices(index_list),
    _ordered(ordered) {
    _is_subset = true;
}

bool TableView::NumericFilter(int64_t column_index, double min_value, double max_value) {
    if (_table) {
        return NumericFilter(_table->GetColumnByIndex(column_index), min_value, max_value);
    } else {
        return false;
    }
}

bool TableView::NumericFilter(const string& column_name_or_id, double min_value, double max_value) {
    if (_table) {
        return NumericFilter(_table->GetColumn(column_name_or_id), min_value, max_value);
    } else {
        return false;
    }
}

bool TableView::NumericFilter(Column* column, double min_value, double max_value) {
    if (!column) {
        return false;
    }

    switch (column->data_type) {
        case FLOAT: return NumericFilterTemplated<float>(column, min_value, max_value);
        case DOUBLE: return NumericFilterTemplated<double>(column, min_value, max_value);
        case INT: return NumericFilterTemplated<int>(column, min_value, max_value);
        case LONG: return NumericFilterTemplated<long>(column, min_value, max_value);
        default: return false;
    }
}

template<class T>
bool TableView::NumericFilterTemplated(Column* column, double min_value, double max_value) {
    IndexList matching_indices;

    auto numeric_column = dynamic_cast<NumericColumn<T>*>(column);
    if (!numeric_column) {
        return false;
    }

    T typed_min = min_value;
    T typed_max = max_value;

    if (!isfinite(min_value)) {
        typed_min = numeric_limits<T>::lowest();
    }

    if (!isfinite(max_value)) {
        typed_max = numeric_limits<T>::max();
    }

    size_t num_entries = numeric_column->entries.size();

    if (_is_subset) {
        // Only iterate through existing indices
        for (auto i: _subset_indices) {
            // Skip invalid entries
            if (i < 0 || i >= num_entries) {
                continue;
            }
            T val = numeric_column->entries[i];
            if (val >= typed_min && val <= typed_max) {
                matching_indices.push_back(i);
            }
        }
    } else {
        // Iterate through all possible indices
        for (auto i = 0; i < num_entries; i++) {
            T val = numeric_column->entries[i];
            if (val >= typed_min && val <= typed_max) {
                matching_indices.push_back(i);
            }
        }
    }

    if (matching_indices.size() == num_entries) {
        _subset_indices.clear();
        _is_subset = false;
    } else {
        _is_subset = true;
        _subset_indices = matching_indices;
    }
    return true;
}

bool TableView::StringFilter(int64_t column_index, string search_string, bool case_insensitive) {
    if (_table) {
        return StringFilter(_table->GetColumnByIndex(column_index), search_string, case_insensitive);
    } else {
        return false;
    }
}

bool TableView::StringFilter(const string& column_name_or_id, string search_string, bool case_insensitive) {
    if (_table) {
        return StringFilter(_table->GetColumn(column_name_or_id), search_string, case_insensitive);
    } else {
        return false;
    }
}

bool TableView::StringFilter(Column* column, string search_string, bool case_insensitive) {
    IndexList matching_indices;

    auto string_column = dynamic_cast<StringColumn*>(column);
    if (!_table || !string_column) {
        return false;
    }
    size_t num_entries = string_column->entries.size();

    if (case_insensitive) {
        // If case-insensitive, must transform strings to lower-case while iterating
        transform(search_string.begin(), search_string.end(), search_string.begin(), ::tolower);
        if (_is_subset) {
            for (auto i: _subset_indices) {
                // Skip invalid entries
                if (i < 0 || i >= num_entries) {
                    continue;
                }
                auto val = string_column->entries[i];
                transform(val.begin(), val.end(), val.begin(), ::tolower);
                if (val.find(search_string) != string::npos) {
                    matching_indices.push_back(i);
                }
            }
        } else {
            for (auto i = 0; i < num_entries; i++) {
                auto val = string_column->entries[i];
                transform(val.begin(), val.end(), val.begin(), ::tolower);
                if (val.find(search_string) != string::npos) {
                    matching_indices.push_back(i);
                }
            }
        }
    } else {
        if (_is_subset) {
            for (auto i: _subset_indices) {
                // Skip invalid entries
                if (i < 0 || i >= num_entries) {
                    continue;
                }
                auto& val = string_column->entries[i];
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
    }

    if (matching_indices.size() == num_entries) {
        _subset_indices.clear();
        _is_subset = false;
    } else {
        _is_subset = true;
        _subset_indices = matching_indices;
    }
    return true;
}

bool TableView::Invert() {
    IndexList inverted_indices;
    auto total_row_count = _table->NumRows();

    if (_is_subset) {
        if (_subset_indices.empty()) {
            // Invert of NONE is ALL, so new view is not a subset
            _is_subset = false;
            _subset_indices.clear();
        } else if (_ordered) {
            // Can only invert if subset is ordered
            auto inverted_row_count = total_row_count - _subset_indices.size();
            inverted_indices.reserve(inverted_row_count);
            auto end = _subset_indices.end();
            auto it = _subset_indices.begin();
            auto next_val = *it;
            for (auto i = 0; i < total_row_count; i++) {
                // index is in the existing set
                if (i == next_val) {
                    it++;

                    if (it == end) {
                        next_val = -1;
                    } else {
                        next_val = *it;
                    }
                } else {
                    inverted_indices.push_back(i);
                }
            }
            _subset_indices = inverted_indices;
        } else {
            return false;
        }
    } else {
        // Inverse of ALL is NONE
        _is_subset = true;
        _subset_indices.clear();
    }
    return true;
}

bool TableView::Combine(const TableView& second) {
    // If either of the views are not valid, the combined table is not valid
    if (!_table || !second._table) {
        return false;
    }
    // If the views point to different tables, the combined table is not valid
    if (_table != second._table) {
        return false;
    }
    // If either table is not a subset, the combined table is not a subset
    if (!(_is_subset && second._is_subset)) {
        _is_subset = false;
        return true;
    }

    // If either table has unordered indices, they cannot be combined
    if (!(_ordered && second._ordered)) {
        return false;
    }

    IndexList combined_indices;
    set_union(_subset_indices.begin(), _subset_indices.end(), second._subset_indices.begin(), second._subset_indices.end(), back_inserter(combined_indices));
    if (combined_indices.size() == _table->NumRows()) {
        _subset_indices.clear();
        _is_subset = false;
    } else {
        _is_subset = true;
        _subset_indices = combined_indices;
    }

    return true;
}

bool TableView::SortByColumn(int64_t column_index, bool ascending) {
    if (_table) {
        return SortByColumn(_table->GetColumnByIndex(column_index), ascending);
    } else {
        return false;
    }
}

bool TableView::SortByColumn(const string& column_name_or_id, bool ascending) {
    if (_table) {
        return SortByColumn(_table->GetColumn(column_name_or_id), ascending);
    } else {
        return false;
    }
}

bool TableView::SortByColumn(Column* column, bool ascending) {
    if (!column) {
        return false;
    }

    switch (column->data_type) {
        case FLOAT: return SortByNumericColumn<float>(column, ascending);
        case DOUBLE: return SortByNumericColumn<double>(column, ascending);
        case INT: return SortByNumericColumn<int>(column, ascending);
        case LONG: return SortByNumericColumn<long>(column, ascending);
        case STRING: return SortByStringColumn(column, ascending);
        default: return false;
    }
}

bool TableView::SortByStringColumn(Column* column, bool ascending) {
    auto string_column = dynamic_cast<StringColumn*>(column);
    if (!string_column) {
        return false;
    }

    // If we're sorting an entire column, we first need to populate the indices
    if (!_is_subset) {
        _subset_indices.resize(_table->NumRows());
        std::iota(_subset_indices.begin(), _subset_indices.end(), 0);
        _is_subset = true;
    }

    // Perform ascending or descending sort
    if (ascending) {
        std::sort(std::execution::par_unseq, _subset_indices.begin(), _subset_indices.end(), [string_column](int64_t a, int64_t b) {
            return string_column->entries[a] < string_column->entries[b];
        });
    } else {
        std::sort(std::execution::par_unseq, _subset_indices.begin(), _subset_indices.end(), [string_column](int64_t a, int64_t b) {
            return string_column->entries[a] > string_column->entries[b];
        });
    }

    // After sorting by a specific column, the table view is no longer ordered by index
    _ordered = false;
    return true;
}

bool TableView::SortByIndex() {
    if (!_ordered) {
        sort(execution::par_unseq, _subset_indices.begin(), _subset_indices.end());
    }
    _ordered = true;
    return true;
}

size_t TableView::NumRows() const {
    if (_is_subset) {
        return _subset_indices.size();
    } else if (_table) {
        return _table->NumRows();
    }
    return 0;
}

vector<string> TableView::StringValues(int64_t column_index, int64_t start, int64_t end) const {
    if (_table) {
        return StringValues(_table->GetColumnByIndex(column_index), start, end);
    }
    return vector<string>();
}

vector<string> TableView::StringValues(const string& column_name_or_id, int64_t start, int64_t end) const {
    if (_table) {
        return StringValues(_table->GetColumn(column_name_or_id), start, end);
    }
    return vector<string>();
}

vector<string> TableView::StringValues(Column* column, int64_t start, int64_t end) const {
    auto string_column = dynamic_cast<StringColumn*>(column);
    if (!string_column || string_column->entries.empty()) {
        return vector<string>();
    }

    if (_is_subset) {
        int64_t N = _subset_indices.size();
        int64_t begin_index = clamp(start, 0L, N - 1);
        int64_t end_index = clamp(end, begin_index, N - 1);
        if (end < 0) {
            end_index = N - 1;
        }

        auto begin_it = _subset_indices.begin() + begin_index;
        auto end_it = _subset_indices.begin() + end_index;
        vector<string> values;
        values.reserve(distance(begin_it, end_it));

        auto& entries = string_column->entries;
        for (auto it = begin_it; it != end_it; it++) {
            values.push_back(entries[*it]);
        }
        return values;
    } else {
        int64_t N = string_column->entries.size();
        int64_t begin_index = clamp(start, 0L, N - 1);
        int64_t end_index = clamp(end, begin_index, N - 1);
        if (end < 0) {
            end_index = N - 1;
        }

        auto begin_it = string_column->entries.begin() + begin_index;
        auto end_it = string_column->entries.end() + end_index;
        return vector<string>(begin_it, end_it);
    }
}
}