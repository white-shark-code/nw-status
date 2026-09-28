#include "utils.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  int short_mode = 0;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "--short") == 0 || strcmp(argv[i], "-s") == 0) {
      short_mode = 1;
    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      printf("Usage: %s [--short|-s] [--help|-h]\n", argv[0]);
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
