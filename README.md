
# CSV Profiler and Preprocessing Tool

## Overview
This project is a C-based tool designed to load, analyze, and preprocess CSV data files. It provides features for viewing data, computing statistics, investigating data quality, cleaning data, and preparing data for further analysis. The tools are useful for data profiling, cleaning, and feature preparation tasks and can serve as a lightweight preprocessing component in data pipelines or machine learning workflows.

## Project Structure
- `Data/`: Contains input CSV files (e.g., `book.csv`) and output files (e.g., `output_data.csv`).
- `include/`: Header files defining interfaces.
- `src_code/`: Source code files implementing the tool.
- `.vscode/`: VSCode configuration for building the project.

## Installation
1. Ensure GCC is installed on your system.
2. Place your CSV file (e.g., `book.csv`) in the `Data/` directory.
3. Open the project in VSCode and build using `Ctrl+Shift+B`.

## Usage
1. Run `./csv_tool.exe` from the `src_code` directory.
2. Enter the CSV filename (e.g., `book.csv`).
3. Use the interactive menu to:
   - `[1]` View data (head/tail).
   - `[2]` View statistics.
   - `[3]` Investigate data.
   - `[4]` Clean data (e.g., count N/A with `isna`).
   - `[5]` Prepare data.
   - `[6]` Quit and save to `output_cleaned.csv` for new data and `output_stats.csv`.

## Notes
- I don't create a GUI verison of this, even if the GUI would provide better experience and interractivity, as I plan to develop it further in order to implement in a data pipelines or ML flows in the future.
- Modifications are applied in memory and saved to `output_cleaned.csv`, leaving the original `book.csv` unchanged.
- Debug output is available under `[4]` to diagnose issues.
- To prepare data for machine Learning, use the `[5]`.
- Shoutout to Chun Chomroen for contribution in this project.
## Limitation:
- The features for data preparation are not yet configured to a very practical and professional usages. We are not able to configure it to create a new column for every labeling yet due to tight deadline for submission to school. It modifys the existing columns that the user ask to work on for now.
- Features like min-max normalization and Gussian Distribution are not yet implemented either. It is a part of future development.

## License
This project is under MIT license.
