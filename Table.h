#ifndef VOTABLE_TEST__TABLE_H_
#define VOTABLE_TEST__TABLE_H_

#include <string>
#include <vector>
#include <unordered_map>
#include "Columns.h"

#define MAX_HEADER_SIZE (64 * 1024)

namespace carta {

class Table {
public:
    Table(const std::string& filename, bool header_only = false);
    ~Table();
    bool IsValid() const;
    void PrintInfo(bool skip_unknowns = true);
    Column* GetColumnByIndex(int i);
    Column* GetColumnByName(const std::string& name);
    Column* GetColumnById(const std::string& id);
    Column* GetColumn(const std::string& name_or_id);
    size_t NumColumns();
    size_t NumRows();
protected:

    bool PopulateFields(const pugi::xml_node& table);
    bool PopulateRows(const pugi::xml_node& table);

    bool _valid;
    int64_t _num_rows;
    std::string _filename;
    std::vector<Column*> _columns;
    std::unordered_map<std::string, Column*> _column_name_map;
    std::unordered_map<std::string, Column*> _column_id_map;
    std::string GetHeader(const std::string& filename) const;
};
}
#endif //VOTABLE_TEST__TABLE_H_
