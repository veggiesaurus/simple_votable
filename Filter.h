#ifndef VOTABLE_TEST__FILTER_H_
#define VOTABLE_TEST__FILTER_H_

#include "Columns.h"

namespace carta {

class Filter {
public:
    virtual std::vector<int64_t> Execute() = 0;
    virtual bool IsValid() { return _valid; }
protected:
    bool _valid = false;
};

class StringFilter : public Filter {
public:
    StringFilter(Column* column, const std::string& search_string, bool case_insensitive);
    std::vector<int64_t> Execute() override;

protected:
    std::string _search_string;
    bool _case_insensitive;
    StringColumn* _column;
};

class NumericFilter : public Filter {
public:
    NumericFilter(Column* column, double min_value, double max_value);
    std::vector<int64_t> Execute() override;

protected:
    double _min_value;
    double _max_value;
    Column* _column;
    template<class T>
    std::vector<int64_t> TemplatedExecute();
};

class LogicalFilter : public Filter {
public:
    enum LogicalOperator {
        AND,
        OR
    };

    LogicalFilter(LogicalOperator op, std::vector<Filter*> filters);
    LogicalFilter(LogicalOperator op, std::initializer_list<Filter*> filters);
    bool IsValid() override;
    std::vector<int64_t> Execute() override;
protected:
    std::vector<Filter*> _filters;
    LogicalOperator _operator;
};

}

#endif //VOTABLE_TEST__FILTER_H_
