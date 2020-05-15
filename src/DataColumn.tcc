#ifndef VOTABLE_TEST__DATACOLUMN_TCC_
#define VOTABLE_TEST__DATACOLUMN_TCC_

#include "Columns.h"

#include <algorithm>

namespace carta {
template<class T>
DataColumn<T>::DataColumn(const std::string& name_chr): Column(name_chr) {
    // Assign type based on template type or traits
    if constexpr(std::is_same_v<T, std::string>) {
        data_type = STRING;
    } else if constexpr(std::is_same_v<T, double>) {
        data_type = DOUBLE;
    } else if constexpr(std::is_floating_point_v<T>) {
        data_type = FLOAT_GENERIC;
    } else if constexpr(std::is_same_v<T, int64_t>) {
        data_type = INT64;
    } else if constexpr(std::is_arithmetic_v<T>) {
        data_type = INT_GENERIC;
    } else {
        data_type = UNSUPPORTED;
    }

    if (data_type == UNSUPPORTED) {
        data_type_size = 0;
    } else if (data_type == STRING) {
        data_type_size = 1;
    } else {
        data_type_size = sizeof(T);
    }
}

template<class T>
T DataColumn<T>::FromText(const pugi::xml_text& text) {
    // Parse properly based on template type or traits
    if constexpr(std::is_same_v<T, std::string>) {
        return text.as_string();
    } else if constexpr (std::is_same_v<T, double>) {
        return text.as_double(std::numeric_limits<T>::quiet_NaN());
    } else if constexpr (std::is_floating_point_v<T>) {
        return text.as_float(std::numeric_limits<T>::quiet_NaN());
    } else if constexpr(std::is_same_v<T, int64_t>) {
        return text.as_llong(0);
    } else if constexpr(std::is_arithmetic_v<T>) {
        return text.as_int(0);
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
void DataColumn<T>::FillFromBuffer(const uint8_t* ptr, int num_rows, size_t stride) {
    // Shifts by the column's offset
    ptr += data_offset;

    if (!stride || !data_type_size || num_rows > entries.size()) {
        return;
    }

    // Copy data from buffer, taking column stride into account
    T* val_ptr = entries.data();
    for (auto i = 0; i < num_rows; i++) {
        memcpy(val_ptr + i, ptr + stride * i, sizeof(T));
    }

    // Convert from big-endian to little-endian if the data type holds multiple bytes
    if (data_type_size == 2) {
        for (auto i = 0; i < num_rows; i++) {
            uint16_t* int_ptr = (uint16_t*) val_ptr + i;
            *int_ptr = __builtin_bswap16(*int_ptr);
        }
    } else if (data_type_size == 4) {
        for (auto i = 0; i < num_rows; i++) {
            uint32_t* int_ptr = (uint32_t*) val_ptr + i;
            *int_ptr = __builtin_bswap32(*int_ptr);
        }
    } else if (data_type_size == 8) {
        for (auto i = 0; i < num_rows; i++) {
            uint64_t* int_ptr = (uint64_t*) val_ptr + i;
            *int_ptr = __builtin_bswap64(*int_ptr);
        }
    }
}

template<class T>
void DataColumn<T>::Resize(size_t capacity) {
    entries.resize(capacity);
}

template<class T>
size_t DataColumn<T>::NumEntries() const {
    return entries.size();
}

template<class T>
void DataColumn<T>::SortIndices(IndexList& indices, bool ascending) const {
    if (indices.empty() || entries.empty()) {
        return;
    }

    // Perform ascending or descending sort
    if (ascending) {
        std::sort(indices.begin(), indices.end(), [&](int64_t a, int64_t b) {
            return entries[a] < entries[b];
        });
    } else {
        std::sort(indices.begin(), indices.end(), [&](int64_t a, int64_t b) {
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
