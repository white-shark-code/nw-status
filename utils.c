#include "utils.h"
#include <stdio.h>

VECTOR_DEFINE(char, Char, char)
VECTOR_DEFINE(Wireless, Wirelesses, wirelesses)
VECTOR_DEFINE(IPv4, AddressesIPv4, addresses_ipv4)
VECTOR_DEFINE(IPv6, AddressesIPv6, addresses_ipv6)

VecChar *slice_char(const char *str, size_t len) {
  if (!str)
    return NULL;
  size_t row_len = strlen(str);
  if (row_len < len)
    return NULL;
  VecChar *row = vec_init_char_with_capacity(len + 1);
  if (!row)
    return NULL;
  memcpy(row->data, str, len);
  row->data[len] = '\0';
  row->size = len;
  return row;
}

FILE *read_file(const char *path) {
  if (!path)
    return NULL;
  FILE *f = fopen(path, "r");
  if (!f) {
    perror("Couldn't read the file(/proc/net/wireless)");
    exit(-1);
  }
  return f;
}

Wireless get_wire_iface(const char *row) {
  Wireless wire = {0};
  if (!row)
    return wire;
  sscanf(row, " %63[^:]: %4s %f %f %f", wire.name, wire.status, &wire.link,
         &wire.level, &wire.noise);
  return wire;
}

VecWirelesses *get_all_wire_ifaces(FILE *file) {
  char *line = NULL;
  size_t capacity = 0;
  ssize_t n_bytes;

  if (getline(&line, &capacity, file) == -1) {
    free(line);
    perror("Headline is empty /proc/net/wireless");
    return NULL;
  };
  if (getline(&line, &capacity, file) == -1) {
    free(line);
    perror("Headline is empty /proc/net/wireless");
    return NULL;
  };

  VecWirelesses *wirelesses = vec_init_wirelesses_with_capacity(3);
  if (!wirelesses) {
    free(line);
    return NULL;
  }

  while ((n_bytes = getline(&line, &capacity, file)) != -1) {
    VecChar *row = slice_char(line, (size_t)n_bytes - 1);
    if (!row || !row->data) {
      fprintf(stderr, "Slice char failed for line: %s\n", line);
      if (row) vec_free_char(row);
      continue;
    }
    Wireless wire = get_wire_iface(row->data);
    if (wire.name[0] == '\0') {
      vec_free_char(row);
      printf("You don't have wireless interfaces");
      free(line);
      vec_free_wirelesses(wirelesses);
      return NULL;
    };
    vec_free_char(row);
    if (vec_push_wirelesses(wirelesses, wire)) {
      fprintf(stderr, "Wire can't push inside wirelesses: %s\n", wire.name);
      free(line);
      vec_free_wirelesses(wirelesses);
      return NULL;
    }
  }

  free(line);

  return wirelesses;
}

Network *network_init() {
  Network *network = malloc(sizeof(Network));
  if (!network) {
    return NULL;
  }

  network->wire = malloc(sizeof(Wireless));
  if (!network->wire) {
    return NULL;
  }
  network->addresses_ipv4 = vec_init_addresses_ipv4();
  network->addresses_ipv6 = vec_init_addresses_ipv6();

  return network;
}

Network *network_init_with_capacity(size_t capacity) {
  Network *network = malloc(sizeof(Network));
  if (!network) {
    return NULL;
  }

  network->wire = malloc(sizeof(Wireless));
  if (!network->wire) {
    free(network);
    return NULL;
  }
  network->addresses_ipv4 = vec_init_addresses_ipv4_with_capacity(capacity);
  network->addresses_ipv6 = vec_init_addresses_ipv6_with_capacity(capacity);

  if (!network->addresses_ipv4 || !network->addresses_ipv6) {
    free(network->wire);
    free(network);
    return NULL;
  }

  return network;
}

void network_free(Network *network) {
  if (!network) {
    return;
  }

  free(network->wire);
  vec_free_addresses_ipv4(network->addresses_ipv4);
  vec_free_addresses_ipv6(network->addresses_ipv6);
}

VecNetwork *vec_networks_init() {
  VecNetwork *networks = malloc(sizeof(VecNetwork));
  if (!networks) {
    return NULL;
  }

  networks->data = NULL;
  networks->capacity = 0;
  networks->size = 0;

  return networks;
}

int vec_networks_push(VecNetwork *networks, Network *network) {
  if (!networks) {
    return -1;
  }
  if (!network) {
    return -1;
  }

  if (networks->capacity == networks->size) {
    size_t new_capacity = networks->capacity ? networks->capacity * 2 : 8;
    Network *p = realloc(networks->data, new_capacity * sizeof(Network));
    if (!p) {
      return -1;
    }
    networks->capacity = new_capacity;
    networks->data = p;
  }

  networks->data[networks->size] = *network;
  networks->size++;
  return 0;
}

void vec_networks_free(VecNetwork *networks) {
  if (!networks) {
    return;
  }

  for (size_t i = 0; i < networks->size; i++) {
    network_free(&networks->data[i]);
  }

  free(networks->data);
  free(networks);
}

VecNetwork *prepair_networks_using_wireless(VecWirelesses *wirelesses) {
  if (!wirelesses) {
    return NULL;
  }
  if (wirelesses->size == 0) {
    return NULL;
  }

  VecNetwork *networks = vec_networks_init();
  if (!networks) {
    return NULL;
  }

  for (size_t i = 0; wirelesses->size > i; i++) {
    Network *net_i = network_init();
    if (!net_i) {
      vec_networks_free(networks);
      return NULL;
    }

    *net_i->wire = wirelesses->data[i];

    if (vec_networks_push(networks, net_i) == -1) {
      network_free(net_i);
      vec_networks_free(networks);
      return NULL;
    }
    free(net_i);
  }

  return networks;
}

ssize_t check_exist_wire(struct ifaddrs *ifa, VecNetwork *networks) {
  for (size_t i = 0; networks->size > i; i++) {
    char *wire_name = networks->data[i].wire->name;
    char *ifa_name = ifa->ifa_name;
    int ife = strcmp(ifa_name, wire_name);
    if (ife != 0)
      continue;
    else {
      return (ssize_t)i;
    }
  }
  return -1;
}

int get_addr(VecNetwork *networks) {
  struct ifaddrs *ifaddr, *ifa;
  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    exit(1);
  }

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;

    ssize_t network_id;
    if ((network_id = check_exist_wire(ifa, networks)) == -1) {
      continue;
    }

    Network *network = &networks->data[network_id];

    int ipv4_type = AF_INET;
    int ipv6_type = AF_INET6;

    if (ifa->ifa_addr->sa_family == ipv4_type) {
      struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
      IPv4 ipv4;
      inet_ntop(AF_INET, &sa->sin_addr, ipv4.address, sizeof ipv4.address);
      if (vec_push_addresses_ipv4(network->addresses_ipv4, ipv4) == -1) {
        return -1;
      }
    } else if (ifa->ifa_addr->sa_family == ipv6_type) {
      struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)ifa->ifa_addr;
      IPv6 ipv6;
      inet_ntop(AF_INET6, &sa6->sin6_addr, ipv6.address, sizeof ipv6.address);

      if (IN6_IS_ADDR_LINKLOCAL(&sa6->sin6_addr)) {
        strcpy(ipv6.type, "link-local");
      } else if (IN6_IS_ADDR_MULTICAST(&sa6->sin6_addr)) {
        strcpy(ipv6.type, "multicast"); // ff02::1 и т.п.
      } else if (IN6_IS_ADDR_ULA(&sa6->sin6_addr)) {
        strcpy(ipv6.type, "ULA"); // fc00::/7
      } else {
        strcpy(ipv6.type, "global");
      }

      ipv6.scope_id = sa6->sin6_scope_id;

      if (vec_push_addresses_ipv6(network->addresses_ipv6, ipv6) == -1) {
        return -1;
      }
    }
  }

  freeifaddrs(ifaddr);

  return 0;
}

void vec_networks_json_output(VecNetwork *networks) {
  printf("{\n");
  printf("\t\"networks\": [\n");
  for (size_t i = 0; i < networks->size; i++) {
    printf("\t\t{\n");
    printf("\t\t\t\"interface\": \"%s\",\n", networks->data[i].wire->name);
    printf("\t\t\t\"status\": \"%s\",\n", networks->data[i].wire->status);
    printf("\t\t\t\"level\": %f,\n", networks->data[i].wire->level);
    printf("\t\t\t\"link\": %f,\n", networks->data[i].wire->link);
    printf("\t\t\t\"noise\": %f,\n", networks->data[i].wire->noise);
    printf("\t\t\t\"ip-addresses\": {\n");
    printf("\t\t\t\t\"IPv4\": [");
    if (networks->data[i].addresses_ipv4->size != 0) {
      printf("\n");
      for (size_t j = 0; j < networks->data[i].addresses_ipv4->size; j++) {
        printf("\t\t\t\t\t\"%s\"",
               networks->data[i].addresses_ipv4->data[j].address);
        if (j + 1 < networks->data[i].addresses_ipv4->size) {
          printf(",");
        }
        printf("\n");
      }
      printf("\t\t\t\t]");
    } else {
      printf("]");
    }
    printf(",\n");
    printf("\t\t\t\t\"IPv6\": [");
    if (networks->data[i].addresses_ipv6->size != 0) {
      printf("\n");
      for (size_t j = 0; j < networks->data[i].addresses_ipv6->size; j++) {
        printf("\t\t\t\t\t{\"address\": \"%s\", \"type\": \"%s\", \"scope_id\": %u}",
               networks->data[i].addresses_ipv6->data[j].address,
               networks->data[i].addresses_ipv6->data[j].type,
               networks->data[i].addresses_ipv6->data[j].scope_id);
        if (j + 1 < networks->data[i].addresses_ipv6->size) {
          printf(",");
        }
        printf("\n");
      }
      printf("\t\t\t\t]");
    } else {
      printf("]");
    }
    printf("\n\t\t\t}\n");
    printf("\t\t}");
    if (i + 1 < networks->size) {
      printf(",");
    }
    printf("\n");
  }
  printf("\t]\n");
  printf("}\n");
}