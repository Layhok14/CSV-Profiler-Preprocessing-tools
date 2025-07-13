#ifndef DATA_PREPARATION_H
#define DATA_PREPARATION_H

#include "fileio.h"

// Declare functions with full parameter list to match .c implementations

int bucket_col(Table *t, int col, int nbuckets, int preserve_header);
int outlier_detection(Table *t);
int scale_to_range(Table *t, int col, double a, double b, int preserve_header);

#endif
