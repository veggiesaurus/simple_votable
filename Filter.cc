#include <string>
#include <utility>
#include "Filter.h"

using namespace carta;

StringFilter::StringFilter(Column* column, const std::string& search_string, bool case_insensitive) :
    _search_string(search_string),
    _case_insensitive(case_insensitive) {
    _column = dynamic_cast<StringColumn*>(column);
    _valid = _column && _column->data_type == DataType::STRING;

    if (_case_insensitive) {
        std::transform(_search_string.begin(), _search_string.end(), _search_string.begin(), ::tolower);
    }
}

std::vector<int64_t> StringFilter::Execute() {
    if (!_valid) {
        return std::vector<int64_t>();
    }

    size_t num_entries = _column->entries.size();
    std::vector<int64_t> matching_indices;

    if (_case_insensitive) {
        // If case-insensitive, must transform strings to lower-case while iterating
        for (auto i = 0; i < num_entries; i++) {
            auto val = _column->entries[i];
            std::transform(val.begin(), val.end(), val.begin(), ::tolower);
            if (val.find(_search_string) != std::string::npos) {
                matching_indices.push_back(i);
            }
        }
    } else {
        for (auto i = 0; i < num_entries; i++) {
            auto& val = _column->entries[i];
            if (val.find(_search_string) != std::string::npos) {
                matching_indices.push_back(i);
            }
        }
    }
    return matching_indices;
}

NumericFilter::NumericFilter(carta::Column* column, double min_value, double max_value) :
    _min_value(min_value), _max_value(max_value), _column(column) {
    // Valid columns can be int, long, float or double
    _valid = (dynamic_cast<NumericColumn<float>*>(column) && column->data_type == DataType::FLOAT)
        || (dynamic_cast<NumericColumn<double>*>(column) && column->data_type == DataType::DOUBLE)
        || (dynamic_cast<NumericColumn<int>*>(column) && column->data_type == DataType::INT)
        || (dynamic_cast<NumericColumn<int64_t>*>(column) && column->data_type == DataType::LONG);

    // at least one of min and max must be defined
    _valid &= (std::isfinite(_max_value) || std::isfinite(_min_value));
}

std::vector<int64_t> NumericFilter::Execute() {
    if (!_valid) {
        return std::vector<int64_t>();
    }

    // Is this really the best way to do this?
    switch (_column->data_type) {
        case FLOAT: return TemplatedExecute<float>();
        case DOUBLE: return TemplatedExecute<double>();
        case INT: return TemplatedExecute<int>();
        case LONG: return TemplatedExecute<long>();
        default:return std::vector<int64_t>();
    }
}

template<class T>
std::vector<int64_t> NumericFilter::TemplatedExecute() {
    std::vector<int64_t> matching_indices;

    auto numeric_column = dynamic_cast<NumericColumn<T>*>(_column);
    if (!numeric_column) {
        return matching_indices;
    }

    size_t num_entries = numeric_column->entries.size();
    T typed_min = _min_value;
    T typed_max = _max_value;

    if (!std::isfinite(_min_value)) {
        typed_min = std::numeric_limits<T>::lowest();
    }

    if (!std::isfinite(_max_value)) {
        typed_max = std::numeric_limits<T>::max();
    }

    for (auto i = 0; i < num_entries; i++) {
        T val = numeric_column->entries[i];
        if (val >= typed_min && val <= typed_max) {
            matching_indices.push_back(i);
        }
    }

    return matching_indices;
}

LogicalFilter::LogicalFilter(LogicalOperator op, std::vector<Filter*> filters) :
    _operator(op) {
    _filters = std::move(filters);
}

LogicalFilter::LogicalFilter(LogicalOperator op, std::initializer_list<Filter*> filters) :
    _operator(op) {
    for (auto& filter: filters) {
        _filters.push_back(filter);
    }
}

bool LogicalFilter::IsValid() {
    // Filter is only valid if all its dependents are valid
    return !_filters.empty() && std::all_of(_filters.begin(), _filters.end(), [](Filter* f) {
        return f && f->IsValid();
    });
}
std::vector<int64_t> LogicalFilter::Execute() {
    std::vector<int64_t> matching_indices;

    if (!IsValid()) {
        return matching_indices;
    }

    matching_indices = _filters[0]->Execute();

    for (auto i = 1; i < _filters.size(); i++) {
        auto indices = _filters[i]->Execute();
        std::vector<int64_t> combined_indices;
        if (_operator == AND) {
            std::set_intersection(matching_indices.begin(), matching_indices.end(), indices.begin(), indices.end(), std::back_inserter(combined_indices));
        } else {
            std::set_union(matching_indices.begin(), matching_indices.end(), indices.begin(), indices.end(), std::back_inserter(combined_indices));
        }
        matching_indices.swap(combined_indices);
    }

    return matching_indices;
}
