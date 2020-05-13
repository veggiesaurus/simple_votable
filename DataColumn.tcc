#ifndef VOTABLE_TEST__DATACOLUMN_TCC_
#define VOTABLE_TEST__DATACOLUMN_TCC_

#include "Columns.h"
#include <execution>

namespace carta {
template<class T>
DataColumn<T>::DataColumn(const std::string& name_chr): Column(name_chr) {
    // Assign type based on template type or traits
    if constexpr(std::is_same_v<T, std::string>) {
        data_type = STRING;
    } else if constexpr(std::is_same_v<T, float>) {
        data_type = FP32;
    } else if constexpr(std::is_floating_point_v<T>) {
        data_type = FLOAT_GENERIC;
    } else if constexpr(std::is_same_v<T, int>) {
        data_type = INT32;
    } else if constexpr(std::is_arithmetic_v<T>) {
        data_type = ARITHMETIC;
    } else {
        data_type = UNSUPPORTED;
    }
}

template<class T>
T DataColumn<T>::FromText(const pugi::xml_text& text) {
    // Parse properly based on template type or traits
    if constexpr(std::is_same_v<T, std::string>) {
        return text.as_string();
    } else if constexpr (std::is_same_v<T, float>) {
        return text.as_float(std::numeric_limits<T>::quiet_NaN());
    } else if constexpr (std::is_floating_point_v<T>) {
        return text.as_double((double) std::numeric_limits<T>::quiet_NaN());
    } else if constexpr(std::is_arithmetic_v<T>) {
        return text.as_llong(0);
    } else {
        return T();
    }
}

template<class T>
void DataColumn<T>::SetFromText(const pugi::xml_text& text, size_t index) {
    entries[index] = FromText(text);
}

template<class T>
void DataColumn<T>::SetEmpty(size_t index) {
    if constexpr(std::numeric_limits<T>::has_quiet_NaN) {
        entries[index] = std::numeric_limits<T>::quiet_NaN();
    } else {
        entries[index] = T();
    }
}

template<class T>
void DataColumn<T>::FillFromText(const pugi::xml_text& text) {
    entries.emplace_back(FromText(text));
}

template<class T>
void DataColumn<T>::FillEmpty() {
    if constexpr(std::numeric_limits<T>::has_quiet_NaN) {
        entries.emplace_back(std::numeric_limits<T>::quiet_NaN());
    } else {
        entries.emplace_back(T());
    }
}

template<class T>
void DataColumn<T>::Resize(size_t capacity) {
    entries.resize(capacity);
}

template<class T>
void DataColumn<T>::Reserve(size_t capacity) {
    entries.reserve(capacity);
}

template<class T>
size_t DataColumn<T>::NumEntries() const {
    return entries.size();
}

template<class T>
void DataColumn<T>::SortIndices(IndexList& indices, bool ascending) const {
    // Perform ascending or descending sort
    if (ascending) {
        std::sort(std::execution::par_unseq, indices.begin(), indices.end(), [&](int64_t a, int64_t b) {
            return entries[a] < entries[b];
        });
    } else {
        std::sort(std::execution::par_unseq, indices.begin(), indices.end(), [&](int64_t a, int64_t b) {
            return entries[a] > entries[b];
        });
    }
}

template<class T>
void DataColumn<T>::FilterIndices(IndexList& existing_indices, bool is_subset, double min_value, double max_value) const {
    // only apply to template types that are arithmetic
    if constexpr (std::is_arithmetic_v<T>) {
        T typed_min = min_value;
        T typed_max = max_value;

        IndexList matching_indices;

        if (!std::isfinite(min_value)) {
            typed_min = std::numeric_limits<T>::lowest();
        }

        if (!std::isfinite(max_value)) {
            typed_max = std::numeric_limits<T>::max();
        }

        size_t num_entries = entries.size();

        if (is_subset) {
            // Only iterate through existing indices
            for (auto i: existing_indices) {
                // Skip invalid entries
                if (i < 0 || i >= num_entries) {
                    continue;
                }
                T val = entries[i];
                if (val >= typed_min && val <= typed_max) {
                    matching_indices.push_back(i);
                }
            }
        } else {
            // Iterate through all possible indices
            for (auto i = 0; i < num_entries; i++) {
                T val = entries[i];
                if (val >= typed_min && val <= typed_max) {
                    matching_indices.push_back(i);
                }
            }
        }

        existing_indices.swap(matching_indices);
    }
}

}

#endif //VOTABLE_TEST__DATACOLUMN_TCC_
