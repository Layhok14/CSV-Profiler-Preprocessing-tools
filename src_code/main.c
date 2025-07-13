#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fileio.h"
#include "view.h"
#include "preprocess.h"
#include "stats.h"
#include "Investigate.h"
#include "data_preparation.h"
#include "export.h"



void intro() {
    printf("#################################################################\n");
    printf("####           CSV Profiler and Preprocessing Tool           ####\n");
    printf("#################################################################\n\n");
    printf("Welcome to the CSV Profiler and Preprocessing Tool!\n");
    printf("This system helps you analyze and preprocess CSV data files.\n");
    printf("Explore the features below through our interactive menu.\n\n");
}

int main() {
    Table table = {0};
    char filename[256];
    char choice[10];
    int sub_choice, num, col;
    char value[256];
    intro();
    
    printf("Enter CSV filename: ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = 0;

    if (!load_csv(&table, filename)) {
        printf("Failed to load CSV file.\n");
        return 1;
    }
    export_stats_csv(&table, "../Data/initial_stats.csv");
    while (1) {
        printf("\nMain Menu:\n");
        printf("[1] View data: head(num), tail(num)\n");
        printf("[2] View stats: info, stat\n");
        printf("[3] Investigate: column, count_isna(col), na_count, find_outlier()\n");
        printf("[4] Clean data (Preprocess): isna, find_na, fill_na, drop_na, convert_to_num\n");
        printf("[5] Prepare Data: bucket_col(), outlier_detection(), scale_to_range(col,a,b)\n");
        printf("[6] Quit\n");
        printf("Enter choice: ");
        fgets(choice, sizeof(choice), stdin);
        choice[strcspn(choice, "\n")] = 0;

        if (strcmp(choice, "1") == 0) {
            while (1) {
                printf("\nView Menu:\n");
                printf("[1] Head\n");
                printf("[2] Tail\n");
                printf("[3] Exit to Main Menu\n");
                printf("Enter sub-choice: ");
                if (scanf("%d", &sub_choice) != 1) {
                    while (getchar() != '\n');
                    continue;
                }
                while (getchar() != '\n');
                if (sub_choice == 3) break;

                printf("Enter number of rows: ");
                if (scanf("%d", &num) != 1) {
                    while (getchar() != '\n');
                    continue;
                }
                while (getchar() != '\n');
                if (sub_choice == 1) print_head(&table, num);
                else if (sub_choice == 2) print_tail(&table, num);
            }

        } else if (strcmp(choice, "2") == 0) {
            while (1) {
                printf("\nStats Menu:\n");
                printf("[a] info\n");
                printf("[b] stat\n");
                printf("[x] Exit to Main Menu\n");
                printf("Enter choice: ");
                fgets(choice, sizeof(choice), stdin);
                choice[strcspn(choice, "\n")] = 0;

                if (strcmp(choice, "a") == 0) {
                    print_info(&table);
                } else if (strcmp(choice, "b") == 0) {
                    print_stats(&table);
                } else if (strcmp(choice, "x") == 0) {
                    break;
                } else {
                    printf("Invalid choice. Try again.\n");
                }
            }

        } else if (strcmp(choice, "3") == 0) {
            while (1) {
                printf("\nInvestigate Menu:\n");
                printf("[a] List Columns\n");
                printf("[b] count_isna(col)\n");
                printf("[c] na_count()\n");
                printf("[d] find_outlier()\n");
                printf("[x] Exit to Main Menu\n");
                printf("Enter choice: ");
                fgets(choice, sizeof(choice), stdin);
                choice[strcspn(choice, "\n")] = 0;

                if (strcmp(choice, "a") == 0) {
                    list_columns(&table);
                } else if (strcmp(choice, "b") == 0) {
                    printf("Enter column index: ");
                    if (scanf("%d", &col) == 1) {
                        while (getchar() != '\n');
                        count_isna(&table, col);
                    }
                } else if (strcmp(choice, "c") == 0) {
                    na_count(&table);
                } else if (strcmp(choice, "d") == 0) {
                    find_outlier(&table);
                } else if (strcmp(choice, "x") == 0) {
                    break;
                } else {
                    printf("Invalid choice.\n");
                }
            }

        } else if (strcmp(choice, "4") == 0) {
            while (1) {
                printf("\nPreprocess Menu:\n");
                printf("[a] isna()\n");
                printf("[b] find_na(col)\n");
                printf("[c] fill_na(col, value)\n");
                printf("[d] drop_na(col)\n");
                printf("[e] convert_to_num(col)\n");
                printf("[x] Exit to Main Menu\n");
                printf("Enter choice: ");
                fgets(choice, sizeof(choice), stdin);
                choice[strcspn(choice, "\n")] = 0;

                if (strcmp(choice, "a") == 0) {
                    isna(&table);
                } else if (strcmp(choice, "b") == 0) {
                    printf("Enter column index: ");
                    if (scanf("%d", &col) == 1) {
                        while (getchar() != '\n');
                        find_na(&table, col);
                    }
                } else if (strcmp(choice, "c") == 0) {
                    printf("Enter column index: ");
                    if (scanf("%d", &col) == 1) {
                        while (getchar() != '\n');
                        printf("Enter value (0, mean, median): ");
                        fgets(value, sizeof(value), stdin);
                        value[strcspn(value, "\n")] = 0;
                        fill_na(&table, col, value);
                    }
                } else if (strcmp(choice, "d") == 0) {
                    printf("Enter column index: ");
                    if (scanf("%d", &col) == 1) {
                        while (getchar() != '\n');
                        drop_na(&table, col);
                    }
                } else if (strcmp(choice, "e") == 0) {
                    printf("Enter column index: ");
                    if (scanf("%d", &col) == 1) {
                        while (getchar() != '\n');
                        convert_to_num(&table, col);
                    }
                } else if (strcmp(choice, "x") == 0) {
                    break;
                } else {
                    printf("Invalid choice.\n");
                }
            }

        } else if (strcmp(choice, "5") == 0) {
            while (1) {
                printf("\nPrepare Data Menu:\n");
                printf("[a] bucket_col(col, buckets)\n");
                printf("[b] outlier_detection()\n");
                printf("[c] scale_to_range(col, a, b)\n");
                printf("[x] Exit to Main Menu\n");
                printf("Enter choice: ");
                fgets(choice, sizeof(choice), stdin);
                choice[strcspn(choice, "\n")] = 0;

                if (strcmp(choice, "a") == 0) {
                    printf("Enter column index: ");
                    if (scanf("%d", &col) == 1) {
                        while (getchar() != '\n');
                        printf("Enter number of buckets: ");
                        int buckets;
                        if (scanf("%d", &buckets) == 1) {
                            while (getchar() != '\n');
                            bucket_col(&table, col, buckets, 1);
                        }
                    }
                } else if (strcmp(choice, "b") == 0) {
                    outlier_detection(&table);
                } else if (strcmp(choice, "c") == 0) {
                    printf("Enter column index: ");
                    if (scanf("%d", &col) == 1) {
                        while (getchar() != '\n');
                        double a, b;
                        printf("Enter min (a): ");
                        scanf("%lf", &a);
                        printf("Enter max (b): ");
                        scanf("%lf", &b);
                        while (getchar() != '\n');
                        scale_to_range(&table, col, a, b, 1);
                    }
                } else if (strcmp(choice, "x") == 0) {
                    break;
                } else {
                    printf("Invalid choice.\n");
                }
            }

        } else if (strcmp(choice, "6") == 0) {
            save_csv(&table, "../Data/output_cleaned.csv");
            export_stats_csv(&table, "../Data/output_stats.csv");
            printf("Cleaned data saved to 'Data/output_cleaned.csv'.\n");
            printf("Statistics saved to 'Data/output_stats.csv'.\n");
            free_table(&table);
            break;
        } else {
            printf("Invalid main menu choice.\n");
        }
    }

    return 0;
}
