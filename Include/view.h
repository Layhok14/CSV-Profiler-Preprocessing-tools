
#ifndef VIEW_H
#define VIEW_H

#include "fileio.h"

void print_head(const Table *table, int num);
void print_tail(const Table *table, int num);
void print_row_range(const Table *table, int start, int end);

#endif
