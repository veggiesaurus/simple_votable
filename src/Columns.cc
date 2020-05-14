#include <memory>
#include "Columns.h"
#include "DataColumn.tcc"

namespace carta {
using namespace std;

Column::Column(const string& name_chr) {
    name = name_chr;
    data_type = UNSUPPORTED;
}

std::unique_ptr<Column> Column::FromField(const pugi::xml_node& field) {
    auto data_type = field.attribute("datatype");
    string name = field.attribute("name").as_string();
    string array_size_string = field.attribute("arraysize").as_string();
    string type_string = data_type.as_string();

    unique_ptr<Column> column;

    if (type_string == "char") {
        column = make_unique<DataColumn<string>>(name);
    } else if (!array_size_string.empty()) {
        // Can't support array-based column types other than char
        column = make_unique<Column>(name);
    } else if (type_string == "int") {
        column = make_unique<DataColumn<int>>(name);
    } else if (type_string == "short") {
        column = make_unique<DataColumn<int16_t>>(name);
    } else if (type_string == "unsignedByte") {
        column = make_unique<DataColumn<uint8_t>>(name);
    } else if (type_string == "long") {
        column = make_unique<DataColumn<int64_t>>(name);
    } else if (type_string == "float") {
        column = make_unique<DataColumn<float>>(name);
    } else if (type_string == "double") {
        column = make_unique<DataColumn<double>>(name);
    } else {
        column = make_unique<Column>(name);
    }

    column->id = field.attribute("ID").as_string();
    column->description = field.attribute("description").as_string();
    column->unit = field.attribute("unit").as_string();
    column->ucd = field.attribute("ucd").as_string();
    return column;
}

string Column::Info() {
    auto type_string = data_type == UNSUPPORTED ? "unsupported" : data_type == STRING ? "string" : fmt::format("{} bytes per entry", data_type_size);
    auto unit_string = unit.empty() ? "" : fmt::format("Unit: {}; ", unit);
    auto description_string = description.empty() ? "" : fmt::format("Description: {}; ", description);
    return fmt::format("Name: {}; Data: {}; {}{}\n", name, type_string, unit_string, description_string);
}

}
