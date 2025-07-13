#include <stdio.h>
#include "investigate.h"
#include "fileio.h"
#include "stats.h" // For get_column_as_float

void print_column(const Table *table) {
    if (!table || !table->headers) {
        printf("Invalid table or headers.\n");
        return;
    }
    printf("\nColumn names in the CSV file:\n");
    for (int j = 0; j < table->cols; j++) {
        printf("%d: %s\n", j, table->headers[j] ? table->headers[j] : "N/A");
    }
}

void count_isna(const Table *table, int col) {
    if (!table || col < 0 || col >= table->cols) {
        printf("Invalid column index or table.\n");
        return;
    }
    int count = 0;
    for (int i = 0; i < table->rows; i++) {
        if (table->data[i] && table->data[i][col] && strcmp(table->data[i][col], "N/A") == 0) {
            count++;
        }
    }
    printf("Column %d (%s) has %d N/A values.\n", col, table->headers[col], count);
}

void na_count(const Table *table) {
    if (!table) {
        printf("Invalid table.\n");
        return;
    }
    int total_na = 0;
    for (int i = 0; i < table->rows; i++) {
        for (int j = 0; j < table->cols; j++) {
            if (table->data[i] && table->data[i][j] && strcmp(table->data[i][j], "N/A") == 0) {
                total_na++;
            }
        }
    }
    printf("Total number of N/A values across the table: %d\n", total_na);
}

void find_outlier(const Table *table) {
    if (!table) {
        printf("Invalid table.\n");
        return;
    }
    printf("\nOutlier Detection (using IQR method) for numerical columns\n");
    printf("Column Id  Column Name  Row Index  Outlier Value\n");

    for (int j = 0; j < table->cols; j++) {
        if (table->types[j] == TYPE_INT || table->types[j] == TYPE_FLOAT) {
            float *vals = get_column_as_float(table, j);
            int size = table->rows - 1;
            if (size > 0) {
                double q1 = calculate_quantile(vals, size, 0.25);
                double q3 = calculate_quantile(vals, size, 0.75);
                double iqr = q3 - q1;
                double lower_bound = q1 - 1.5 * iqr;
                double upper_bound = q3 + 1.5 * iqr;

                bool has_outliers = false;
                for (int i = 0; i < size; i++) {
                    if (!isnan(vals[i]) && (vals[i] < lower_bound || vals[i] > upper_bound) && vals[i] > 0) {
                        if (!has_outliers) {
                            printf("%-10d %-11s %-9d %-12.2f\n", j, table->headers[j], i + 1, vals[i]);
                            has_outliers = true;
                        } else {
                            printf("%-10s %-11s %-9d %-12.2f\n", "", "", i + 1, vals[i]);
                        }
                    }
                }
                // No "None" row if no outliers > 0 are found
            }
            free(vals);
        }
    }
}
void list_columns(const Table *table) {
    for (int i = 0; i < table->cols; i++) {
        printf("[%d] %s\n", i, table->headers[i]);
    }
}