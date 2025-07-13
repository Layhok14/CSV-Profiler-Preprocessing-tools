#ifndef INVESTIGATE_H
#define INVESTIGATE_H

#include "fileio.h"

void list_columns(const Table *table);
void count_isna(const Table *table, int col_index);
void na_count(const Table *table);
void find_outlier(const Table *table);

#endif
