#include "Columns.h"

std::string Column::Info() {
    auto type_string = data_type == UNSUPPORTED ? fmt::format("{} (unsupported)", _data_type_string) : _data_type_string;
    auto unit_string = unit.empty() ? "" : fmt::format("Unit: {}; ", unit);
    auto description_string = description.empty() ? "" : fmt::format("Description: {}; ", description);
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

UnsupportedColumn::UnsupportedColumn(const std::string& name_chr, const std::string& type_chr) {
    name = name_chr;
    data_type = UNSUPPORTED;
    _data_type_string = type_chr;
}
void UnsupportedColumn::Reserve(size_t capacity) {
}
void UnsupportedColumn::FillFromText(const pugi::xml_text&) {
}
void UnsupportedColumn::FillEmpty() {

}
