#ifndef VOTABLE_TEST__VOTABLE_H_
#define VOTABLE_TEST__VOTABLE_H_

#include <string>
#include <vector>
#include <unordered_map>
#include "Columns.h"

class VOTable {
public:
    VOTable(const std::string& filename);
    ~VOTable();
    bool IsValid() const;
    void PrintInfo(bool skip_unknowns = true);
    Column* GetColumn(int i);
    Column* GetColumn(const std::string& name);
    size_t NumColumns();
    size_t NumRows();
protected:

    bool PopulateFields(const pugi::xml_node& table_node);
    bool PopulateRows(const pugi::xml_node& table_node);

    bool _valid;
    int64_t _num_rows;
    std::string _filename;
    std::vector<Column*> _columns;
    std::unordered_map<std::string, Column*> _column_map;
};

#endif //VOTABLE_TEST__VOTABLE_H_
