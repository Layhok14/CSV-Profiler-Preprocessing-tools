#ifndef EXPORT_H
#define EXPORT_H

#include "fileio.h"

void export_cleaned_data(const Table *table, const char *filename);
void export_stats_csv(const Table *table, const char *filename);
#endif
