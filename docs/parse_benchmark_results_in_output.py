#!/bin/env python3

#!/usr/bin/env python3
import os
import re
from glob import glob

def parse_benchmark_file(filepath):
    """Parse a benchmark result file and extract statistics for each label."""
    results = []
    current_label = None

    with open(filepath, 'r') as f:
        lines = f.readlines()

    for line in lines:
        line = line.strip()

        # Find label
        if line.startswith('label'):
            # Extract label name between quotes
            match = re.search(r'label "(.*)"\.', line)
            if match:
                current_label = match.group(1)

        # Find mean and standard error
        elif line.startswith('Mean'):
            mean = float(re.search(r'Mean\s+(\d+\.\d+)', line).group(1))

        elif line.startswith('S.E. Mean'):
            se_mean = float(re.search(r'S\.E\. Mean\s+(\d+\.\d+)', line).group(1))
            if current_label:
                results.append({
                    'label': current_label,
                    'mean': mean,
                    'se_mean': se_mean
                })
                current_label = None

    return results

def print_summary(filepath, results):
    """Print summary of results in the requested format with aligned columns."""
    filename = os.path.basename(filepath)
    print("BENCHMARK TYPE:                         LABEL:                 MEAN:  (+/- STDERR)  FPS MEAN: (+/- SE)")
    for result in results:
        # Calculate FPS (1000/x) for both mean and SE
        fps_mean = 1000.0 / result['mean'] if result['mean'] != 0 else float('inf')
        fps_se = 1000.0 / result['se_mean'] if result['se_mean'] != 0 else float('inf')

        print(f"{filename:<20} {result['label']:<20} {result['mean']:>8.2f} ({result['se_mean']:>8.2f}) "
              f"{fps_mean:>8.2f} ({fps_se:>8.2f})")

def main():
    # Find all result files in the output directory
    result_files = glob('output/benchmark/RESULT_*.TXT')  # Changed to .TXT

    if not result_files:
        print("No result files found in the output directory!")
        return

    # Process each file
    for filepath in sorted(result_files):
        try:
            results = parse_benchmark_file(filepath)
            print_summary(filepath, results)
        except Exception as e:
            print(f"Error processing {filepath}: {str(e)}")
            continue

if __name__ == "__main__":
    main()
