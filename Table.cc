#include <fstream>
#include <iostream>
#include <fmt/format.h>
#include "Table.h"

namespace carta {
using namespace std;

Table::Table(const string& filename, bool header_only)
    : _filename(filename), _num_rows(0) {
    pugi::xml_document doc;

    // read the first 64K only and construct a header from this
    if (header_only) {
        string header_string = GetHeader(filename);
        auto result = doc.load_string(header_string.c_str(), pugi::parse_default | pugi::parse_fragment);
        if (!result && result.status != pugi::status_end_element_mismatch) {
            fmt::print("{}\n", result.description());
            _valid = false;
            return;
        }
    } else {
        auto result = doc.load_file(_filename.c_str(), pugi::parse_default | pugi::parse_embed_pcdata);
        if (!result) {
            fmt::print("{}\n", result.description());
            _valid = false;
            return;
        }
    }

    auto votable = doc.child("VOTABLE");

    if (!votable) {
        fmt::print("Missing XML element VOTABLE\n");
        _valid = false;
        return;
    }

    auto resource = votable.child("RESOURCE");
    if (!resource) {
        fmt::print("Missing XML element RESOURCE\n");
        _valid = false;
        return;
    }

    auto table = resource.child("TABLE");
    if (!table) {
        fmt::print("Missing XML element TABLE\n");
        _valid = false;
        return;
    }

    if (!PopulateFields(table)) {
        _valid = false;
        return;
    }

    // Once fields are populated, stop parsing
    if (header_only) {
        _valid = true;
        return;
    }

    if (!PopulateRows(table)) {
        _valid = false;
        return;
    }

    _valid = true;
}

string Table::GetHeader(const string& filename) {
    ifstream in(filename);
    string header_string;
    // Measure entire file size to ensure we don't read past EOF
    in.seekg(0, ios_base::end);
    size_t header_size = min(size_t(in.tellg()), size_t(MAX_HEADER_SIZE));
    header_string.resize(header_size);
    in.seekg(0, ios_base::beg);
    in.read(&header_string[0], header_string.size());
    in.close();

    // Resize to exclude the start of the <DATA> tag
    auto data_index = header_string.find("<DATA>");
    if (data_index != string::npos) {
        header_string.resize(data_index);
    }
    return header_string;
}

bool Table::PopulateFields(const pugi::xml_node& table) {
    if (!table) {
        return false;
    }

    for (auto& field: table.children("FIELD")) {
        Column* column = Column::FromField(field);
        _columns.push_back(column);
        if (!column->name.empty()) {
            _column_name_map[column->name] = column;
        }
        if (!column->id.empty()) {
            _column_id_map[column->id] = column;
        }
    }

    return !_columns.empty();
}

bool Table::PopulateRows(const pugi::xml_node& table) {
    auto data = table.child("DATA");
    auto table_data = data.child("TABLEDATA");
    if (!table_data) {
        return false;
    }

    // VOTable standard specifies TABLEDATA element contains only TR children, which contain only TD children
    auto row_nodes = table_data.children();

    std::vector<pugi::xml_node> rows;
    for (auto& row : row_nodes) {
        rows.push_back(row);
    }

    _num_rows = rows.size();
    for (auto column: _columns) {
        column->Resize(_num_rows);
    }

#pragma omp parallel default(none) shared(_num_rows, rows)
    {
#pragma omp for schedule(static)
        for (auto i = 0; i < _num_rows; i++) {
            auto& row = rows[i];
            auto column_iterator = _columns.begin();
            auto column_nodes = row.children();
            for (auto& td: column_nodes) {
                if (column_iterator == _columns.end()) {
                    break;
                }
                (*column_iterator)->SetFromText(td.text(), i);
                column_iterator++;
            }

            // Fill remaining / missing columns
            while (column_iterator != _columns.end()) {
                (*column_iterator)->SetEmpty(i);
                column_iterator++;
            }
        }
    }
    return true;
}

bool Table::IsValid() const {
    return _valid;
}

void Table::PrintInfo(bool skip_unknowns) const {
    fmt::print("Rows: {}; Columns: {};\n", _num_rows, _columns.size());
    for (auto column: _columns) {
        if (!skip_unknowns || column->data_type != UNSUPPORTED) {
            fmt::print(column->Info());
        }
    }
}

Column* Table::GetColumnByIndex(int i) const {
    if (i < _columns.size()) {
        return _columns[i];
    }
    return nullptr;
}

Column* Table::GetColumnByName(const std::string& name) const {
    auto it = _column_name_map.find(name);
    if (it != _column_name_map.end()) {
        return it->second;
    }
    return nullptr;
}

Column* Table::GetColumnById(const std::string& id) const {
    auto it = _column_id_map.find(id);
    if (it != _column_id_map.end()) {
        return it->second;
    }
    return nullptr;
}

Column* Table::GetColumn(const string& name_or_id) const {
    // Search first by ID and then by name
    auto id_result = GetColumnById(name_or_id);
    if (id_result) {
        return id_result;
    }
    return GetColumnByName(name_or_id);
}

size_t Table::NumColumns() const {
    return _columns.size();
}

size_t Table::NumRows() const {
    return _num_rows;
}

Table::~Table() {
    for (auto column: _columns) {
        delete column;
    }
}

TableView Table::View() const {
    return TableView(this);
}

}