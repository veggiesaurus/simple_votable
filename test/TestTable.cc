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

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

