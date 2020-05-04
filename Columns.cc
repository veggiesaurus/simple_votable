#include <numeric>
#include "Columns.h"

namespace carta {
using namespace std;

Column* Column::FromField(const pugi::xml_node& field) {
    auto data_type = field.attribute("datatype");
    string name = field.attribute("name").as_string();
    string array_size_string = field.attribute("arraysize").as_string();
    string type_string = data_type.as_string();

    Column* column;

    if (type_string == "char") {
        column = new StringColumn(name);
    } else if (!array_size_string.empty()) {
        // Can't support array-based column types other than char
        column = new UnsupportedColumn(name);
    } else if (type_string == "int" || type_string == "short" || type_string == "unsignedByte") {
        column = new NumericColumn<int>(name);
    } else if (type_string == "long") {
        column = new NumericColumn<long>(name);
    } else if (type_string == "float") {
        column = new NumericColumn<float>(name);
    } else if (type_string == "double") {
        column = new NumericColumn<double>(name);
    } else {
        column = new UnsupportedColumn(name);
    }

    if (!array_size_string.empty()) {
        column->data_type_string = fmt::format("{}[{}]", type_string, array_size_string);
    } else {
        column->data_type_string = type_string;
    }

    column->id = field.attribute("ID").as_string();
    column->description = field.attribute("description").as_string();
    column->unit = field.attribute("unit").as_string();
    column->ucd = field.attribute("ucd").as_string();
    return column;
}

std::vector<int64_t> Column::InvertIndices(const vector<int64_t>& indices, int64_t total_row_count) {
    vector<int64_t> inverted_indices;
    auto inverted_row_count = total_row_count - indices.size();

    // after inversion, all indices are included
    if (indices.empty()) {
        inverted_indices.resize(indices.size());
        std::iota(inverted_indices.begin(), inverted_indices.end(), 0);
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

#pragma region StringColumn

string Column::Info() {
    auto type_string = data_type == UNSUPPORTED ? fmt::format("{} (unsupported)", data_type_string) : data_type_string;
    auto unit_string = unit.empty() ? "" : fmt::format("Unit: {}; ", unit);
    auto description_string = description.empty() ? "" : fmt::format("Description: {}; ", description);
    return fmt::format("Name: {}; Type: {}; {}{}\n", name, type_string, unit_string, description_string);
}

StringColumn::StringColumn(const string& name_chr) {
    name = name_chr;
    data_type = STRING;
}

void StringColumn::SetFromText(const pugi::xml_text& text, size_t index) {
    if (!text.empty()) {
        entries[index] = text.get();
    } else {
        SetEmpty(index);
    }
}

void StringColumn::SetEmpty(size_t index) {
    entries[index] = "";
}

void StringColumn::FillFromText(const pugi::xml_text& text) {
    if (!text.empty()) {
        entries.emplace_back(text.get());
    } else {
        FillEmpty();
    }
}

void StringColumn::FillEmpty() {
    entries.emplace_back("");
}

void StringColumn::Resize(size_t capacity) {
    entries.resize(capacity);
}

void StringColumn::Reserve(size_t capacity) {
    entries.reserve(capacity);
}

std::vector<int64_t> StringColumn::GetFilteredIndices(double min_value, double max_value, int64_t start_index, int64_t end_index) {
    return std::vector<int64_t>();
}

std::vector<int64_t> StringColumn::GetFilteredIndices(std::string search_string, bool case_insensitive, int64_t start_index, int64_t end_index) {
    std::vector<int64_t> matching_indices;

    if (case_insensitive) {
        std::transform(search_string.begin(), search_string.end(), search_string.begin(), ::tolower);
    }

    if (start_index < 0 || start_index >= entries.size()) {
        start_index = 0;
    }

    if (end_index < 0 || end_index >= entries.size()) {
        end_index = entries.size() - 1;
    }

    if (case_insensitive) {
        for (auto i = start_index; i < end_index; i++) {
            auto val = entries[i];
            std::transform(val.begin(), val.end(), val.begin(), ::tolower);
            if (val.find(search_string) != string::npos) {
                matching_indices.push_back(i);
            }
        }
    } else {
        for (auto i = start_index; i < end_index; i++) {
            auto& val = entries[i];
            if (val.find(search_string) != string::npos) {
                matching_indices.push_back(i);
            }
        }
    }

    return matching_indices;
}

#pragma endregion

#pragma region UnsupportedColumn

UnsupportedColumn::UnsupportedColumn(const string& name_chr) {
    name = name_chr;
    data_type = UNSUPPORTED;
}

void UnsupportedColumn::Resize(size_t capacity) {
}

void UnsupportedColumn::Reserve(size_t capacity) {
}

void UnsupportedColumn::SetFromText(const pugi::xml_text&, size_t) {
}

void UnsupportedColumn::SetEmpty(std::size_t) {
}

void UnsupportedColumn::FillFromText(const pugi::xml_text& text) {
}

void UnsupportedColumn::FillEmpty() {
}

std::vector<int64_t> UnsupportedColumn::GetFilteredIndices(double min_value, double max_value, int64_t start_index, int64_t end_index) {
    return std::vector<int64_t>();
}

std::vector<int64_t> UnsupportedColumn::GetFilteredIndices(std::string search_string, bool case_sensitive, int64_t start_index, int64_t end_index) {
    return std::vector<int64_t>();
}

#pragma endregion

}
