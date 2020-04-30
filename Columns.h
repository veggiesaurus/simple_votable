#ifndef VOTABLE_TEST__COLUMNS_H_
#define VOTABLE_TEST__COLUMNS_H_

#include <pugixml.hpp>
#include <string>
#include <vector>
#include <limits>
#include <cmath>
#include <type_traits>
#include <fmt/format.h>

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

class UnsupportedColumn : public Column {
public:
    UnsupportedColumn(const std::string& name_chr, const std::string& type_chr);
    ~UnsupportedColumn() override = default;
    void Reserve(size_t capacity) override;
    void FillFromText(const pugi::xml_text&) override;
    void FillEmpty() override;
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
};

template<class T>
NumericColumn<T>::NumericColumn(const std::string& name_chr) {
    if (std::is_same<T, float>::value) {
        data_type = FLOAT;
        _data_type_string = "float";
    } else if (std::is_same<T, double>::value) {
        data_type = DOUBLE;
        _data_type_string = "double";
    }else if (std::is_same<T, int>::value) {
        data_type = INT;
        _data_type_string = "int";
    }else if (std::is_same<T, int64_t>::value) {
        data_type = LONG;
        _data_type_string = "long";
    }
    name = name_chr;
}

template<class T>
void NumericColumn<T>::Reserve(size_t capacity) {
    entries.reserve(capacity);
}

template<class T>
void NumericColumn<T>::FillFromText(const pugi::xml_text& text) {
    if (data_type == FLOAT) {
        entries.emplace_back(text.as_float((float) std::numeric_limits<T>::quiet_NaN()));
    } else if (data_type == DOUBLE) {
        entries.emplace_back(text.as_double((double) std::numeric_limits<T>::quiet_NaN()));
    } else if (data_type == INT) {
        entries.emplace_back(text.as_int((int) std::numeric_limits<T>::quiet_NaN()));
    } else if (data_type == LONG) {
        entries.emplace_back(text.as_llong((int64_t) std::numeric_limits<T>::quiet_NaN()));
    }
}

template<class T>
void NumericColumn<T>::FillEmpty() {
    entries.emplace_back(std::numeric_limits<T>::quiet_NaN());
}

#endif //VOTABLE_TEST__COLUMNS_H_
