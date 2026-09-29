#include "utils.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  int short_mode = 0;
  long limit = -1;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--short") == 0 || strcmp(argv[i], "-s") == 0) {
      short_mode = 1;
    } else if (strcmp(argv[i], "--limit") == 0 || strcmp(argv[i], "-n") == 0) {
      const char *flag = argv[i];
      if (i + 1 >= argc) {
        fprintf(stderr, "Invalid value for %s: missing value\n", flag);
        return 1;
      }
      char *endptr = NULL;
      long val = strtol(argv[i + 1], &endptr, 10);
      if (endptr == argv[i + 1] || *endptr != '\0' || val < 1 ||
          val > INT_MAX) {
        fprintf(stderr, "Invalid value for %s: %s\n", flag, argv[i + 1]);
        return 1;
      }
      limit = val;
      i++;
    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      printf("Usage: %s [--short|-s] [--limit|-n N] [--help|-h]\n", argv[0]);
      return 0;
    } else {
      fprintf(stderr, "Unknown option: %s\n", argv[i]);
      return 1;
    }
  }

  FILE *file = read_file(STATIC_PATH_PROC_WIRELES);
  if (!file) {
    return 1;
  }

  VecWirelesses *wires = get_all_wire_ifaces(file);
  if (!wires) {
    fclose(file);
    return 1;
  }

  VecNetwork *ptr_networks = prepair_networks_using_wireless(wires);
  if (!ptr_networks) {
    vec_free_wirelesses(wires);
    fclose(file);
    return 1;
  }

  enrich_networks_with_ssid(ptr_networks);

  if (vec_networks_limit(ptr_networks, limit) == -1) {
    vec_networks_free(ptr_networks);
    vec_free_wirelesses(wires);
    fclose(file);
    return 1;
  }

  if (get_addr(ptr_networks) == -1) {
    vec_networks_free(ptr_networks);
    vec_free_wirelesses(wires);
    fclose(file);
    return 1;
  }

  if (short_mode)
    vec_networks_short_output(ptr_networks);
  else {
#if OUTPUT_FORMAT_TERMINAL
    vec_networks_terminal_output(ptr_networks);
#else
    vec_networks_json_output(ptr_networks);
#endif
  }

  vec_networks_free(ptr_networks);
  vec_free_wirelesses(wires);
  fclose(file);

  return 0;
}
