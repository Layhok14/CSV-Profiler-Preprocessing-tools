#ifndef STATS_H
#define STATS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fileio.h"

double calculate_mean(float *vals, int size);
double calculate_sd(float *vals, int size);
double calculate_min(float *vals, int size);
double calculate_max(float *vals, int size);
double calculate_quantile(float *vals, int size, double quantile);
void print_info(const Table *table);
void print_stats(const Table *table);
void export_stats_csv(const Table *table, const char *filename);


#endif