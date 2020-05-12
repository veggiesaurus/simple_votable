#ifndef VOTABLE_TEST__TABLEVIEW_H_
#define VOTABLE_TEST__TABLEVIEW_H_

#include "Table.h"

namespace carta {

class Table;

class TableView {
public:
    TableView(const Table* table);
    TableView(const Table* table, const IndexList& index_list, bool ordered = true);

    // Filtering
    bool NumericFilter(Column* column, double min_value, double max_value);
    bool NumericFilter(int64_t column_index, double min_value, double max_value);
    bool NumericFilter(const std::string& column_name_or_id, double min_value, double max_value);

    bool StringFilter(Column* column, std::string search_string, bool case_insensitive);
    bool StringFilter(int64_t column_index, std::string search_string, bool case_insensitive);
    bool StringFilter(const std::string& column_name_or_id, std::string search_string, bool case_insensitive);

    bool Invert();
    bool Combine(const TableView& second);

    // Sorting
    bool SortByColumn(Column* column, bool ascending = true);
    bool SortByColumn(int64_t column_index, bool ascending = true);
    bool SortByColumn(const std::string& column_name_or_id, bool ascending = true);

    bool SortByIndex();

    // Retrieving data
    size_t NumRows() const;
    template<class T>
    std::vector<T> NumericValues(Column* column, int64_t start = -1, int64_t end = -1) const;
    template<class T>
    std::vector<T> NumericValues(int64_t column_index, int64_t start = -1, int64_t end = -1) const;
    template<class T>
    std::vector<T> NumericValues(const std::string& column_name_or_id, int64_t start = -1, int64_t end = -1) const;
    std::vector<std::string> StringValues(Column* column, int64_t start = -1, int64_t end = -1) const;
    std::vector<std::string> StringValues(int64_t column_index, int64_t start = -1, int64_t end = -1) const;
    std::vector<std::string> StringValues(const std::string& column_name_or_id, int64_t start = -1, int64_t end = -1) const;

protected:
    template<class T>
    bool NumericFilterTemplated(Column* column, double min_value, double max_value);
    template<class T>
    bool SortByNumericColumn(Column* column, bool ascending);
    bool SortByStringColumn(Column* column, bool ascending);

    bool _is_subset;
    bool _ordered;
    IndexList _subset_indices;
    const Table* _table;

};
}

#include "TableView.tcc"

#endif //VOTABLE_TEST__TABLEVIEW_H_