#ifndef PREPROCESS_H
#define PREPROCESS_H

#include "fileio.h"
#include <stdbool.h>


void isna(const Table *table);
void find_na(const Table *table, int col);
void fill_na(Table *table, int col, const char *val);
void drop_na(Table *table, int col);
void convert_to_num(Table *table, int col);

#endif