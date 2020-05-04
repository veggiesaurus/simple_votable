#ifndef VOTABLE_TEST__FILTER_H_
#define VOTABLE_TEST__FILTER_H_

#include "Columns.h"

namespace carta {

typedef std::vector<int64_t> IndexList;
enum LogicalOperator {
    AND,
    OR
};

IndexList StringFilter(Column* column, std::string search_string, bool case_insensitive);
IndexList NumericFilter(Column* column, double min_value, double max_value);
IndexList LogicalFilter(LogicalOperator op, const IndexList& A, const IndexList& B);
IndexList InvertIndices(const IndexList& indices, int64_t total_row_count);
bool SortByColumn(IndexList& indices, Column* column, bool ascending);

}

#endif //VOTABLE_TEST__FILTER_H_
