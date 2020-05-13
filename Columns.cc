#include <memory>
#include <numeric>
#include "Columns.h"

namespace carta {
using namespace std;

std::unique_ptr<Column> Column::FromField(const pugi::xml_node& field) {
    auto data_type = field.attribute("datatype");
    string name = field.attribute("name").as_string();
    string array_size_string = field.attribute("arraysize").as_string();
    string type_string = data_type.as_string();

    unique_ptr<Column> column;

    if (type_string == "char") {
        column = make_unique<StringColumn>(name);
    } else if (!array_size_string.empty()) {
        // Can't support array-based column types other than char
        column = make_unique<UnsupportedColumn>(name);
    } else if (type_string == "int" || type_string == "short" || type_string == "unsignedByte") {
        column = make_unique<NumericColumn<int>>(name);
    } else if (type_string == "long") {
        column = make_unique<NumericColumn<int64_t>>(name);
    } else if (type_string == "float") {
        column = make_unique<NumericColumn<float>>(name);
    } else if (type_string == "double") {
        column = make_unique<NumericColumn<double>>(name);
    } else {
        column = make_unique<UnsupportedColumn>(name);
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

#pragma endregion

}
