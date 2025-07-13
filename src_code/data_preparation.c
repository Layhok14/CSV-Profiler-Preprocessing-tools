
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include "data_preparation.h"

// Helper functions
int parse_num(const char *s, double *out) {
    if (!s || !*s) return 0;
    char *end;
    double v = strtod(s, &end);
    if (*end) return 0;
    if (out) *out = v;
    return 1;
}

static int cmpdbl(const void *a, const void *b) {
    double A = *(double *)a, B = *(double *)b;
    return (A > B) - (A < B);
}

static double median(const double *a, int n) {
    return (n & 1) ? a[n / 2] : (a[n / 2 - 1] + a[n / 2]) / 2.0;
}

/* =========================================================
 * bucket_col()
 * Buckets numeric values in a column into specified ranges.
 * Arguments: Table *t, int col, int nbuckets, int preserve_header
 * Returns: 1 on success, 0 on failure
 * ======================================================= */
int bucket_col(Table *t, int col, int nbuckets, int preserve_header) {
    printf("[bucket_col] Starting bucketing for column %d with %d buckets...\n", col, nbuckets);
    
    if (!t || col < 0 || col >= t->cols || nbuckets < 1) {
        printf("[bucket_col] Error: Invalid input (table=%p, col=%d, nbuckets=%d)\n", 
               (void*)t, col, nbuckets);
        return 0;
    }

    double vmin = DBL_MAX, vmax = -DBL_MAX, v;
    int valid_count = 0;
    for (int r = 0; r < t->rows; ++r) {
        if (parse_num(t->data[r][col], &v)) {
            if (v < vmin) vmin = v;
            if (v > vmax) vmax = v;
            valid_count++;
        }
    }

    if (valid_count == 0) {
        printf("[bucket_col] Error: No valid numeric data in column %d\n", col);
        return 0;
    }
    if (fabs(vmax - vmin) < 1e-12) {
        printf("[bucket_col] Warning: All values in column %d are identical (%.6g). Adjusting range.\n", col, vmin);
        vmax = vmin + 1.0;
    }

    double width = (vmax - vmin) / nbuckets;
    char label[64];
    for (int r = 0; r < t->rows; ++r) {
        if (parse_num(t->data[r][col], &v)) {
            int idx = (int)floor((v - vmin) / width);
            if (idx == nbuckets) idx--;
            double lo = vmin + idx * width;
            double hi = lo + width;
            snprintf(label, sizeof(label), "[%.2f-%.2f)", lo, hi);
            char *new_str = strdup(label);
            if (!new_str) {
                printf("[bucket_col] Error: Memory allocation failed for row %d\n", r);
                return 0;
            }
            free(t->data[r][col]);
            t->data[r][col] = new_str;
        }
        if (t->rows > 100 && (r + 1) % (t->rows / 10 + 1) == 0) {
            printf("[bucket_col] Progress: Processed %d/%d rows\n", r + 1, t->rows);
        }
    }

    if (!preserve_header && t->headers && t->headers[col]) {
        char buf[128];
        snprintf(buf, sizeof(buf), "%s_binned_%d", t->headers[col], nbuckets);
        char *new_header = strdup(buf);
        if (!new_header) {
            printf("[bucket_col] Error: Memory allocation failed for header\n");
            return 0;
        }
        free(t->headers[col]);
        t->headers[col] = new_header;
    }

    printf("[bucket_col] Success: Column %d bucketed into %d ranges\n", col, nbuckets);
    return 1;
}

/* =========================================================
 * outlier_detection()
 * Detects outliers in numeric columns using IQR method.
 * Arguments: Table *t
 * Returns: 1 on success, 0 on failure
 * ======================================================= */
int outlier_detection(Table *t) {
    printf("[outlier_detection] Starting outlier detection across %d columns...\n", t ? t->cols : 0);
    
    if (!t || t->cols <= 0 || t->rows <= 0) {
        printf("[outlier_detection] Error: Invalid table (table=%p, rows=%d, cols=%d)\n",
               (void*)t, t ? t->rows : 0, t ? t->cols : 0);
        return 0;
    }

    int any = 0;
    for (int col = 0; col < t->cols; ++col) {
        double *vals = malloc(t->rows * sizeof(double));
        int *rows = malloc(t->rows * sizeof(int));
        if (!vals || !rows) {
            printf("[outlier_detection] Error: Memory allocation failed for column %d\n", col);
            free(vals);
            free(rows);
            return 0;
        }

        int n = 0;
        double v;
        for (int r = 0; r < t->rows; ++r) {
            if (parse_num(t->data[r][col], &v)) {
                vals[n] = v;
                rows[n] = r;
                ++n;
            }
        }

        if (n < 4) {
            printf("[outlier_detection] Skipping column %d: Insufficient numeric data (%d values)\n", col, n);
            free(vals);
            free(rows);
            continue;
        }

        qsort(vals, n, sizeof(double), cmpdbl);
        int half = n / 2;
        double q1 = median(vals, half);
        double q3 = median(vals + (n % 2 ? half + 1 : half), half);
        double iqr = q3 - q1, lo = q1 - 1.5 * iqr, hi = q3 + 1.5 * iqr;

        for (int i = 0; i < n; ++i) {
            if (vals[i] < lo || vals[i] > hi) {
                if (!any) {
                    printf("\n[outlier_detection] Outliers detected:\n");
                    any = 1;
                }
                printf("  Column %d, Row %d: Value %.6g (Q1=%.6g, Q3=%.6g, IQR=%.6g)\n",
                       col, rows[i], vals[i], q1, q3, iqr);
            }
        }
        free(vals);
        free(rows);

        if (t->rows > 100 && (col + 1) % (t->cols / 10 + 1) == 0) {
            printf("[outlier_detection] Progress: Processed %d/%d columns\n", col + 1, t->cols);
        }
    }

    if (!any) {
        printf("[outlier_detection] No outliers detected in any column.\n");
    }
    printf("[outlier_detection] Success: Outlier detection completed.\n");
    return 1;
}

/* =========================================================
 * scale_to_range()
 * Scales numeric values in a column to a specified range [a, b].
 * Arguments: Table *t, int col, double a, double b, int preserve_header
 * Returns: 1 on success, 0 on failure
 * ======================================================= */
int scale_to_range(Table *t, int col, double a, double b, int preserve_header) {
    printf("[scale_to_range] Starting scaling for column %d to range [%.2f, %.2f]...\n", col, a, b);
    
    if (!t || col < 0 || col >= t->cols || fabs(a - b) < 1e-12) {
        printf("[scale_to_range] Error: Invalid input (table=%p, col=%d, a=%.2f, b=%.2f)\n",
               (void*)t, col, a, b);
        return 0;
    }

    double vmin = DBL_MAX, vmax = -DBL_MAX, v;
    int valid_count = 0;
    for (int r = 0; r < t->rows; ++r) {
        if (parse_num(t->data[r][col], &v)) {
            if (v < vmin) vmin = v;
            if (v > vmax) vmax = v;
            valid_count++;
        }
    }

    if (valid_count == 0) {
        printf("[scale_to_range] Error: No valid numeric data in column %d\n", col);
        return 0;
    }

    if (fabs(vmax - vmin) < 1e-12) {
        printf("[scale_to_range] Warning: All values in column %d are identical (%.6g). Setting to midpoint.\n", col, vmin);
        double mid = (a + b) / 2.0;
        char buf[64];
        snprintf(buf, sizeof(buf), "%.6f", mid);
        for (int r = 0; r < t->rows; ++r) {
            if (parse_num(t->data[r][col], &v)) {
                char *new_str = strdup(buf);
                if (!new_str) {
                    printf("[scale_to_range] Error: Memory allocation failed for row %d\n", r);
                    return 0;
                }
                free(t->data[r][col]);
                t->data[r][col] = new_str;
            }
        }
        if (!preserve_header && t->headers && t->headers[col]) {
            char buf2[128];
            snprintf(buf2, sizeof(buf2), "%s_scaled_[%.0f_%.0f]", t->headers[col], a, b);
            char *new_header = strdup(buf2);
            if (!new_header) {
                printf("[scale_to_range] Error: Memory allocation failed for header\n");
                return 0;
            }
            free(t->headers[col]);
            t->headers[col] = new_header;
        }
        printf("[scale_to_range] Success: Column %d scaled to midpoint %.6f\n", col, mid);
        return 1;
    }

    double scale = (b - a) / (vmax - vmin);
    char buf[64];
    for (int r = 0; r < t->rows; ++r) {
        if (parse_num(t->data[r][col], &v)) {
            double s = a + (v - vmin) * scale;
            snprintf(buf, sizeof(buf), "%.6f", s);
            char *new_str = strdup(buf);
            if (!new_str) {
                printf("[scale_to_range] Error: Memory allocation failed for row %d\n", r);
                return 0;
            }
            free(t->data[r][col]);
            t->data[r][col] = new_str;
        }
        if (t->rows > 100 && (r + 1) % (t->rows / 10 + 1) == 0) {
            printf("[scale_to_range] Progress: Processed %d/%d rows\n", r + 1, t->rows);
        }
    }

    if (!preserve_header && t->headers && t->headers[col]) {
        char buf2[128];
        snprintf(buf2, sizeof(buf2), "%s_scaled_[%.0f_%.0f]", t->headers[col], a, b);
        char *new_header = strdup(buf2);
        if (!new_header) {
            printf("[scale_to_range] Error: Memory allocation failed for header\n");
            return 0;
        }
        free(t->headers[col]);
        t->headers[col] = new_header;
    }

    printf("[scale_to_range] Success: Column %d scaled to range [%.2f, %.2f]\n", col, a, b);
    return 1;
}

/* =========================================================
 * impute_missing()
 * Replaces missing or non-numeric values in a column with the mean.
 * Arguments: Table *t, int col, int preserve_header
 * Returns: 1 on success, 0 on failure
 * ======================================================= */
int impute_missing(Table *t, int col, int preserve_header) {
    printf("[impute_missing] Starting imputation for column %d...\n", col);
    
    if (!t || col < 0 || col >= t->cols) {
        printf("[impute_missing] Error: Invalid input (table=%p, col=%d)\n", (void*)t, col);
        return 0;
    }

    double *vals = malloc(t->rows * sizeof(double));
    if (!vals) {
        printf("[impute_missing] Error: Memory allocation failed for column %d\n", col);
        return 0;
    }

    int n = 0;
    double v, sum = 0;
    for (int r = 0; r < t->rows; ++r) {
        if (parse_num(t->data[r][col], &v)) {
            vals[n++] = v;
            sum += v;
        }
    }

    if (n == 0) {
        printf("[impute_missing] Error: No valid numeric data in column %d\n", col);
        free(vals);
        return 0;
    }

    double mean = sum / n;
    char buf[64];
    snprintf(buf, sizeof(buf), "%.6f", mean);
    int imputed_count = 0;
    for (int r = 0; r < t->rows; ++r) {
        if (!parse_num(t->data[r][col], &v)) {
            char *new_str = strdup(buf);
            if (!new_str) {
                printf("[impute_missing] Error: Memory allocation failed for row %d\n", r);
                free(vals);
                return 0;
            }
            free(t->data[r][col]);
            t->data[r][col] = new_str;
            imputed_count++;
        }
        if (t->rows > 100 && (r + 1) % (t->rows / 10 + 1) == 0) {
            printf("[impute_missing] Progress: Processed %d/%d rows\n", r + 1, t->rows);
        }
    }

    free(vals);

    if (!preserve_header && t->headers && t->headers[col]) {
        char buf2[128];
        snprintf(buf2, sizeof(buf2), "%s_imputed_mean", t->headers[col]);
        char *new_header = strdup(buf2);
        if (!new_header) {
            printf("[impute_missing] Error: Memory allocation failed for header\n");
            return 0;
        }
        free(t->headers[col]);
        t->headers[col] = new_header;
    }

    printf("[impute_missing] Success: Imputed %d missing/non-numeric values in column %d with mean %.6f\n",
           imputed_count, col, mean);
    return 1;
}

