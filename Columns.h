#ifndef VOTABLE_TEST__COLUMNS_H_
#define VOTABLE_TEST__COLUMNS_H_

#include <pugixml.hpp>
#include <string>
#include <vector>
#include <limits>
#include <cmath>
#include <fmt/format.h>

enum DataType {
    STRING,
    FLOAT,
    DOUBLE,
    INT,
    LONG,
    UNKNOWN
};

class Column {
public:
    virtual void FillEmpty() = 0;
    virtual void FillFromText(const pugi::xml_text& text) = 0;
    virtual void Reserve(size_t capacity) = 0;
    virtual std::string Info();
    virtual ~Column() = default;

    DataType data_type;
    std::string name;
    std::string id;
    std::string unit;
    std::string ucd;
    std::string ref;
    std::string description;
protected:
    std::string _data_type_string;
};

class StringColumn : public Column {
public:
    std::vector<std::string> entries;
    StringColumn(const std::string& name_chr);
    ~StringColumn() override = default;
    void Reserve(size_t capacity) override;
    void FillFromText(const pugi::xml_text& text) override;
    void FillEmpty() override;
};

class IntColumn : public Column {
public:
    std::vector<int32_t> entries;
    IntColumn(const std::string& name_chr);
    ~IntColumn() override = default;
    void Reserve(size_t capacity) override;
    void FillFromText(const pugi::xml_text& text) override;
    void FillEmpty() override;
};

class LongColumn : public Column {
public:
    std::vector<int64_t> entries;
    LongColumn(const std::string& name_chr);
    ~LongColumn() override = default;
    void Reserve(size_t capacity) override;
    void FillFromText(const pugi::xml_text& text) override;
    void FillEmpty() override;
};

class FloatColumn : public Column {
public:
    std::vector<float> entries;
    FloatColumn(const std::string& name_chr);
    ~FloatColumn() override = default;
    void Reserve(size_t capacity) override;
    void FillFromText(const pugi::xml_text& text) override;
    void FillEmpty() override;
};

class DoubleColumn : public Column {
public:
    std::vector<double> entries;
    DoubleColumn(const std::string& name_chr);
    ~DoubleColumn() override = default;
    void Reserve(size_t capacity) override;
    void FillFromText(const pugi::xml_text& text) override;
    void FillEmpty() override;
};

class DummyColumn : public Column {
public:
    DummyColumn(const std::string& name_chr, const std::string& type_chr);
    ~DummyColumn() override = default;
    void Reserve(size_t capacity) override;
    void FillFromText(const pugi::xml_text&) override;
    void FillEmpty() override;
};

#endif //VOTABLE_TEST__COLUMNS_H_
