/*
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <cstdio>
#include <cstring>
#include <map>
#include <string>

struct memory_metrics {
  size_t private_clean = 0;
  size_t private_dirty = 0;
  size_t shared_clean = 0;
  size_t shared_dirty = 0;
  size_t pss = 0;  // Proportional Set Size
  size_t swap = 0;
  size_t heap_size = 0;
  size_t stack_size = 0;
  size_t code_size = 0;
};

class memory_analyzer {
private:
  static std::string format_size(size_t kb) {
    constexpr size_t KB = 1;
    constexpr size_t MB = 1024 * KB;
    constexpr size_t GB = 1024 * MB;

    char buffer[64];
    if (kb >= GB) {
      sprintf(buffer, "%.2f GB", static_cast<double>(kb) / GB);
    } else if (kb >= MB) {
      sprintf(buffer, "%.2f MB", static_cast<double>(kb) / MB);
    } else if (kb >= KB) {
      sprintf(buffer, "%.2f KB", static_cast<double>(kb) / KB);
    } else {
      sprintf(buffer, "%zu B", kb * 1024);
    }
    return std::string(buffer);
  }

  static size_t parse_kb_value(const char* line) {
    size_t value = 0;
    while (*line && (*line < '0' || *line > '9')) line++;  // Skip to first digit
    if (*line) {
      sscanf(line, "%lu", &value);
    }
    return value;
  }

public:
  static memory_metrics analyze_memory() {
    memory_metrics metrics;
    FILE* fp = fopen("/proc/self/smaps", "r");
    if (!fp) {
      printf("Failed to open /proc/self/smaps\n");
      return metrics;
    }

    char line[256];
    bool in_heap = false;
    bool in_stack = false;
    bool in_code = false;

    while (fgets(line, sizeof(line), fp)) {
      // Check region type
      if (strstr(line, "[heap]")) {
        in_heap = true;
        in_stack = false;
        in_code = false;
      } else if (strstr(line, "[stack]")) {
        in_heap = false;
        in_stack = true;
        in_code = false;
      } else if (strstr(line, "r-xp")) {
        in_heap = false;
        in_stack = false;
        in_code = true;
      }

      // Parse metrics
      if (strncmp(line, "Private_Clean:", 13) == 0) {
        size_t val = parse_kb_value(line + 13);
        metrics.private_clean += val;
        if (in_heap) metrics.heap_size += val;
        if (in_stack) metrics.stack_size += val;
        if (in_code) metrics.code_size += val;
      } else if (strncmp(line, "Private_Dirty:", 13) == 0) {
        size_t val = parse_kb_value(line + 13);
        metrics.private_dirty += val;
        if (in_heap) metrics.heap_size += val;
        if (in_stack) metrics.stack_size += val;
        if (in_code) metrics.code_size += val;
      } else if (strncmp(line, "Shared_Clean:", 12) == 0) {
        metrics.shared_clean += parse_kb_value(line + 12);
      } else if (strncmp(line, "Shared_Dirty:", 12) == 0) {
        metrics.shared_dirty += parse_kb_value(line + 12);
      } else if (strncmp(line, "Pss:", 4) == 0) {
        metrics.pss += parse_kb_value(line + 4);
      } else if (strncmp(line, "Swap:", 5) == 0) {
        metrics.swap += parse_kb_value(line + 5);
      }
    }

    fclose(fp);
    return metrics;
  }

  static void print_report() {
    memory_metrics metrics = analyze_memory();
    printf("\n=== Memory Usage Report ===\n");
    printf("Private Clean Memory:     %s\n", format_size(metrics.private_clean).c_str());
    printf("Private Dirty Memory:     %s\n", format_size(metrics.private_dirty).c_str());
    printf("Shared Clean Memory:      %s\n", format_size(metrics.shared_clean).c_str());
    printf("Shared Dirty Memory:      %s\n", format_size(metrics.shared_dirty).c_str());
    printf("Proportional Set Size:    %s\n", format_size(metrics.pss).c_str());
    printf("Swap Usage:               %s\n", format_size(metrics.swap).c_str());

    printf("\nSpecific Regions:\n");
    printf("Heap Size:                %s\n", format_size(metrics.heap_size).c_str());
    printf("Stack Size:               %s\n", format_size(metrics.stack_size).c_str());
    printf("Code Size:                %s\n", format_size(metrics.code_size).c_str());

    size_t total_private = metrics.private_clean + metrics.private_dirty;
    size_t total_shared = metrics.shared_clean + metrics.shared_dirty;
    size_t total_rss = total_private + total_shared;

    printf("\nTotals:\n");
    printf("Total Private Memory:     %s\n", format_size(total_private).c_str());
    printf("Total Shared Memory:      %s\n", format_size(total_shared).c_str());
    printf("Total Memory (RSS):       %s\n", format_size(total_rss).c_str());
  }
};
