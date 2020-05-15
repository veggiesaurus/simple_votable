#ifndef VOTABLE_TEST__COLUMNS_H_
#define VOTABLE_TEST__COLUMNS_H_

#include <string>
#include <vector>
#include <limits>
#include <cmath>
#include <type_traits>
#include <pugixml.hpp>
#include <fitsio.h>
#include <fmt/format.h>

namespace carta {

typedef std::vector<int64_t> IndexList;
template<class T>
class DataColumn;

enum DataType {
    STRING,
    DOUBLE,
    FLOAT_GENERIC,
    INT64,
    INT_GENERIC,
    UNSUPPORTED
};

class Column {
public:
    Column(const std::string& name_chr);
    virtual ~Column() = default;
    virtual void SetFromText(const pugi::xml_text& text, size_t index) {};
    virtual void SetEmpty(size_t index) {};
    virtual void FillFromText(const pugi::xml_text& text) {};
    virtual void FillEmpty() {};
    virtual void FillFromBuffer(const uint8_t * ptr, int num_rows, size_t stride) {};
    virtual void Resize(size_t capacity) {};
    virtual void Reserve(size_t capacity) {};
    virtual size_t NumEntries() const { return 0; };
    virtual void SortIndices(IndexList& indices, bool ascending) const {};
    virtual void FilterIndices(IndexList& existing_indices, bool is_subset, double min_val, double max_val) const {}
    virtual std::string Info();

    // Factory for constructing a column from a <FIELD> node
    static std::unique_ptr<Column> FromField(const pugi::xml_node& field);
    static std::unique_ptr<Column> FromFITS(const std::string& col_name, const std::string& unit, int col_type, int repeat, size_t data_offset);
    static std::unique_ptr<Column> FromFitsPtr(fitsfile* fits_ptr, int column_index, size_t& data_offset);

    DataType data_type;
    std::string name;
    std::string id;
    std::string unit;
    std::string ucd;
    std::string ref;
    std::string description;
    std::string data_type_string;
    size_t data_type_size;
    size_t data_offset;
};

template<class T>
class DataColumn : public Column {
public:
    std::vector<T> entries;
    DataColumn(const std::string& name_chr);
    virtual ~DataColumn() = default;
    void SetFromText(const pugi::xml_text& text, size_t index) override;
    void SetEmpty(size_t index) override;
    void FillFromText(const pugi::xml_text& text) override;
    void FillEmpty() override;
    void FillFromBuffer(const uint8_t * ptr, int num_rows, size_t stride) override;
    void Resize(size_t capacity) override;
    void Reserve(size_t capacity) override;
    size_t NumEntries() const override;
    void SortIndices(IndexList& indices, bool ascending) const override;
    void FilterIndices(IndexList& existing_indices, bool is_subset, double min_value, double max_value) const override;

    static const DataColumn<T>* TryCast(const Column* column) {
        if (!column || column->data_type == UNSUPPORTED) {
            return nullptr;
        }
        return dynamic_cast<const DataColumn<T>*>(column);
    }
protected:
    T FromText(const pugi::xml_text& text);
};
}

#endif //VOTABLE_TEST__COLUMNS_H_