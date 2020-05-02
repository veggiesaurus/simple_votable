#include <chrono>
#include <numeric>
#include <fmt/format.h>
#include "Table.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fmt::print("Incorrect arguments supplied\n");
        return 1;
    }

    string filename = argv[1];
    string column_to_sum = argv[2];
    bool header_only = (argc == 4);

    auto t_start = chrono::high_resolution_clock::now();
    carta::Table table(filename, header_only);
    auto t_end = chrono::high_resolution_clock::now();
    double dt = 1.0e-3 * std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start).count();
    if (table.IsValid()) {
        table.PrintInfo(false);
        fmt::print("Read {} {} in {} ms\n", header_only ? "header of" : "table", filename, dt);

        carta::Column* ra_column = table.GetColumn(column_to_sum);
        if (ra_column) {
            auto float_column = dynamic_cast<carta::NumericColumn<float>*>(ra_column);
            if (float_column) {
                double sum = accumulate(float_column->entries.begin(), float_column->entries.end(), 0.0);
                fmt::print("Mean of column \"{}\": {} {}\n", column_to_sum, sum / table.NumRows(), ra_column->unit);
            } else {
                auto double_column = dynamic_cast<carta::NumericColumn<double>*>(ra_column);
                if (double_column) {
                    double sum = accumulate(double_column->entries.begin(), double_column->entries.end(), 0.0);
                    fmt::print("Mean of column \"{}\": {} {}\n", column_to_sum, sum / table.NumRows(), ra_column->unit);
                } else {
                    fmt::print("Column with name \"{}\" is not a floating-point type!\n", column_to_sum);
                }
            }
        } else {
            fmt::print("Column with name \"{}\" not found!\n", column_to_sum);
        }
    }

    return 0;
}

