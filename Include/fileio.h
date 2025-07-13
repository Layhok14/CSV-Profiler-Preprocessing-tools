#ifndef FILEIO_H
#define FILEIO_H

#include <stdbool.h>
#include <stdio.h>

#define MAX_LINE_LEN 1024
#define MAX_COLS 100

typedef enum
{
    TYPE_STRING,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BOOL,
    TYPE_NULL
} FieldType;

typedef struct
{
    char ***data;     // data[row][col] as strings
    FieldType *types; // inferred type per column
    int rows;
    int cols;
    int capacity;   // dynamic row allocation
    char **headers; // column headers
} Table;

bool load_csv(Table *table, const char *filename);
bool save_csv(const Table *table, const char *filepath);
int get_column_index(const Table *table, const char *column_name);
void free_table(Table *table);
FieldType infer_type(const char *str);
int *get_column_as_int(const Table *table, int col_index);
float *get_column_as_float(const Table *table, int col_index);
bool *get_column_as_bool(const Table *table, int col_index);

#endif // FILEIO_H