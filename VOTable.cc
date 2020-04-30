#include <fmt/format.h>
#include "VOTable.h"

using namespace std;

VOTable::VOTable(const string& filename)
    : _filename(filename) {
    pugi::xml_document doc;
    auto result = doc.load_file(_filename.c_str(), pugi::parse_default | pugi::parse_embed_pcdata);

    if (result) {
        auto votable = doc.child("VOTABLE");

        if (!votable) {
            fmt::print("Missing XML element VOTABLE\n");
            _valid = false;
            return;
        }

        auto resource = votable.child("RESOURCE");
        if (!votable) {
            fmt::print("Missing XML element RESOURCE\n");
            _valid = false;
            return;
        }

        auto table = resource.child("TABLE");
        if (!votable) {
            fmt::print("Missing XML element TABLE\n");
            _valid = false;
            return;
        }

        if (!PopulateFields(table)) {
            _valid = false;
            return;
        }

        // Reserve capacity for the appropriate number of rows
        size_t expected_num_rows = table.attribute("nrows").as_ullong();
        if (expected_num_rows > 0) {
            for (auto column: _columns) {
                column->Reserve(expected_num_rows);
            }
        }

        if (!PopulateRows(table)) {
            _valid = false;
            return;
        }

        _valid = true;
    } else {
        fmt::print("{}\n", result.description());
        _valid = false;
    }
}
bool VOTable::PopulateFields(const pugi::xml_node& table_node) {
    if (!table_node) {
        return false;
    }

    for (auto field: table_node.children("FIELD")) {
        auto data_type = field.attribute("datatype");
        string name = field.attribute("name").as_string();
        string array_size_string = field.attribute("arraysize").as_string();
        string type_string = data_type.as_string();

        Column* column;

        if (type_string == "char") {
            column = new StringColumn(name);
        } else if (!array_size_string.empty()) {
            // Can't support array-based column types other than char
            column = new UnsupportedColumn(name, fmt::format("{} [{}]", type_string, array_size_string));
        } else if (type_string == "int" || type_string == "short" || type_string == "unsignedByte") {
            column = new NumericColumn<int>(name);
        } else if (type_string == "long") {
            column = new NumericColumn<long>(name);
        } else if (type_string == "float") {
            column = new NumericColumn<float>(name);
        } else if (type_string == "double") {
            column = new NumericColumn<double>(name);
        } else {
            column = new UnsupportedColumn(name, type_string);
        }

        column->description = field.attribute("description").as_string();
        column->unit = field.attribute("unit").as_string();
        column->ucd = field.attribute("ucd").as_string();
        _columns.push_back(column);
        _column_map[name] = column;
    }

    return !_columns.empty();
}
bool VOTable::PopulateRows(const pugi::xml_node& table_node) {
    auto data = table_node.child("DATA");
    auto table_data = data.child("TABLEDATA");

    if (!table_data) {
        return false;
    }

    // VOTable standard specifies TABLEDATA element contains only TR children, which contain only TD children
    auto row_nodes = table_data.children();
    for (auto& row : row_nodes) {
        auto column_iterator = _columns.begin();
        auto column_nodes = row.children();
        for (auto& td: column_nodes) {
            if (column_iterator == _columns.end()) {
                fmt::print("Malformed VOTable: Too many TD entries for row {}", _num_rows);
                return false;
            }
            (*column_iterator)->FillFromText(td.text());
            column_iterator++;
        }

        // Fill remaining / missing columns
        while (column_iterator != _columns.end()) {
            (*column_iterator)->FillEmpty();
            column_iterator++;
        }
        _num_rows++;
    }
    return true;
}

bool VOTable::IsValid() const {
    return _valid;
}
void VOTable::PrintInfo(bool skip_unknowns) {
    fmt::print("Rows: {}; Columns: {};\n", _num_rows, _columns.size());
    for (auto column: _columns) {
        if (!skip_unknowns || column->data_type != UNSUPPORTED) {
            fmt::print(column->Info());
        }
    }
}

Column* VOTable::GetColumn(const std::string& name) {
    auto it = _column_map.find(name);
    if (it != _column_map.end()) {
        return it->second;
    }
    return nullptr;
}
Column* VOTable::GetColumn(int i) {
    if (i < _columns.size()) {
        return _columns[i];
    }
    return nullptr;
}
size_t VOTable::NumColumns() {
    return _columns.size();
}

size_t VOTable::NumRows() {
    return _num_rows;
}

VOTable::~VOTable() {
    for (auto column: _columns) {
        delete column;
    }
}
