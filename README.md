This is an example implementation of a VOTable parser. It uses pugixml (Debian package `libpugixml-dev`) to parse the source VOTable file, and then builts an in-memory table using strong typing of columns. Only string, integer (`short`, `int` and `long`) and floating point (`float` and `double`) data types are currently supported. 

To run, pass in arguments as follows:
```
votable_test <name of VOTable file> <first column name> <second column name> <headeronly?>
```
where `first column name` and `second column name` are the names of columns to filter on.


After building the table, it calculates the average value of a specified column. Note that this column must have a `datatype` of `double` or `float`. If any argument is passed in the `headeronly` field, only the first 64 kB of the table are read, and this is used to construct the header itself, rather than reading the entire table.

OpenMP can be used to parallelize the the in-memory table creating by enabling the `#define` `PARALLEL_FILL` in `Table.cc`