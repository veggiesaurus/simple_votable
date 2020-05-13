#include <chrono>
#include <numeric>
#include <algorithm>

#include <fmt/format.h>
#include "Table.h"

using namespace std;
using namespace carta;
int main(int argc, char* argv[]) {
    if (argc < 4) {
        fmt::print("Incorrect arguments supplied\n");
        return 1;
    }

    string filename = argv[1];
    string column_to_sum = argv[2];
    string column_to_sum2 = argv[3];
    bool header_only = (argc == 5);

    auto t_start = chrono::high_resolution_clock::now();
    Table table(filename, header_only);
    auto t_end = chrono::high_resolution_clock::now();
    double dt = 1.0e-3 * std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count();
    if (table.IsValid()) {
        table.PrintInfo(false);
        fmt::print("Read {} {} in {} ms. Table dimensions: {} x {}\n", header_only ? "header of" : "table", filename, dt, table.NumRows(), table.NumColumns());

        auto first_column = table[column_to_sum];
        auto second_column = table[column_to_sum2];
        if (first_column && second_column) {
            auto float_column = DataColumn<float>::TryCast(first_column);
            auto double_column = DataColumn<double>::TryCast(first_column);
            double sum_first;
            if (float_column) {
                sum_first = accumulate(float_column->entries.begin(), float_column->entries.end(), 0.0);
            } else if (double_column) {
                sum_first = accumulate(double_column->entries.begin(), double_column->entries.end(), 0.0);
            } else {
                fmt::print("Column with name \"{}\" is not a floating-point type!\n", column_to_sum);
                return 1;
            }

            auto float_column2 = DataColumn<float>::TryCast(second_column);
            auto double_column2 = DataColumn<double>::TryCast(second_column);
            double sum_second = 0;

            if (float_column2) {
                sum_second = accumulate(float_column2->entries.begin(), float_column2->entries.end(), 0.0);
            } else if (double_column2) {
                sum_second = accumulate(double_column2->entries.begin(), double_column2->entries.end(), 0.0);
            } else {
                fmt::print("Column with name \"{}\" is not a floating-point type!\n", column_to_sum2);
                return 1;
            }

            double mean = sum_first / table.NumRows();
            double mean2 = sum_second / table.NumRows();
            fmt::print("Mean of column \"{}\": {:.3f} {}\n", column_to_sum, mean, first_column->unit);
            fmt::print("Mean of column \"{}\": {:.3f} {}\n", column_to_sum2, mean2, second_column->unit);

            auto t_start_filter = chrono::high_resolution_clock::now();
            auto filtered_table = table.View();
            filtered_table.NumericFilter(first_column, mean, NAN);
            filtered_table.NumericFilter(second_column, mean2, NAN);
            auto num_matches = filtered_table.NumRows();
            auto t_end_filter = chrono::high_resolution_clock::now();
            double dt_filter = 1.0e-3 * std::chrono::duration_cast<std::chrono::microseconds>(t_end_filter - t_start_filter).count();

            double test_val = NAN;
            auto t_start_sort = chrono::high_resolution_clock::now();
            if (num_matches) {
                filtered_table.SortByColumn(first_column, false);
                if (double_column) {
                    test_val = filtered_table.Values<double>(double_column, 0, 1)[0];
                } else {
                    test_val = filtered_table.Values<float>(float_column, 0, 1)[0];
                }
            }

            auto t_end_sort = chrono::high_resolution_clock::now();
            double dt_sort = 1.0e-3 * std::chrono::duration_cast<std::chrono::microseconds>(t_end_sort - t_start_sort).count();

            // Try to get a string column with name or ID "MAIN_ID"
            string string_name = "MAIN_ID";
            string test_string = "COSMOS";
            auto string_column = table[string_name];
            if (string_column) {
                string first_string;
                auto t_start_string = chrono::high_resolution_clock::now();
                auto string_matches = table.View();
                string_matches.StringFilter(string_column, test_string, false);
                string_matches.SortByColumn(string_column, true);
                auto num_string_matches = string_matches.NumRows();
                if (num_string_matches) {
                    first_string = string_matches.Values<string>(string_column, 0, 1)[0];
                }
                auto t_end_string = chrono::high_resolution_clock::now();
                double dt_string = 1.0e-3 * std::chrono::duration_cast<std::chrono::microseconds>(t_end_string - t_start_string).count();
                fmt::print("{} entries with \"{}\" containing the string \"{}\" found in {:2f} ms. First string: \"{}\"\n",
                           num_string_matches,
                           string_name,
                           test_string,
                           dt_string,
                           first_string);
            }

            fmt::print("{} entries with \"{}\" >= {:.3f} && \"{}\" >= {:.3f}\n",
                       num_matches, first_column->name, mean, second_column->name, mean2);
            fmt::print("Filtering done in {:.2f} ms\n", dt_filter);
            fmt::print("Sorting of {} entries by \"{}\" done in {:.2f} ms. Highest value: {:.3f}\n", num_matches, first_column->name, dt_sort, test_val);
        } else {
            fmt::print("Column with name \"{}\" not found!\n", column_to_sum);
        }
    }

    return 0;
}

