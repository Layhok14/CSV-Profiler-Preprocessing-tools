#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<math.h>
#include "fileio.h"
#include "export.h"

const char *field_type_to_str(FieldType type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_FLOAT: return "float";
        case TYPE_BOOL: return "bool";
        case TYPE_NULL: return "null";
        case TYPE_STRING:
        default: return "string";
    }
}

void export_cleaned_data(const Table *table, const char *filename) {
    if (!save_csv(table, filename)) {
        fprintf(stderr, "Failed to export cleaned data to %s\n", filename);
    } else {
        printf("Cleaned data saved to: %s\n", filename);
    }
}

void export_stats_csv(const Table *table, const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Failed to export stats to %s\n", filename);
        return;
    }

    fprintf(fp, "Column,Non-NULL Count,Type,Min,Max,Mean,StdDev\n");

    for (int j = 0; j < table->cols; j++) {
        int count = 0;
        double sum = 0.0, sum_sq = 0.0;
        double min = INFINITY, max = -INFINITY;

        if (table->types[j] == TYPE_INT || table->types[j] == TYPE_FLOAT) {
            for (int i = 1; i < table->rows; i++) {
                if (table->data[i][j] && strlen(table->data[i][j]) > 0 && strcmp(table->data[i][j], "N/A") != 0) {
                    double val = atof(table->data[i][j]);
                    count++;
                    sum += val;
                    sum_sq += val * val;
                    if (val < min) min = val;
                    if (val > max) max = val;
                }
            }

            double mean = count ? sum / count : 0;
            double stddev = count ? sqrt((sum_sq / count) - (mean * mean)) : 0;

            fprintf(fp, "%s,%d,%s,%.2f,%.2f,%.2f,%.2f\n",
                table->headers[j],
                count,
                field_type_to_str(table->types[j]),
                min, max, mean, stddev
            );

        } else {
            for (int i = 1; i < table->rows; i++) {
                if (table->data[i][j] && strlen(table->data[i][j]) > 0 && strcmp(table->data[i][j], "N/A") != 0)
                    count++;
            }

            fprintf(fp, "%s,%d,%s,,,,\n",
                table->headers[j],
                count,
                field_type_to_str(table->types[j])
            );
        }
    }
    fclose(fp);
    printf("Stats exported to: %s\n", filename);
}