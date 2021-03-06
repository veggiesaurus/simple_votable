#include <gtest/gtest.h>
#include <fmt/format.h>

#include "Table.h"

using namespace std;
using namespace carta;

const string test_base = "../test/test_fits";

string test_path(const string& filename) {
    return fmt::format("{}/{}", test_base, filename);
}
TEST(BasicParsing, ParseIvoaExampleHeaderOnly) {
    Table table(test_path("ivoa_example.fits"), true);
    EXPECT_TRUE(table.IsValid());
    EXPECT_EQ(table.NumRows(), 0);
}

TEST(BasicParsing, ParseIvoaExample) {
    Table table(test_path("ivoa_example.fits"));
    EXPECT_TRUE(table.IsValid());
    EXPECT_EQ(table.NumRows(), 3);
}

TEST(ParsedTable, CorrectFieldCount) {
    Table table(test_path("ivoa_example.fits"));
    EXPECT_TRUE(table.IsValid());
    EXPECT_EQ(table.NumColumns(), 6);
}

TEST(ParsedTable, CorrectFieldNames) {
    Table table(test_path("ivoa_example.fits"));
    EXPECT_EQ(table[0]->name, "RA");
    EXPECT_EQ(table[1]->name, "Dec");
    EXPECT_EQ(table[2]->name, "Name");
    EXPECT_EQ(table[3]->name, "RVel");
    EXPECT_EQ(table[4]->name, "e_RVel");
    EXPECT_EQ(table[5]->name, "R");
}

TEST(ParsedTable, CorrectFieldUnits) {
    Table table(test_path("ivoa_example.fits"));
    EXPECT_EQ(table[0]->unit, "deg");
    EXPECT_EQ(table[1]->unit, "deg");
    EXPECT_TRUE(table[2]->unit.empty());
    EXPECT_EQ(table[3]->unit, "km/s");
    EXPECT_EQ(table[4]->unit, "km/s");
    EXPECT_EQ(table[5]->unit, "Mpc");
}

TEST(ParsedTable, CorrectFieldTypes) {
    Table table(test_path("ivoa_example.fits"));
    EXPECT_EQ(table[0]->data_type, FLOAT);
    EXPECT_EQ(table[1]->data_type, FLOAT);
    EXPECT_EQ(table[2]->data_type, STRING);
    EXPECT_EQ(table[3]->data_type, INT32);
    EXPECT_EQ(table[4]->data_type, INT16);
    EXPECT_EQ(table[5]->data_type, FLOAT);
}

TEST(ParsedTable, CorrectFieldSizes) {
    Table table(test_path("ivoa_example.fits"));
    EXPECT_EQ(table[0]->data_type_size, 4);
    EXPECT_EQ(table[1]->data_type_size, 4);
    EXPECT_EQ(table[2]->data_type_size, 6);
    EXPECT_EQ(table[3]->data_type_size, 4);
    EXPECT_EQ(table[4]->data_type_size, 2);
    EXPECT_EQ(table[5]->data_type_size, 4);
}

TEST(ParsedTable, CorrectNameLookups) {
    Table table(test_path("ivoa_example.fits"));
    EXPECT_EQ(table["RA"]->name, "RA");
    EXPECT_EQ(table["Dec"]->name, "Dec");
    EXPECT_EQ(table["Name"]->name, "Name");
    EXPECT_EQ(table["RVel"]->name, "RVel");
    EXPECT_EQ(table["e_RVel"]->name, "e_RVel");
    EXPECT_EQ(table["R"]->name, "R");
    EXPECT_EQ(table["dummy"], nullptr);
    EXPECT_EQ(table[""], nullptr);
}

TEST(ParsedTable, CorrectColumnTypes) {
    Table table(test_path("ivoa_example.fits"));
    EXPECT_NE(DataColumn<float>::TryCast(table["RA"]), nullptr);
    EXPECT_EQ(DataColumn<double>::TryCast(table["RA"]), nullptr);

    EXPECT_NE(DataColumn<string>::TryCast(table["Name"]), nullptr);
    EXPECT_EQ(DataColumn<int>::TryCast(table["Name"]), nullptr);

    EXPECT_NE(DataColumn<int>::TryCast(table["RVel"]), nullptr);
    EXPECT_EQ(DataColumn<string>::TryCast(table["RVel"]), nullptr);

    EXPECT_NE(DataColumn<int16_t>::TryCast(table["e_RVel"]), nullptr);
    EXPECT_EQ(DataColumn<int>::TryCast(table["e_RVel"]), nullptr);
}

TEST(ParsedTable, CorrectDataValues) {
    Table table(test_path("ivoa_example.fits"));

    auto& col1_vals = DataColumn<float>::TryCast(table["RA"])->entries;
    EXPECT_EQ(col1_vals.size(), 3);
    EXPECT_FLOAT_EQ(col1_vals[0], 10.68f);
    EXPECT_FLOAT_EQ(col1_vals[1], 287.43f);

    auto& col2_vals = DataColumn<float>::TryCast(table["Dec"])->entries;
    EXPECT_EQ(col2_vals.size(), 3);
    EXPECT_FLOAT_EQ(col2_vals[0], 41.27f);
    EXPECT_FLOAT_EQ(col2_vals[1], -63.85f);

    auto& col3_vals = DataColumn<string>::TryCast(table["Name"])->entries;
    EXPECT_EQ(col3_vals.size(), 3);
    EXPECT_EQ(col3_vals[0], "N 224");
    EXPECT_EQ(col3_vals[1], "N 6744");

    auto& col5_vals = DataColumn<int16_t>::TryCast(table["e_RVel"])->entries;
    EXPECT_EQ(col5_vals.size(), 3);
    EXPECT_EQ(col5_vals[0], 5);
    EXPECT_EQ(col5_vals[1], 6);
}

TEST(Filtering, FailOnWrongFilterType) {
    Table table(test_path("ivoa_example.fits"));
    EXPECT_FALSE(table.View().StringFilter(table["dummy"], "N 224"));
    EXPECT_FALSE(table.View().StringFilter(table["RA"], "N 224"));

    EXPECT_FALSE(table.View().NumericFilter(table["dummy"], RANGE_INCLUSIVE, 0, 100));
    EXPECT_FALSE(table.View().NumericFilter(table["Name"], RANGE_INCLUSIVE, 0, 100));
}

TEST(Filtering, PassOnCorrectFilterType) {
    Table table(test_path("ivoa_example.fits"));
    EXPECT_TRUE(table.View().StringFilter(table["Name"], "N 224"));
    EXPECT_TRUE(table.View().NumericFilter(table["RA"], RANGE_INCLUSIVE, 0, 100));
}

TEST(Filtering, CaseSensitiveStringFilter) {
    Table table(test_path("ivoa_example.fits"));

    auto view = table.View();
    view.StringFilter(table["Name"], "N 224");
    EXPECT_EQ(view.NumRows(), 1);
    view.StringFilter(table["Name"], "n 224");
    EXPECT_EQ(view.NumRows(), 0);
    view.StringFilter(table["Name"], "N 598");
    EXPECT_EQ(view.NumRows(), 0);
}

TEST(Filtering, CaseInsensitiveStringFilter) {
    Table table(test_path("ivoa_example.fits"));

    auto view = table.View();
    view.StringFilter(table["Name"], "N 224", true);
    EXPECT_EQ(view.NumRows(), 1);
    view.StringFilter(table["Name"], "n 224", true);
    EXPECT_EQ(view.NumRows(), 1);
    view.StringFilter(table["Name"], "N 598", true);
    EXPECT_EQ(view.NumRows(), 0);
}

TEST(Filtering, FailFilterExtractMistypedValues) {
    Table table(test_path("ivoa_example.fits"));

    auto view = table.View();
    auto double_vals = view.Values<double>(table["RA"]);
    EXPECT_TRUE(double_vals.empty());
    auto string_vals = view.Values<string>(table["RA"]);
    EXPECT_TRUE(string_vals.empty());

    view.StringFilter(table["Name"], "N 6744");
    auto float_vals = view.Values<float>(table["Name"]);
    EXPECT_TRUE(float_vals.empty());
}

TEST(Filtering, FilterExtractValues) {
    Table table(test_path("ivoa_example.fits"));

    auto view = table.View();
    view.NumericFilter(table["RA"], GREATER_OR_EQUAL, 10);
    auto string_vals = view.Values<string>(table["Name"]);
    EXPECT_EQ(string_vals.size(), 3);
    EXPECT_EQ(string_vals[0], "N 224");

    view.StringFilter(table["Name"], "N 6744");
    auto float_vals = view.Values<float>(table["RA"]);
    EXPECT_EQ(float_vals.size(), 1);
    EXPECT_FLOAT_EQ(float_vals[0], 287.43f);
}

TEST(Filtering, NumericFilterEqual) {
    Table table(test_path("ivoa_example.fits"));
    auto view = table.View();
    view.NumericFilter(table["RA"], EQUAL, 287.43);
    EXPECT_EQ(view.NumRows(), 1);
    view.Reset();
    view.NumericFilter(table["e_RVel"], EQUAL, 3);
    EXPECT_EQ(view.NumRows(), 1);
}

TEST(Filtering, NumericFilterNotEqual) {
    Table table(test_path("ivoa_example.fits"));
    auto view = table.View();
    view.NumericFilter(table["RA"], NOT_EQUAL, 287.43);
    EXPECT_EQ(view.NumRows(), 2);
    view.Reset();
    view.NumericFilter(table["e_RVel"], NOT_EQUAL, 3);
    EXPECT_EQ(view.NumRows(), 2);
}

TEST(Filtering, NumericFilterGreater) {
    Table table(test_path("ivoa_example.fits"));

    auto view = table.View();
    view.NumericFilter(table["RA"], GREATER_OR_EQUAL, 10);
    EXPECT_EQ(view.NumRows(), 3);
    view.NumericFilter(table["RA"], GREATER_OR_EQUAL, 11);
    EXPECT_EQ(view.NumRows(), 2);
    view.NumericFilter(table["RA"], GREATER_OR_EQUAL, 300);
    EXPECT_EQ(view.NumRows(), 0);
}

TEST(Filtering, NumericFilterLesser) {
    Table table(test_path("ivoa_example.fits"));

    auto view = table.View();
    view.NumericFilter(table["RA"], LESSER_OR_EQUAL, 300);
    EXPECT_EQ(view.NumRows(), 3);
    view.NumericFilter(table["RA"], LESSER_OR_EQUAL, 11);
    EXPECT_EQ(view.NumRows(), 1);
    view.NumericFilter(table["RA"], LESSER_OR_EQUAL, 10);
    EXPECT_EQ(view.NumRows(), 0);
}

TEST(Filtering, NumericFilterRange) {
    Table table(test_path("ivoa_example.fits"));

    auto view = table.View();
    view.NumericFilter(table["RA"], RANGE_INCLUSIVE, 10, 300);
    EXPECT_EQ(view.NumRows(), 3);
    view.NumericFilter(table["RA"], RANGE_INCLUSIVE, 11, 300);
    EXPECT_EQ(view.NumRows(), 2);
    view.NumericFilter(table["RA"], RANGE_INCLUSIVE, 11, 14);
    EXPECT_EQ(view.NumRows(), 0);
}

TEST(Sorting, FailSortMissingColummn) {
    Table table(test_path("ivoa_example.fits"));

    auto view = table.View();
    EXPECT_FALSE(view.SortByColumn(nullptr));
}

TEST(Sorting, SortNumericAscending) {
    Table table(test_path("ivoa_example.fits"));

    auto view = table.View();
    EXPECT_TRUE(view.SortByColumn(table["RA"]));
    auto vals = view.Values<float>(table["RA"]);
    EXPECT_FLOAT_EQ(vals[0], 10.68f);
    EXPECT_FLOAT_EQ(vals[1], 23.48f);
    EXPECT_FLOAT_EQ(vals[2], 287.43f);
}

TEST(Sorting, SortNumericDescending) {
    Table table(test_path("ivoa_example.fits"));

    auto view = table.View();
    EXPECT_TRUE(view.SortByColumn(table["RA"], false));
    auto vals = view.Values<float>(table["RA"]);
    EXPECT_FLOAT_EQ(vals[0], 287.43f);
    EXPECT_FLOAT_EQ(vals[1], 23.48f);
    EXPECT_FLOAT_EQ(vals[2], 10.68f);
}

TEST(Sorting, SortNumericSubset) {
    Table table(test_path("ivoa_example.fits"));

    // Ascending sort
    auto view = table.View();
    view.NumericFilter(table["RA"], RANGE_INCLUSIVE, 11, 300);
    EXPECT_TRUE(view.SortByColumn(table["RA"]));
    auto vals = view.Values<float>(table["RA"]);
    EXPECT_FLOAT_EQ(vals[0], 23.48f);
    EXPECT_FLOAT_EQ(vals[1], 287.43f);
}

TEST(Sorting, SortStringAscending) {
    Table table(test_path("ivoa_example.fits"));

    auto view = table.View();
    EXPECT_TRUE(view.SortByColumn(table["Name"]));
    auto vals = view.Values<string>(table["Name"]);
    EXPECT_EQ(vals[0], "N 224");
    EXPECT_EQ(vals[1], "N 598");
    EXPECT_EQ(vals[2], "N 6744");
}

TEST(Sorting, SortStringDescending) {
    Table table(test_path("ivoa_example.fits"));

    auto view = table.View();
    EXPECT_TRUE(view.SortByColumn(table["Name"], false));
    auto vals = view.Values<string>(table["Name"]);
    EXPECT_EQ(vals[0], "N 6744");
    EXPECT_EQ(vals[1], "N 598");
    EXPECT_EQ(vals[2], "N 224");
}

TEST(Sorting, SortStringSubset) {
    Table table(test_path("ivoa_example.fits"));

    // Ascending sort
    auto view = table.View();
    view.NumericFilter(table["RA"], RANGE_INCLUSIVE, 11, 300);
    EXPECT_TRUE(view.SortByColumn(table["Name"]));
    auto vals = view.Values<string>(table["Name"]);
    EXPECT_EQ(vals[0], "N 598");
    EXPECT_EQ(vals[1], "N 6744");
}

TEST(Arrays, ParseArrayFile) {
    Table table(test_path("array_types.fits"));
    EXPECT_TRUE(table.IsValid());
    EXPECT_EQ(table.NumRows(), 3);
}

TEST(Arrays, IgnoreArrayTypes) {
    Table table(test_path("array_types.fits"));
    EXPECT_EQ(table["FixedArray"]->data_type, UNKNOWN_TYPE);
    EXPECT_EQ(table["BoundedArray"]->data_type, UNKNOWN_TYPE);
    EXPECT_EQ(table["UnboundedArray"]->data_type, UNKNOWN_TYPE);
    EXPECT_EQ(table["FixedArray2D"]->data_type, UNKNOWN_TYPE);
    EXPECT_EQ(table["BoundedArray2D"]->data_type, UNKNOWN_TYPE);
    EXPECT_EQ(table["UnboundedArray2D"]->data_type, UNKNOWN_TYPE);
}

TEST(Arrays, CorrectScalarData) {
    Table table(test_path("array_types.fits"));
    auto& scalar1_vals = DataColumn<float>::TryCast(table["Scalar1"])->entries;
    auto& scalar2_vals = DataColumn<float>::TryCast(table["Scalar2"])->entries;
    EXPECT_FLOAT_EQ(scalar1_vals[0], 1.0f);
    EXPECT_FLOAT_EQ(scalar1_vals[1], 2.0f);
    EXPECT_FLOAT_EQ(scalar1_vals[2], 3.0f);
    EXPECT_FLOAT_EQ(scalar2_vals[0], 2.0f);
    EXPECT_FLOAT_EQ(scalar2_vals[1], 4.0f);
    EXPECT_FLOAT_EQ(scalar2_vals[2], 6.0f);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

