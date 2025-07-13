#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "preprocess.h"

// Display the number of "N/A" values in each column
void isna(const Table *table) {
    if (!table) {
        printf("Error: Table pointer is NULL.\n");
        return;
    }
    if (table->cols <= 0 || table->rows <= 0) {
        printf("Error: Invalid table dimensions (cols=%d, rows=%d).\n", table->cols, table->rows);
        return;
    }
    if (!table->data || !table->headers) {
        printf("Error: Table data or headers not initialized.\n");
        return;
    }

    for (int j = 0; j < table->cols; j++) {
        int count = 0;
        int valid_rows = 0;
        for (int i = 0; i < table->rows; i++) {
            if (table->data[i] && table->data[i][j] != NULL) {
                valid_rows++;
                if (strcmp(table->data[i][j], "N/A") == 0) {
                    count++;
                }
            } else {
                printf("Null at row %d, col %d\n", i, j);
            }
        }
        if (j < table->cols) { // Ensure j is within headers bounds
            printf("Column %d (%s): %d N/A\n", j, table->headers[j], count);
        } else {
            printf("Error: Column index %d exceeds header count.\n", j);
            break;
        }
    }
}

// Display row indices containing "N/A" in a specified column
void find_na(const Table *table, int col) {
    if (!table) {
        printf("Error: Table pointer is NULL.\n");
        return;
    }
    if (table->cols <= 0 || table->rows <= 0) {
        printf("Error: Invalid table structure.\n");
        return;
    }
    if (!table->data || !table->headers) {
        printf("Error: Table data or headers not initialized.\n");
        return;
    }
    if (col < 0 || col >= table->cols) {
        printf("Invalid column index.\n");
        return;
    }
    bool found = false;
    for (int i = 0; i < table->rows; i++) {
        if (table->data[i] && table->data[i][col] && strcmp(table->data[i][col], "N/A") == 0) {
            printf("Row %d contains N/A in column %d\n", i, col);
            found = true;
        }
    }
    if (!found) {
        printf("No N/A values found in column %d\n", col);
    }
}

// Fill "N/A" with 0, mean, or median of a numerical column
void fill_na(Table *table, int col, const char *val) {
    if (!table || table->cols <= 0 || table->rows <= 0) {
        printf("Error: Invalid table structure.\n");
        return;
    }
    if (!table->data || !table->headers) {
        printf("Error: Table data or headers not initialized.\n");
        return;
    }
    if (col < 0 || col >= table->cols) {
        printf("Invalid column index.\n");
        return;
    }
    if (strcmp(val, "0") == 0) {
        for (int i = 0; i < table->rows; i++) {
            if (table->data[i] && table->data[i][col] && strcmp(table->data[i][col], "N/A") == 0) {
                free(table->data[i][col]);
                table->data[i][col] = strdup("0");
            }
        }
        printf("N/A values in column %d filled with 0.\n", col);
        return;
    }

    int *int_vals = NULL;
    float *float_vals = NULL;
    int valid_count = 0;
    for (int i = 0; i < table->rows; i++) {
        if (table->data[i] && table->data[i][col] && strcmp(table->data[i][col], "N/A") != 0) {
            valid_count++;
        }
    }
    if (valid_count == 0) {
        printf("No valid numerical values to calculate mean/median.\n");
        return;
    }

    int_vals = malloc(valid_count * sizeof(int));
    float_vals = malloc(valid_count * sizeof(float));
    int int_idx = 0, float_idx = 0;
    for (int i = 0; i < table->rows; i++) {
        if (table->data[i] && table->data[i][col] && strcmp(table->data[i][col], "N/A") != 0) {
            char *endptr;
            strtol(table->data[i][col], &endptr, 10);
            if (*endptr == '\0') {
                int_vals[int_idx++] = atoi(table->data[i][col]);
            } else {
                float_vals[float_idx++] = atof(table->data[i][col]);
            }
        }
    }

    if (strcmp(val, "mean") == 0) {
        float sum = 0;
        int count = 0;
        for (int i = 0; i < int_idx; i++) sum += int_vals[i], count++;
        for (int i = 0; i < float_idx; i++) sum += float_vals[i], count++;
        float mean = sum / count;
        char buffer[32];
        sprintf(buffer, "%.2f", mean);
        for (int i = 0; i < table->rows; i++) {
            if (table->data[i] && table->data[i][col] && strcmp(table->data[i][col], "N/A") == 0) {
                free(table->data[i][col]);
                table->data[i][col] = strdup(buffer);
            }
        }
        printf("N/A values in column %d filled with mean %.2f.\n", col, mean);
    } else if (strcmp(val, "median") == 0) {
        if (int_idx > 0) {
            for (int i = 0; i < int_idx - 1; i++) {
                for (int j = i + 1; j < int_idx; j++) {
                    if (int_vals[i] > int_vals[j]) {
                        int temp = int_vals[i];
                        int_vals[i] = int_vals[j];
                        int_vals[j] = temp;
                    }
                }
            }
            int median = int_vals[int_idx / 2];
            char buffer[32];
            sprintf(buffer, "%d", median);
            for (int i = 0; i < table->rows; i++) {
                if (table->data[i] && table->data[i][col] && strcmp(table->data[i][col], "N/A") == 0) {
                    free(table->data[i][col]);
                    table->data[i][col] = strdup(buffer);
                }
            }
            printf("N/A values in column %d filled with median %d.\n", col, median);
        } else if (float_idx > 0) {
            for (int i = 0; i < float_idx - 1; i++) {
                for (int j = i + 1; j < float_idx; j++) {
                    if (float_vals[i] > float_vals[j]) {
                        float temp = float_vals[i];
                        float_vals[i] = float_vals[j];
                        float_vals[j] = temp;
                    }
                }
            }
            float median = float_vals[float_idx / 2];
            char buffer[32];
            sprintf(buffer, "%.2f", median);
            for (int i = 0; i < table->rows; i++) {
                if (table->data[i] && table->data[i][col] && strcmp(table->data[i][col], "N/A") == 0) {
                    free(table->data[i][col]);
                    table->data[i][col] = strdup(buffer);
                }
            }
            printf("N/A values in column %d filled with median %.2f.\n", col, median);
        }
    } else {
        printf("Invalid fill option. Use '0', 'mean', or 'median'.\n");
    }

    free(int_vals);
    free(float_vals);
}

// Drop all rows with "N/A" in the specified column
void drop_na(Table *table, int col) {
    if (!table || table->cols <= 0 || table->rows <= 0) {
        printf("Error: Invalid table structure.\n");
        return;
    }
    if (!table->data || !table->headers) {
        printf("Error: Table data or headers not initialized.\n");
        return;
    }
    if (col < 0 || col >= table->cols) {
        printf("Invalid column index.\n");
        return;
    }
    int i = 0;
    while (i < table->rows) {
        if (table->data[i] && table->data[i][col] && strcmp(table->data[i][col], "N/A") == 0) {
            for (int j = 0; j < table->cols; j++) {
                if (table->data[i][j]) free(table->data[i][j]);
            }
            free(table->data[i]);
            for (int k = i; k < table->rows - 1; k++) {
                table->data[k] = table->data[k + 1];
            }
            table->rows--;
            table->data = realloc(table->data, table->rows * sizeof(char **));
            if (!table->data && table->rows > 0) {
                printf("Error: Memory reallocation failed.\n");
                return;
            }
        } else {
            i++;
        }
    }
}

// Convert a numerical column's string values to appropriate numeric type
void convert_to_num(Table *table, int col) {
    if (!table || table->cols <= 0 || table->rows <= 0) {
        printf("Error: Invalid table structure.\n");
        return;
    }
    if (!table->data || !table->headers) {
        printf("Error: Table data or headers not initialized.\n");
        return;
    }
    if (col < 0 || col >= table->cols) {
        printf("Invalid column index.\n");
        return;
    }
    bool is_numeric = false;
    for (int i = 0; i < table->rows; i++) {
        if (table->data[i] && table->data[i][col]) {
            char *endptr;
            strtol(table->data[i][col], &endptr, 10);
            if (*endptr == '\0') is_numeric = true;
            else {
                strtod(table->data[i][col], &endptr);
                if (*endptr == '\0') is_numeric = true;
                else break;
            }
        }
    }
    if (!is_numeric) {
        printf("Error: Column %d is not numerical.\n", col);
        return;
    }
    for (int i = 0; i < table->rows; i++) {
        if (table->data[i] && table->data[i][col]) {
            char *endptr;
            long int_val = strtol(table->data[i][col], &endptr, 10);
            if (*endptr == '\0') {
                char buffer[32];
                sprintf(buffer, "%ld", int_val);
                free(table->data[i][col]);
                table->data[i][col] = strdup(buffer);
            } else {
                float float_val = strtod(table->data[i][col], &endptr);
                char buffer[32];
                sprintf(buffer, "%.2f", float_val);
                free(table->data[i][col]);
                table->data[i][col] = strdup(buffer);
            }
        }
    }
    printf("Column %d converted to numerical values.\n", col);
}