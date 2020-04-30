#ifndef VOTABLE_TEST__NUMERICCOLUMN_TCC_
#define VOTABLE_TEST__NUMERICCOLUMN_TCC_

namespace carta {
template<class T>
NumericColumn<T>::NumericColumn(const std::string& name_chr) {
    if (std::is_same<T, float>::value) {
        data_type = FLOAT;
    } else if (std::is_same<T, double>::value) {
        data_type = DOUBLE;
    } else if (std::is_same<T, int>::value) {
        data_type = INT;
    } else if (std::is_same<T, int64_t>::value) {
        data_type = LONG;
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

}

#endif //VOTABLE_TEST__NUMERICCOLUMN_TCC_
