#include <chrono>
#include <numeric>
#include <execution>
#include <algorithm>

#include <fmt/format.h>
#include "Table.h"
#include "Filter.h"

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
        fmt::print("Read {} {} in {} ms\n", header_only ? "header of" : "table", filename, dt);

        Column* first_column = table.GetColumn(column_to_sum);
        Column* second_column = table.GetColumn(column_to_sum2);
        if (first_column && second_column) {
            auto float_column = dynamic_cast<NumericColumn<float>*>(first_column);
            auto double_column = dynamic_cast<NumericColumn<double>*>(first_column);

            double sum_first;
            if (float_column) {
                sum_first = accumulate(float_column->entries.begin(), float_column->entries.end(), 0.0);
            } else if (double_column) {
                sum_first = accumulate(double_column->entries.begin(), double_column->entries.end(), 0.0);
            } else {
                fmt::print("Column with name \"{}\" is not a floating-point type!\n", column_to_sum);
                return 1;
            }

            auto float_column2 = dynamic_cast<NumericColumn<float>*>(second_column);
            auto double_column2 = dynamic_cast<NumericColumn<double>*>(second_column);
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
            auto first_matches = NumericFilter(first_column, mean, NAN).Execute();
            auto second_matches = NumericFilter(second_column, mean2, NAN).Execute();

            // Calculate set intersection to get indices of rows that pass EITHER filters
            std::vector<int64_t> match_intersection = LogicalFilter(
                LogicalFilter::AND, vector<Filter*>{{new NumericFilter(first_column, mean, NAN),
                                                     new NumericFilter(second_column, mean2, NAN)}}).Execute();
            auto num_intersections = match_intersection.size();

            std::vector<int64_t> match_union = LogicalFilter(
                LogicalFilter::OR, vector<Filter*>{{new NumericFilter(first_column, mean, NAN),
                                                     new NumericFilter(second_column, mean2, NAN)}}).Execute();
            auto num_unions = match_union.size();

            std::vector<int64_t> match_not = Column::InvertIndices(match_union, table.NumRows());
            auto num_inverted = match_not.size();
            auto t_end_filter = chrono::high_resolution_clock::now();
            double dt_filter = 1.0e-3 * std::chrono::duration_cast<std::chrono::microseconds>(t_end_filter - t_start_filter).count();

            double test_val = NAN;
            auto t_start_sort = chrono::high_resolution_clock::now();
            if (!match_union.empty()) {
                if (float_column) {
                    // For a serial sort, remove the first argument
                    std::sort(std::execution::par_unseq, match_union.begin(), match_union.end(), [float_column](int64_t a, int64_t b) {
                        return float_column->entries[a] < float_column->entries[b];
                    });
                    test_val = float_column->entries[match_union[0]];
                } else {
                    std::sort(std::execution::par_unseq, match_union.begin(), match_union.end(), [double_column](int64_t a, int64_t b) {
                        return double_column->entries[a] < double_column->entries[b];
                    });
                    test_val = double_column->entries[match_union[0]];
                }
            }

            auto t_end_sort = chrono::high_resolution_clock::now();
            double dt_sort = 1.0e-3 * std::chrono::duration_cast<std::chrono::microseconds>(t_end_sort - t_start_sort).count();

            // Try to get a string column with name or ID "MAIN_ID"
            string string_name = "MAIN_ID";
            string test_string = "COSMOS";
            auto string_column = dynamic_cast<StringColumn*>(table.GetColumn(string_name));
            if (string_column) {
                auto t_start_string = chrono::high_resolution_clock::now();
                auto string_matched_indices = ((Column*) (string_column))->GetFilteredIndices(test_string, true);
                auto num_matches = string_matched_indices.size();
                auto t_end_string = chrono::high_resolution_clock::now();
                double dt_string = 1.0e-3 * std::chrono::duration_cast<std::chrono::microseconds>(t_end_string - t_start_string).count();
                fmt::print("{} entries with \"{}\" containing the string \"{}\" found in {:2f} ms\n", num_matches, string_name, test_string, dt_string);
            }

            fmt::print("{} entries with \"{}\" >= {:.3f} && \"{}\" >= {:.3f}\n",
                       num_intersections, first_column->name, mean, second_column->name, mean2);
            fmt::print("{} entries with \"{}\" >= {:.3f} || \"{}\" >= {:.3f}\n",
                       num_unions, first_column->name, mean, second_column->name, mean2);
            fmt::print("{} entries with \"{}\" < {:.3f} && \"{}\" < {:.3f}\n",
                       num_inverted, first_column->name, mean, second_column->name, mean2);
            fmt::print("Filtering done in {:.2f} ms\n", dt_filter);
            fmt::print("Sorting of {} entries by \"{}\" done in {:.2f} ms. Lowest value: {:.3f}\n", match_union.size(), double_column->name, dt_sort, test_val);
        } else {
            fmt::print("Column with name \"{}\" not found!\n", column_to_sum);
        }
    }

    return 0;
}

