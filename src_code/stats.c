#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "stats.h"

double calculate_mean(float *vals, int size) {
    if (size == 0) return NAN;
    double sum = 0;
    int valid_count = 0;
    for (int i = 0; i < size; i++) {
        if (!isnan(vals[i])) {
            sum += vals[i];
            valid_count++;
        }
    }
    return valid_count > 0 ? sum / valid_count : NAN;
}

double calculate_sd(float *vals, int size) {
    if (size == 0) return NAN;
    double mean = calculate_mean(vals, size);
    double sum_sq = 0;
    int valid_count = 0;
    for (int i = 0; i < size; i++) {
        if (!isnan(vals[i])) {
            sum_sq += pow(vals[i] - mean, 2);
            valid_count++;
        }
    }
    return valid_count > 0 ? sqrt(sum_sq / valid_count) : NAN;
}

double calculate_min(float *vals, int size) {
    if (size == 0) return NAN;
    double min = NAN;
    int i;
    for (i = 0; i < size; i++) {
        if (!isnan(vals[i])) {
            min = vals[i];
            break;
        }
    }
    if (isnan(min)) return NAN;
    for (; i < size; i++) {
        if (!isnan(vals[i]) && vals[i] < min) min = vals[i];
    }
    return min;
}

double calculate_max(float *vals, int size) {
    if (size == 0) return NAN;
    double max = NAN;
    int i;
    for (i = 0; i < size; i++) {
        if (!isnan(vals[i])) {
            max = vals[i];
            break;
        }
    }
    if (isnan(max)) return NAN;
    for (; i < size; i++) {
        if (!isnan(vals[i]) && vals[i] > max) max = vals[i];
    }
    return max;
}

int cmp_doubles(const void *a, const void *b) {
    double x = *(const double *)a;
    double y = *(const double *)b;
    return (x > y) - (x < y);
}

double calculate_quantile(float *vals, int size, double quantile) {
    if (size == 0) return NAN;
    double *valid_vals = malloc(size * sizeof(double));
    if (!valid_vals) return NAN;
    int valid_count = 0;
    for (int i = 0; i < size; i++) {
        if (!isnan(vals[i])) {
            valid_vals[valid_count++] = vals[i];
        }
    }
    if (valid_count == 0) {
        free(valid_vals);
        return NAN;
    }
    qsort(valid_vals, valid_count, sizeof(double), cmp_doubles);
    int idx = (int)(quantile * (valid_count - 1));
    double result = valid_vals[idx];
    free(valid_vals);
    return result;
}

void print_info(const Table *table) {
    if (!table || !table->headers || !table->types || table->cols <= 0) {
        printf("Error: Invalid table\n");
        return;
    }
    const char *dtype_str[] = {"String", "Int", "Float", "Bool", "Null"};
    printf("\nRange index: 0 to %d\n", table->rows - 1);
    printf("#   Column            Dtype\n");
    for (int j = 0; j < table->cols; j++) {
        printf("%-3d %-18s %s\n", j + 1, table->headers[j] ? table->headers[j] : "", dtype_str[table->types[j]]);
    }
}

void print_stats(const Table *table) {
    if (!table || !table->headers || !table->types || table->cols <= 0) {
        printf("Error: Invalid table\n");
        return;
    }
    printf("\n----------------------- Statistics -----------------------\n");

    printf("%-12s", "");
    for (int j = 0; j < table->cols; j++) {
        if (table->types[j] == TYPE_INT || table->types[j] == TYPE_FLOAT) {
            printf("%-15s", table->headers[j] ? table->headers[j] : "");
        }
    }
    printf("\n");

    const char *labels[] = {"Count", "Mean", "SD", "Min", "25%", "50%", "75%", "Max"};
    for (int stat = 0; stat < 8; stat++) {
        printf("%-12s", labels[stat]);
        for (int j = 0; j < table->cols; j++) {
            if (table->types[j] != TYPE_INT && table->types[j] != TYPE_FLOAT) continue;
            float *vals = get_column_as_float(table, j);
            if (!vals) {
                printf("%-15s", "NaN");
                continue;
            }
            int size = table->rows - 1;
            double result = NAN;
            if (size > 0) {
                switch (stat) {
                    case 0: {
                        int count = 0;
                        for (int i = 0; i < size; i++) {
                            if (!isnan(vals[i])) count++;
                        }
                        result = count;
                        break;
                    }
                    case 1: result = calculate_mean(vals, size); break;
                    case 2: result = calculate_sd(vals, size); break;
                    case 3: result = calculate_min(vals, size); break;
                    case 4: result = calculate_quantile(vals, size, 0.25); break;
                    case 5: result = calculate_quantile(vals, size, 0.50); break;
                    case 6: result = calculate_quantile(vals,  size, 0.75); break;
                    case 7: result = calculate_max(vals, size); break;
                }
            }
            if (isnan(result))
                printf("%-15s", "NaN");
            else if (stat == 0)
                printf("%-15.0f", result);
            else
                printf("%-15.2f", result);

            free(vals);
        }
        printf("\n");
    }
}
