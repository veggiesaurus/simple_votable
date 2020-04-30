#include "Columns.h"

std::string Column::Info() {
    auto type_string = data_type == UNKNOWN ? fmt::format("{} (unsupported)", _data_type_string) : _data_type_string;
    auto unit_string = unit.empty() ? "": fmt::format("Unit: {}; ", unit);
    auto description_string = description.empty() ? "": fmt::format("Description: {}; ", description);
    return fmt::format("Name: {}; Type: {}; {}{}\n", name, type_string, unit_string, description_string);
}

StringColumn::StringColumn(const std::string& name_chr) {
    name = name_chr;
    data_type = STRING;
    _data_type_string = "string";
}
void StringColumn::Reserve(size_t capacity) {
    entries.reserve(capacity);
}
void StringColumn::FillFromText(const pugi::xml_text& text) {
    if (!text.empty()) {
        entries.emplace_back(text.as_string());
    } else {
        FillEmpty();
    }
}
void StringColumn::FillEmpty() {
    entries.emplace_back("");
}

IntColumn::IntColumn(const std::string& name_chr) {
    name = name_chr;
    data_type = INT;
    _data_type_string = "int32";
}
void IntColumn::Reserve(size_t capacity) {
    entries.reserve(capacity);
}
void IntColumn::FillFromText(const pugi::xml_text& text) {
    if (!text.empty()) {
        entries.emplace_back(text.as_int());
    } else {
        FillEmpty();
    }
}
void IntColumn::FillEmpty() {
    entries.emplace_back(0);
}

LongColumn::LongColumn(const std::string& name_chr) {
    name = name_chr;
    data_type = LONG;
    _data_type_string = "int64";
}
void LongColumn::Reserve(size_t capacity) {
    entries.reserve(capacity);
}
void LongColumn::FillFromText(const pugi::xml_text& text) {
    if (!text.empty()) {
        entries.emplace_back(text.as_llong());
    } else {
        FillEmpty();
    }
}
void LongColumn::FillEmpty() {
    entries.emplace_back(0);
}

FloatColumn::FloatColumn(const std::string& name_chr) {
    name = name_chr;
    data_type = FLOAT;
    _data_type_string = "float";
}
void FloatColumn::Reserve(size_t capacity) {
    entries.reserve(capacity);
}
void FloatColumn::FillFromText(const pugi::xml_text& text) {
    if (!text.empty()) {
        entries.emplace_back(text.as_float(NAN));
    } else {
        FillEmpty();
    }
}
void FloatColumn::FillEmpty() {
    entries.emplace_back(NAN);
}

DoubleColumn::DoubleColumn(const std::string& name_chr) {
    name = name_chr;
    data_type = DOUBLE;
    _data_type_string = "double";
}
void DoubleColumn::Reserve(size_t capacity) {
    entries.reserve(capacity);
}
void DoubleColumn::FillFromText(const pugi::xml_text& text) {
    if (!text.empty()) {
        entries.emplace_back(text.as_double(NAN));
    } else {
        FillEmpty();
    }
}
void DoubleColumn::FillEmpty() {
    entries.emplace_back(NAN);
}

DummyColumn::DummyColumn(const std::string& name_chr, const std::string& type_chr) {
    name = name_chr;
    data_type = UNKNOWN;
    _data_type_string = type_chr;
}
void DummyColumn::Reserve(size_t capacity) {
}
void DummyColumn::FillFromText(const pugi::xml_text&) {
}
void DummyColumn::FillEmpty() {

}
