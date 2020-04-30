#ifndef VOTABLE_TEST__COLUMNS_H_
#define VOTABLE_TEST__COLUMNS_H_

#include <pugixml.hpp>
#include <string>
#include <vector>
#include <limits>
#include <cmath>
#include <type_traits>
#include <fmt/format.h>

namespace carta {

enum DataType {
    STRING,
    FLOAT,
    DOUBLE,
    INT,
    LONG,
    UNSUPPORTED
};

class Column {
public:
    virtual void FillEmpty() = 0;
    virtual void FillFromText(const pugi::xml_text& text) = 0;
    virtual void Reserve(size_t capacity) = 0;
    virtual std::string Info();
    virtual ~Column() = default;
    virtual Column* Clone() = 0;
    virtual void Append(Column*) = 0;

    // Factory for constructing a column from a <FIELD> node
    static Column* FromField(const pugi::xml_node& field);

    DataType data_type;
    std::string name;
    std::string id;
    std::string unit;
    std::string ucd;
    std::string ref;
    std::string description;
    std::string data_type_string;
};

class StringColumn : public Column {
public:
    std::vector<std::string> entries;
    StringColumn(const std::string& name_chr);
    ~StringColumn() override = default;
    void Reserve(size_t capacity) override;
    void FillFromText(const pugi::xml_text& text) override;
    void FillEmpty() override;
    void Append(Column* column) override {
        auto string_col = dynamic_cast<StringColumn*>(column);
        if (string_col) {
            entries.reserve(entries.size() + string_col->entries.size());
            entries.insert(entries.end(), string_col->entries.begin(), string_col->entries.end());
            string_col->entries.clear();
        }
    }
    Column* Clone() override {
        auto rhs = new StringColumn(name);
        rhs->id = id;
        rhs->unit = unit;
        rhs->description = description;
        rhs->data_type_string = data_type_string;
        return rhs;
    }
};

class UnsupportedColumn : public Column {
public:
    UnsupportedColumn(const std::string& name_chr);
    ~UnsupportedColumn() override = default;
    void Reserve(size_t capacity) override;
    void FillFromText(const pugi::xml_text&) override;
    void FillEmpty() override;
    void Append(Column*) override {};
    Column* Clone() override {
        auto rhs = new UnsupportedColumn(name);
        rhs->id = id;
        rhs->unit = unit;
        rhs->description = description;
        rhs->data_type_string = data_type_string;
        return rhs;
    }
};

template<class T>
class NumericColumn : public Column {
public:
    std::vector<T> entries;
    NumericColumn(const std::string& name_chr);
    ~NumericColumn() override = default;
    void Reserve(size_t capacity) override;
    void FillFromText(const pugi::xml_text& text) override;
    void FillEmpty() override;
    void Append(Column* column) override {
        auto numeric_col = dynamic_cast<NumericColumn<T>*>(column);
        if (numeric_col) {
            entries.reserve(entries.size() + numeric_col->entries.size());
            entries.insert(entries.end(), numeric_col->entries.begin(), numeric_col->entries.end());
            numeric_col->entries.clear();
        }
    }
    Column* Clone() override {
        auto rhs = new NumericColumn<T>(name);
        rhs->id = id;
        rhs->unit = unit;
        rhs->description = description;
        rhs->data_type_string = data_type_string;
        return rhs;
    }
};

}

#include "NumericColumn.tcc"

#endif //VOTABLE_TEST__COLUMNS_H_
