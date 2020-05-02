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
    virtual void SetFromText(const pugi::xml_text& text, size_t index) = 0;
    virtual void SetEmpty(size_t index) = 0;
    virtual void FillFromText(const pugi::xml_text& text) = 0;
    virtual void FillEmpty() = 0;
    virtual void Resize(size_t capacity) = 0;
    virtual void Reserve(size_t capacity) = 0;
    virtual std::string Info();
    virtual ~Column() = default;

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
    void SetFromText(const pugi::xml_text& text, size_t index) override;
    void SetEmpty(size_t index) override;
    void FillFromText(const pugi::xml_text& text) override;
    void FillEmpty() override;
    void Resize(size_t capacity) override;
    void Reserve(size_t capacity) override;
};

class UnsupportedColumn : public Column {
public:
    UnsupportedColumn(const std::string& name_chr);
    ~UnsupportedColumn() override = default;
    void SetFromText(const pugi::xml_text&, size_t index) override;
    void SetEmpty(size_t index) override;
    void FillFromText(const pugi::xml_text& text) override;
    void FillEmpty() override;
    void Resize(size_t capacity) override;
    void Reserve(size_t capacity) override;
};

template<class T>
class NumericColumn : public Column {
public:
    std::vector<T> entries;
    NumericColumn(const std::string& name_chr);
    ~NumericColumn() override = default;
    void SetFromText(const pugi::xml_text& text, size_t index) override;
    void SetEmpty(size_t index) override;
    void FillFromText(const pugi::xml_text& text) override;
    void FillEmpty() override;
    void Resize(size_t capacity) override;
    void Reserve(size_t capacity) override;
};
}

#include "NumericColumn.tcc"

#endif //VOTABLE_TEST__COLUMNS_H_
