#ifndef VOTABLE_TEST__TABLE_H_
#define VOTABLE_TEST__TABLE_H_

#include <string>
#include <vector>
#include <unordered_map>
#include "Columns.h"
#include "TableView.h"

#define MAX_HEADER_SIZE (64 * 1024)

namespace carta {

class TableView;

class Table {
public:
    Table(const std::string& filename, bool header_only = false);
    bool IsValid() const;
    void PrintInfo(bool skip_unknowns = true) const;
    const Column* GetColumnByIndex(int i) const;
    const Column* GetColumnByName(const std::string& name) const;
    const Column* GetColumnById(const std::string& id) const;
    const Column* GetColumn(const std::string& name_or_id) const;
    size_t NumColumns() const;
    size_t NumRows() const;
    TableView View() const;
protected:

    bool PopulateFields(const pugi::xml_node& table);
    bool PopulateRows(const pugi::xml_node& table);

    bool _valid;
    int64_t _num_rows;
    std::string _filename;
    std::vector<std::unique_ptr<Column>> _columns;
    std::unordered_map<std::string, Column*> _column_name_map;
    std::unordered_map<std::string, Column*> _column_id_map;
    static std::string GetHeader(const std::string& filename);
};
}
#endif //VOTABLE_TEST__TABLE_H_
