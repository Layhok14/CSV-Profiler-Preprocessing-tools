#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libgen.h>
#include "fileio.h"
#define INITIAL_ROW_CAPACITY 1000
#define MAX_LINE_LEN 1024
#define DATA_DIR "Data/"
#define ALT_DATA_DIR "../Data/"//If there is problem with the file path
//for each cell type of data
FieldType type(const char *str) {
    if (str == NULL || strlen(str) == 0) return TYPE_NULL;

    if (strcasecmp(str, "true") ==0 || strcasecmp(str, "false")==0){ 
        //More effective than the strcmp
        return TYPE_BOOL;
    }
    char *endptr; 
    strtol(str, &endptr, 10); //Convert from the string to the integer
    if (*endptr == '\0') return TYPE_INT;
    //if using the strtol doesn't work, we have to try to convert it to float
    strtod(str, &endptr);
    if (*endptr == '\0') return TYPE_FLOAT;
    //If it doesn't return the float type, then it is a string
    return TYPE_STRING;
}
void get_directory(const char *filepath, char *out_dir, size_t max_len) {
    const char *last_slash = strrchr(filepath, '/');
    const char *last_backslash = strrchr(filepath, '\\');

    if (!last_slash && !last_backslash) {
        strncpy(out_dir, ".", max_len);
        out_dir[max_len - 1] = '\0';
        return;
    }
    const char *pos = last_slash > last_backslash ? last_slash : last_backslash;
    size_t len = pos - filepath;

    if (len >= max_len) len = max_len - 1;
    strncpy(out_dir, filepath, len);
    out_dir[len] = '\0';
}
char *parse_field(char **line, char delim) {
    char *start = *line;
    char *field = NULL;
    int len = 0;
    bool in_quotes = false;

    while (**line && (**line != delim || in_quotes)) {
        if (**line == '"') {
            in_quotes = !in_quotes;
            (*line)++;
            continue;
        }
        len++;
        (*line)++;
    }

    field = malloc(len + 1);
    if (!field) return NULL;

    strncpy(field, start, len);
    field[len] = '\0';

    if (**line == delim) (*line)++;

    // Remove quotes if if it i si the the cell.
    if (field[0] == '"' && field[len - 1] == '"') {
        field[len - 1] = '\0';
        memmove(field, field + 1, len - 1);
    }

    return field;
}

bool load_csv(Table *table, const char *filename) {
    FILE *fp = NULL;
    char *full_path = NULL;

    // Try the provided filename first
    fp = fopen(filename, "r");
    if (!fp) {
        full_path = malloc(strlen(DATA_DIR) + strlen(filename) + 1);
        if (!full_path) return false;
        sprintf(full_path, "%s%s", DATA_DIR, filename);
        fp = fopen(full_path, "r");
    }
    if (!fp) {
        free(full_path);
        full_path = malloc(strlen(ALT_DATA_DIR) + strlen(filename) + 1);
        if (!full_path) return false;
        sprintf(full_path, "%s%s", ALT_DATA_DIR, filename);
        fp = fopen(full_path, "r");
    }
    free(full_path);
    if (!fp) return false;

    table->rows = 0;
    table->cols = 0;
    table->capacity = INITIAL_ROW_CAPACITY;
    table->data = malloc(table->capacity * sizeof(char **));
    table->types = calloc(MAX_COLS, sizeof(FieldType));
    table->headers = NULL;

    char line[MAX_LINE_LEN];
    bool first_row = true;

    while (fgets(line, MAX_LINE_LEN, fp)) {
        line[strcspn(line, "\n\r")] = '\0';

        if (table->rows >= table->capacity) {
            table->capacity *= 2;
            char ***new_data = realloc(table->data, table->capacity * sizeof(char **));
            if (!new_data) {
                fclose(fp);
                free_table(table);
                return false;
            }
            table->data = new_data;
        }

        int col = 0;
        char *ptr = line;
        int fields_in_row = 0;

        // Count fields in this row to determine allocation size
        char *temp_ptr = ptr;
        while (*temp_ptr) {
            if (*temp_ptr == ',') fields_in_row++;
            temp_ptr++;
        }
        fields_in_row = (fields_in_row > 0) ? fields_in_row + 1 : 1; // Account for the last field

        if (first_row && fields_in_row > table->cols) table->cols = fields_in_row;

        table->data[table->rows] = malloc(table->cols * sizeof(char *));
        if (!table->data[table->rows]) {
            fclose(fp);
            free_table(table);
            return false;
        }

        while (*ptr && col < table->cols) {
            char *field = parse_field(&ptr, ',');
            if (!field) {
                fclose(fp);
                free_table(table);
                return false;
            }

            if (!field || strlen(field) == 0 || (strlen(field) == 1 && isspace(field[0]))) {
                free(field);
                field = strdup("N/A");
                if (!field) {
                    fclose(fp);
                    free_table(table);
                    return false;
                }
            }

            table->data[table->rows][col] = field;

            if (first_row) {
                if (!table->headers) table->headers = malloc(table->cols * sizeof(char *));
                table->headers[col] = strdup(field);
            } else if (table->types && table->rows == 1) {
                FieldType inferred = type(field);
                if (inferred > table->types[col]) table->types[col] = inferred;
            }

            col++;
        }

        // Pad with N/A if row has fewer fields than table->cols
        while (col < table->cols) {
            table->data[table->rows][col] = strdup("N/A");
            if (!table->data[table->rows][col]) {
                fclose(fp);
                free_table(table);
                return false;
            }
            col++;
        }

        table->rows++;
        first_row = false;
    }

    fclose(fp);
    return true;
}

bool save_csv(const Table *table, const char *filepath) {
    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        char *full_path = malloc(strlen(DATA_DIR) + strlen(filepath) + 1);
        if (!full_path) return false;
        sprintf(full_path, "%s%s", DATA_DIR, filepath);
        fp = fopen(full_path, "w");
        free(full_path);
    }

    if (!fp) {
        char *alt_path = malloc(strlen(ALT_DATA_DIR) + strlen(filepath) + 1);
        if (!alt_path) return false;
        sprintf(alt_path, "%s%s", ALT_DATA_DIR, filepath);
        fp = fopen(alt_path, "w");
        free(alt_path);
    }

    if (!fp) return false;

    // Write headers
    for (int j = 0; j < table->cols; j++) {
        fprintf(fp, "%s", table->headers[j] ? table->headers[j] : "N/A");
        if (j < table->cols - 1) fprintf(fp, ",");
    }
    fprintf(fp, "\n");

    // Write data
    for (int i = 0; i < table->rows; i++) {
        for (int j = 0; j < table->cols; j++) {
            fprintf(fp, "%s", table->data[i][j] ? table->data[i][j] : "N/A");
            if (j < table->cols - 1) fprintf(fp, ",");
        }
        fprintf(fp, "\n");
    }

    fclose(fp);
    return true;
}

int get_column_index(const Table *table, const char *column_name) {
    for (int j = 0; j < table->cols; j++) {
        if (table->headers && strcmp(table->headers[j], column_name) == 0) return j;
    }
    return -1;
}

int *get_column_as_int(const Table *table, int col_index) {
    int *array = malloc((table->rows - 1) * sizeof(int));
    for (int i = 1; i < table->rows; i++) {
        array[i - 1] = table->data[i][col_index] ? atoi(table->data[i][col_index]) : 0;
    }
    return array;
}

float *get_column_as_float(const Table *table, int col_index) {
    float *array = malloc((table->rows - 1) * sizeof(float));
    for (int i = 1; i < table->rows; i++) {
        array[i - 1] = table->data[i][col_index] ? atof(table->data[i][col_index]) : 0.0f;
    }
    return array;
}

bool *get_column_as_bool(const Table *table, int col_index) {
    bool *array = malloc((table->rows - 1) * sizeof(bool));
    for (int i = 1; i < table->rows; i++) {
        array[i - 1] = table->data[i][col_index] && (strcasecmp(table->data[i][col_index], "true") == 0);
    }
    return array;
}

void free_table(Table *table) {
    for (int i = 0; i < table->rows; i++) {
        for (int j = 0; j < table->cols; j++) {
            free(table->data[i][j]);
        }
        free(table->data[i]);
    }
    free(table->data);
    free(table->types);
    if (table->headers) {
        for (int j = 0; j < table->cols; j++) {
            free(table->headers[j]);
        }
        free(table->headers);
    }
    table->rows = 0;
    table->cols = 0;
    table->capacity = 0;
}