#include <gtest/gtest.h>
#include <fmt/format.h>

#include "Table.h"

using namespace std;
using namespace carta;

const string test_base = "../test/test_xml";

string test_path(const string& filename) {
    return fmt::format("{}/{}", test_base, filename);
}

TEST(BasicParsing, FailOnEmptyFilename) {
    Table table("");
    EXPECT_FALSE(table.IsValid());
}

TEST(BasicParsing, FailOnEmptyFilenameHeaderOnly) {
    Table table_header_only("", true);
    EXPECT_FALSE(table_header_only.IsValid());
}

TEST(BasicParsing, FailOnMissingResource) {
    Table table(test_path("no_resource.xml"));
    EXPECT_FALSE(table.IsValid());
}

TEST(BasicParsing, FailOnMissingTable) {
    Table table(test_path("no_table.xml"));
    EXPECT_FALSE(table.IsValid());
}

TEST(BasicParsing, FailOnMissingData) {
    Table table(test_path("no_data.xml"));
    EXPECT_FALSE(table.IsValid());
}

TEST(BasicParsing, ParseMissingDataHeaderOnly) {
    Table table(test_path("empty_data.xml"), true);
    EXPECT_TRUE(table.IsValid());
    EXPECT_EQ(table.NumRows(), 0);
}

TEST(BasicParsing, ParseMissingData) {
    Table table(test_path("empty_data.xml"));
    EXPECT_TRUE(table.IsValid());
    EXPECT_EQ(table.NumRows(), 0);
}

TEST(BasicParsing, ParseIvoaExampleHeaderOnly) {
    Table table(test_path("ivoa_example.xml"), true);
    EXPECT_TRUE(table.IsValid());
    EXPECT_EQ(table.NumRows(), 0);
}

TEST(BasicParsing, ParseIvoaExample) {
    Table table(test_path("ivoa_example.xml"));
    EXPECT_TRUE(table.IsValid());
    EXPECT_EQ(table.NumRows(), 3);
}

TEST(ParsedTable, CorrectFieldCount) {
    Table table(test_path("ivoa_example.xml"));
    EXPECT_TRUE(table.IsValid());
    EXPECT_EQ(table.NumColumns(), 6);
}

TEST(ParsedTable, CorrectFieldNames) {
    Table table(test_path("ivoa_example.xml"));
    EXPECT_EQ(table[0]->name, "RA");
    EXPECT_EQ(table[1]->name, "Dec");
    EXPECT_EQ(table[2]->name, "Name");
    EXPECT_EQ(table[3]->name, "RVel");
    EXPECT_EQ(table[4]->name, "e_RVel");
    EXPECT_EQ(table[5]->name, "R");
}

TEST(ParsedTable, CorrectFieldUnits) {
    Table table(test_path("ivoa_example.xml"));
    EXPECT_EQ(table[0]->unit, "deg");
    EXPECT_EQ(table[1]->unit, "deg");
    EXPECT_TRUE(table[2]->unit.empty());
    EXPECT_EQ(table[3]->unit, "km/s");
    EXPECT_EQ(table[4]->unit, "km/s");
    EXPECT_EQ(table[5]->unit, "Mpc");
}

TEST(ParsedTable, CorrectFieldTypes) {
    Table table(test_path("ivoa_example.xml"));
    EXPECT_EQ(table[0]->data_type, FLOAT_GENERIC);
    EXPECT_EQ(table[1]->data_type, FLOAT_GENERIC);
    EXPECT_EQ(table[2]->data_type, STRING);
    EXPECT_EQ(table[3]->data_type, INT_GENERIC);
    EXPECT_EQ(table[4]->data_type, INT_GENERIC);
    EXPECT_EQ(table[5]->data_type, FLOAT_GENERIC);
}

TEST(ParsedTable, CorrectFieldSizes) {
    Table table(test_path("ivoa_example.xml"));
    EXPECT_EQ(table[0]->data_type_size, 4);
    EXPECT_EQ(table[1]->data_type_size, 4);
    EXPECT_EQ(table[2]->data_type_size, 1);
    EXPECT_EQ(table[3]->data_type_size, 4);
    EXPECT_EQ(table[4]->data_type_size, 2);
    EXPECT_EQ(table[5]->data_type_size, 4);
}

TEST(ParsedTable, CorrectNameLookups) {
    Table table(test_path("ivoa_example.xml"));
    EXPECT_EQ(table["RA"]->name, "RA");
    EXPECT_EQ(table["Dec"]->name, "Dec");
    EXPECT_EQ(table["Name"]->name, "Name");
    EXPECT_EQ(table["RVel"]->name, "RVel");
    EXPECT_EQ(table["e_RVel"]->name, "e_RVel");
    EXPECT_EQ(table["R"]->name, "R");
    EXPECT_EQ(table["dummy"], nullptr);
    EXPECT_EQ(table[""], nullptr);
}

TEST(ParsedTable, CorrectIdLookups) {
    Table table(test_path("ivoa_example.xml"));
    EXPECT_EQ(table["col1"]->id, "col1");
    EXPECT_EQ(table["col2"]->id, "col2");
    EXPECT_EQ(table["col3"]->id, "col3");
    EXPECT_EQ(table["col4"]->id, "col4");
    EXPECT_EQ(table["col5"]->id, "col5");
    EXPECT_EQ(table["col6"]->id, "col6");
}

TEST(ParsedTable, CorrectColumnTypes) {
    Table table(test_path("ivoa_example.xml"));
    EXPECT_NE(DataColumn<float>::TryCast(table["col1"]), nullptr);
    EXPECT_EQ(DataColumn<double>::TryCast(table["col1"]), nullptr);

    EXPECT_NE(DataColumn<string>::TryCast(table["col3"]), nullptr);
    EXPECT_EQ(DataColumn<int>::TryCast(table["col3"]), nullptr);

    EXPECT_NE(DataColumn<int>::TryCast(table["col4"]), nullptr);
    EXPECT_EQ(DataColumn<string>::TryCast(table["col4"]), nullptr);

    EXPECT_NE(DataColumn<int16_t>::TryCast(table["col5"]), nullptr);
    EXPECT_EQ(DataColumn<int>::TryCast(table["col5"]), nullptr);
}

TEST(ParsedTable, CorrectDataValues) {
    Table table(test_path("ivoa_example.xml"));

    auto& col1_vals = DataColumn<float>::TryCast(table["col1"])->entries;
    EXPECT_EQ(col1_vals.size(), 3);
    EXPECT_FLOAT_EQ(col1_vals[0], 10.68f);
    EXPECT_FLOAT_EQ(col1_vals[1], 287.43f);

    auto& col2_vals = DataColumn<float>::TryCast(table["col2"])->entries;
    EXPECT_EQ(col2_vals.size(), 3);
    EXPECT_FLOAT_EQ(col2_vals[0], 41.27f);
    EXPECT_FLOAT_EQ(col2_vals[1], -63.85f);

    auto& col3_vals = DataColumn<string>::TryCast(table["col3"])->entries;
    EXPECT_EQ(col3_vals.size(), 3);
    EXPECT_EQ(col3_vals[0], "N 224");
    EXPECT_EQ(col3_vals[1], "N 6744");

    auto& col5_vals = DataColumn<int16_t>::TryCast(table["col5"])->entries;
    EXPECT_EQ(col5_vals.size(), 3);
    EXPECT_EQ(col5_vals[0], 5);
    EXPECT_EQ(col5_vals[1], 6);

    EXPECT_NE(DataColumn<float>::TryCast(table["col1"]), nullptr);
    EXPECT_EQ(DataColumn<double>::TryCast(table["col1"]), nullptr);

    EXPECT_NE(DataColumn<string>::TryCast(table["col3"]), nullptr);
    EXPECT_EQ(DataColumn<int>::TryCast(table["col3"]), nullptr);

    EXPECT_NE(DataColumn<int>::TryCast(table["col4"]), nullptr);
    EXPECT_EQ(DataColumn<string>::TryCast(table["col4"]), nullptr);

    EXPECT_NE(DataColumn<int16_t>::TryCast(table["col5"]), nullptr);
    EXPECT_EQ(DataColumn<int>::TryCast(table["col5"]), nullptr);
}

TEST(FilteringAndSorting, FailOnWrongFilterType) {
    Table table(test_path("ivoa_example.xml"));
    EXPECT_FALSE(table.View().StringFilter(table["dummy"], "N 224"));
    EXPECT_FALSE(table.View().StringFilter(table["col1"], "N 224"));

    EXPECT_FALSE(table.View().NumericFilter(table["dummy"], 0, 100));
    EXPECT_FALSE(table.View().NumericFilter(table["col3"], 0, 100));
}

TEST(FilteringAndSorting, PassOnCorrectFilterType) {
    Table table(test_path("ivoa_example.xml"));
    EXPECT_TRUE(table.View().StringFilter(table["col3"], "N 224"));
    EXPECT_TRUE(table.View().NumericFilter(table["col1"], 0, 100));
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

