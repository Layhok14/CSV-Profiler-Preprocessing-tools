#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "view.h"

#define MAX_COLUMN_WIDTH 30
#define MIN_SPACING 6

// Dynamically calculate width for each column (+1 for "No.")
static int *calculate_column_widths(const Table *table) {
    int *widths = calloc(table->cols + 1, sizeof(int)); // +1 for "No." column
    if (!widths) return NULL;

    widths[0] = 4; // For "No."

    for (int j = 0; j < table->cols; j++) {
        int max_len = strlen(table->headers[j]);
        for (int i = 1; i < table->rows; i++) { // skip header row
            int len = table->data[i][j] ? strlen(table->data[i][j]) : 0;
            if (len > max_len) max_len = len;
        }
        widths[j + 1] = (max_len > MAX_COLUMN_WIDTH) ? MAX_COLUMN_WIDTH : max_len;
    }

    return widths;
}

// Print merged title line with dashes
static void print_merged_title_line(const char *title, int *widths, int cols) {
    int total = 0;
    for (int i = 0; i < cols; i++)
        total += widths[i] + MIN_SPACING;

    int title_len = strlen(title);
    int left = (total - title_len) / 2;
    int right = total - title_len - left;

    for (int i = 0; i < left; i++) printf("-");
    printf("%s", title);
    for (int i = 0; i < right; i++) printf("-");
    printf("\n");
}

// Print wrapped chunk of string for line
static int print_wrapped_field(const char *str, int width, int line) {
    int len = strlen(str);
    int start = line * width;
    if (start >= len) {
        printf("%-*s", width + MIN_SPACING, "");
        return 0;
    }
    int remain = len - start;
    int to_print = (remain > width) ? width : remain;
    printf("%-*.*s", width + MIN_SPACING, to_print, str + start);
    return (start + to_print < len);
}

// Print one row, wrapped if needed
static void print_wrapped_row(const char **row, int *widths, int cols, int row_num) {
    int more;
    int line = 0;
    do {
        more = 0;

        if (line == 0)
            printf("%-*d", widths[0], row_num);
        else
            printf("%-*s", widths[0], "");

        for (int j = 0; j < cols; j++) {
            int field_more = print_wrapped_field(row[j], widths[j + 1], line);
            if (field_more) more = 1;
        }
        printf("\n");
        line++;
    } while (more);
}

// Print a table section given start/end rows and title
static void print_table_section(const Table *table, int start, int end, const char *title) {
    int *widths = calculate_column_widths(table);
    if (!widths) return;

    print_merged_title_line(title, widths + 1, table->cols);

    printf("%-*s", widths[0], "No.");
    for (int j = 0; j < table->cols; j++) {
        printf("%-*s", widths[j + 1] + MIN_SPACING, table->headers[j]);
    }
    printf("\n");

    for (int j = 0; j <= table->cols; j++) {
        for (int k = 0; k < widths[j]; k++) printf("-");
        if (j < table->cols) printf("  | ");
    }
    printf("\n");

    int row_number = start;
    for (int i = start; i < end; i++) {
        print_wrapped_row((const char **)table->data[i], widths, table->cols, row_number++);
    }

    free(widths);
}

// Show top N rows
void print_head(const Table *table, int num) {
    int data_rows = table->rows - 1;
    int count = (num < data_rows) ? num : data_rows;
    print_table_section(table, 1, count + 1, "Header");
}

// Show last N rows
void print_tail(const Table *table, int num) {
    int data_rows = table->rows - 1;
    int count = (num < data_rows) ? num : data_rows;
    int start = data_rows - count + 1;
    if (start < 1) start = 1;
    print_table_section(table, start, table->rows, "Tailer");
}

// Show specific row range (start to end inclusive)
void print_row_range(const Table *table, int start, int end) {
    int data_rows = table->rows - 1;
    if (start < 1) start = 1;
    if (end > data_rows) end = data_rows;
    if (start > end || start > data_rows) {
        printf("Invalid row range.\n");
        return;
    }
    print_table_section(table, start, end + 1, "Row Range");
}
