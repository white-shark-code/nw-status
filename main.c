#include "utils.h"
#include <stdio.h>

int main(void) {
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

  if (get_addr(ptr_networks) == -1) {
    vec_networks_free(ptr_networks);
    vec_free_wirelesses(wires);
    fclose(file);
    return 1;
  }

#if OUTPUT_FORMAT_TERMINAL
  vec_networks_terminal_output(ptr_networks);
#else
  vec_networks_json_output(ptr_networks);
#endif

  vec_networks_free(ptr_networks);
  vec_free_wirelesses(wires);
  fclose(file);

  return 0;
}
