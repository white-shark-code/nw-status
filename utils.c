#include "utils.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
#include <linux/wireless.h>

#ifndef IW_ESSID_OFF
#define IW_ESSID_OFF 0
#endif

#include "config.h"

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
  assert(len <= row_len);
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
    return NULL;
  }
  return f;
}

int get_wireless_ssid(const char *ifname, char *ssid_buf, size_t buf_len) {
  if (!ifname || !ssid_buf || buf_len == 0)
    return -1;

  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (fd < 0)
    return -1;

  struct iwreq wrq;
  memset(&wrq, 0, sizeof(wrq));
  strncpy(wrq.ifr_name, ifname, IFNAMSIZ - 1);
  wrq.ifr_name[IFNAMSIZ - 1] = '\0';

  char essid[IW_ESSID_MAX_SIZE + 1];
  memset(essid, 0, sizeof(essid));
  wrq.u.essid.pointer = essid;
  wrq.u.essid.length = IW_ESSID_MAX_SIZE + 1;
  wrq.u.essid.flags = 0;

  if (ioctl(fd, SIOCGIWESSID, &wrq) < 0) {
    ssid_buf[0] = '\0';
    close(fd);
    return -1;
  }

  if (wrq.u.essid.flags == IW_ESSID_OFF || wrq.u.essid.length == 0) {
    ssid_buf[0] = '\0';
    close(fd);
    return -1;
  }

  size_t essid_len = (size_t)wrq.u.essid.length;
  if (essid_len > (size_t)IW_ESSID_MAX_SIZE)
    essid_len = (size_t)IW_ESSID_MAX_SIZE;
  if (essid_len > buf_len - 1)
    essid_len = buf_len - 1;

  memcpy(ssid_buf, essid, essid_len);
  ssid_buf[essid_len] = '\0';
  ssid_buf[buf_len - 1] = '\0';

  close(fd);
  return 0;
}

void enrich_networks_with_ssid(VecNetwork *networks) {
  if (!networks)
    return;

  for (size_t i = 0; i < networks->size; i++) {
    if (!networks->data[i].wire)
      continue;
    int ret = get_wireless_ssid(networks->data[i].wire->name,
                                networks->data[i].wire->ssid,
                                sizeof(networks->data[i].wire->ssid));
    if (ret == -1) {
      networks->data[i].wire->ssid[0] = '\0';
    }
  }
}

Wireless get_wire_iface(const char *row) {
  Wireless wire = {0};
  if (!row)
    return wire;
  int parsed = sscanf(row, " %63[^:]: %4s %f %f %f", wire.name, wire.status,
                      &wire.link, &wire.level, &wire.noise);
  if (parsed != 5) {
    return (Wireless){0};
  }
  return wire;
}

VecWirelesses *get_all_wire_ifaces(FILE *file) {
  char *line = NULL;
  size_t capacity = 0;
  ssize_t n_bytes;
  VecWirelesses *wirelesses = NULL;
  int ret = -1;

  if (getline(&line, &capacity, file) == -1) {
    perror("Headline is empty /proc/net/wireless");
    goto cleanup;
  };
  if (getline(&line, &capacity, file) == -1) {
    perror("Headline is empty /proc/net/wireless");
    goto cleanup;
  };

  wirelesses = vec_init_wirelesses_with_capacity(3);
  if (!wirelesses) {
    goto cleanup;
  }

  while ((n_bytes = getline(&line, &capacity, file)) != -1) {
    VecChar *row = slice_char(line, (size_t)n_bytes - 1);
    if (!row || !row->data) {
      fprintf(stderr, "Slice char failed for line: %s\n", line);
      if (row)
        vec_free_char(row);
      continue;
    }
    Wireless wire = get_wire_iface(row->data);
    if (wire.name[0] == '\0') {
      vec_free_char(row);
      printf("You don't have wireless interfaces");
      ret = 0;
      goto cleanup;
    };
    vec_free_char(row);
    if (vec_push_wirelesses(wirelesses, wire)) {
      fprintf(stderr, "Wire can't push inside wirelesses: %s\n", wire.name);
      goto cleanup;
    }
  }

  ret = 0;

cleanup:
  free(line);
  if (ret != 0 && wirelesses) {
    vec_free_wirelesses(wirelesses);
    wirelesses = NULL;
  }
  return wirelesses;
}

int is_noise_available(float noise) {
  return noise > NOISE_UNAVAILABLE_THRESHOLD;
}

const char *get_signal_quality(float level) {
  for (size_t i = 0; i < SIGNAL_LEVEL_COUNT; i++) {
    if (level < signal_levels[i].threshold) {
      return signal_levels[i].label;
    }
  }
  return signal_levels[SIGNAL_LEVEL_COUNT - 1].label;
}

const char *get_signal_icon(float level) {
  for (size_t i = 0; i < SIGNAL_LEVEL_COUNT; i++) {
    if (level < signal_levels[i].threshold) {
      return signal_levels[i].icon;
    }
  }
  return signal_levels[SIGNAL_LEVEL_COUNT - 1].icon;
}

const char *get_signal_color(float level) {
  for (size_t i = 0; i < SIGNAL_LEVEL_COUNT; i++) {
    if (level < signal_levels[i].threshold) {
      return signal_levels[i].color;
    }
  }
  return signal_levels[SIGNAL_LEVEL_COUNT - 1].color;
}

const char *get_interface_icon(const char *iface __attribute__((unused))) {
#if INTERFACE_ICON_COUNT > 0
  extern const interface_icon_t interface_icons[];
  for (size_t i = 0; i < INTERFACE_ICON_COUNT; i++) {
    if (strcmp(iface, interface_icons[i].iface) == 0) {
      return interface_icons[i].icon;
    }
  }
#endif
  return DEFAULT_INTERFACE_ICON;
}

const char *get_interface_icon_color(const char *iface
                                     __attribute__((unused))) {
#if INTERFACE_ICON_COUNT > 0
  extern const interface_icon_t interface_icons[];
  for (size_t i = 0; i < INTERFACE_ICON_COUNT; i++) {
    if (strcmp(iface, interface_icons[i].iface) == 0) {
      return interface_icons[i].color ? interface_icons[i].color
                                      : DEFAULT_INTERFACE_ICON_COLOR;
    }
  }
#endif
  return DEFAULT_INTERFACE_ICON_COLOR;
}

const char *get_link_icon(int link __attribute__((unused))) {
#if LINK_ICON_COUNT > 0
  extern const link_icon_t link_icons[];
  for (size_t i = 0; i < LINK_ICON_COUNT; i++) {
    if (link >= link_icons[i].threshold) {
      return link_icons[i].icon;
    }
  }
#endif
  return DEFAULT_LINK_ICON;
}

const char *get_link_icon_color(int link __attribute__((unused))) {
#if LINK_ICON_COUNT > 0
  extern const link_icon_t link_icons[];
  for (size_t i = 0; i < LINK_ICON_COUNT; i++) {
    if (link >= link_icons[i].threshold) {
      return link_icons[i].color ? link_icons[i].color
                                 : DEFAULT_LINK_ICON_COLOR;
    }
  }
#endif
  return DEFAULT_LINK_ICON_COLOR;
}

const char *get_level_icon(float level __attribute__((unused))) {
#if LEVEL_ICON_COUNT > 0
  extern const level_icon_t level_icons[];
  for (size_t i = 0; i < LEVEL_ICON_COUNT; i++) {
    if (level >= level_icons[i].threshold) {
      return level_icons[i].icon;
    }
  }
#endif
  return DEFAULT_LEVEL_ICON;
}

const char *get_level_icon_color(float level __attribute__((unused))) {
#if LEVEL_ICON_COUNT > 0
  extern const level_icon_t level_icons[];
  for (size_t i = 0; i < LEVEL_ICON_COUNT; i++) {
    if (level >= level_icons[i].threshold) {
      return level_icons[i].color ? level_icons[i].color
                                  : DEFAULT_LEVEL_ICON_COLOR;
    }
  }
#endif
  return DEFAULT_LEVEL_ICON_COLOR;
}

const char *get_ipv4_icon(void) { return DEFAULT_IPV4_ICON; }

const char *get_ipv4_icon_color(void) { return DEFAULT_IPV4_ICON_COLOR; }

const char *get_ipv6_icon(const IPv6 *ip) {
  if (!ip)
    return DEFAULT_IPV6_ICON;
  if (strcmp(ip->type, "multicast") == 0)
    return IPV6_ICON_MULTICAST;
  if (strcmp(ip->type, "link-local") == 0)
    return IPV6_ICON_LINK_LOCAL;
  if (strcmp(ip->type, "ULA") == 0)
    return IPV6_ICON_ULA;
  return IPV6_ICON_GLOBAL;
}

const char *get_ipv6_icon_color(const IPv6 *ip) {
  if (!ip)
    return DEFAULT_IPV6_ICON_COLOR;
  if (strcmp(ip->type, "multicast") == 0)
    return "#ff8800";
  if (strcmp(ip->type, "link-local") == 0)
    return "#ffff00";
  if (strcmp(ip->type, "ULA") == 0)
    return "#00ffff";
  return "#ff8800";
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
  memset(network->wire, 0, sizeof(Wireless));
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
  memset(network->wire, 0, sizeof(Wireless));
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
    if (new_capacity > SIZE_MAX / sizeof(Network)) {
      return -1;
    }
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

int vec_networks_limit(VecNetwork *networks, long limit) {
  if (!networks) {
    return -1;
  }
  if (limit < 0) {
    return 0;
  }
  if (networks->size == 0) {
    return 0;
  }

  Network *buf = malloc(networks->size * sizeof(Network));
  if (!buf) {
    return -1;
  }

  size_t pos = 0;
  for (size_t i = 0; i < networks->size; i++) {
    if (networks->data[i].wire && networks->data[i].wire->ssid[0] != '\0') {
      buf[pos] = networks->data[i];
      pos++;
    }
  }
  for (size_t i = 0; i < networks->size; i++) {
    if (!(networks->data[i].wire && networks->data[i].wire->ssid[0] != '\0')) {
      buf[pos] = networks->data[i];
      pos++;
    }
  }
  memcpy(networks->data, buf, networks->size * sizeof(Network));
  free(buf);

  if ((size_t)limit < networks->size) {
    for (size_t i = (size_t)limit; i < networks->size; i++) {
      network_free(&networks->data[i]);
    }
    networks->size = (size_t)limit;
  }

  return 0;
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

ssize_t get_index_wire(const char *ifa_name, VecNetwork *networks) {
  for (size_t i = 0; networks->size > i; i++) {
    if (strcmp(ifa_name, networks->data[i].wire->name) == 0) {
      return (ssize_t)i;
    }
  }
  return -1;
}

int get_addr(VecNetwork *networks) {
#if !OUTPUT_SHOW_IPV4 && !OUTPUT_SHOW_IPV6
  (void)networks;
  return 0;
#else
  struct ifaddrs *ifaddr, *ifa;
  if (getifaddrs(&ifaddr) == -1) {
    perror("getifaddrs");
    exit(1);
  }

  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == NULL)
      continue;

    ssize_t network_id;
    if ((network_id = get_index_wire(ifa->ifa_name, networks)) == -1) {
      continue;
    }

    Network *network = &networks->data[network_id];

#if OUTPUT_SHOW_IPV4
    int ipv4_type = AF_INET;
#endif
#if OUTPUT_SHOW_IPV6
    int ipv6_type = AF_INET6;
#endif

#if OUTPUT_SHOW_IPV4
    if (ifa->ifa_addr->sa_family == ipv4_type) {
      struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
      IPv4 ipv4;
      inet_ntop(AF_INET, &sa->sin_addr, ipv4.address, sizeof ipv4.address);
      if (vec_push_addresses_ipv4(network->addresses_ipv4, ipv4) == -1) {
        return -1;
      }
    }
#endif
#if OUTPUT_SHOW_IPV6
    if (ifa->ifa_addr->sa_family == ipv6_type) {
      struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)ifa->ifa_addr;
      IPv6 ipv6;
      inet_ntop(AF_INET6, &sa6->sin6_addr, ipv6.address, sizeof ipv6.address);

      if (IN6_IS_ADDR_LINKLOCAL(&sa6->sin6_addr)) {
        strcpy(ipv6.type, "link-local");
      } else if (IN6_IS_ADDR_MULTICAST(&sa6->sin6_addr)) {
        strcpy(ipv6.type, "multicast");
      } else if (IN6_IS_ADDR_ULA(&sa6->sin6_addr)) {
        strcpy(ipv6.type, "ULA");
      } else {
        strcpy(ipv6.type, "global");
      }

      ipv6.scope_id = sa6->sin6_scope_id;

      if (vec_push_addresses_ipv6(network->addresses_ipv6, ipv6) == -1) {
        return -1;
      }
    }
#endif
  }

  freeifaddrs(ifaddr);

  return 0;
#endif
}

/* Shared helpers for field filtering */

const char *get_interface_status_str(const Wireless *wire) {
  if (!wire)
    return "DOWN";
  return (strcmp(wire->status, "down") != 0) ? "UP" : "DOWN";
}

int should_show_ipv6_type(const char *type) {
  if (!type)
    return 0;
  if (strcmp(type, "multicast") == 0)
    return OUTPUT_IPV6_TYPE_MULTICAST;
  if (strcmp(type, "link-local") == 0)
    return OUTPUT_IPV6_TYPE_LINK_LOCAL;
  if (strcmp(type, "ULA") == 0)
    return OUTPUT_IPV6_TYPE_ULA;
  if (strcmp(type, "global") == 0)
    return OUTPUT_IPV6_TYPE_GLOBAL;
  return 0;
}

int should_show_ipv6_scope(uint32_t scope) {
  if (OUTPUT_IPV6_MAX_SCOPE_ID == 0)
    return 1;
  return scope <= OUTPUT_IPV6_MAX_SCOPE_ID;
}

/* JSON output static helper functions */

static void json_print_header(const char *indent, const char *newline) {
  printf("{%s", newline);
  printf("%s\"networks\": [%s", indent, newline);
}

static void json_print_footer(const char *indent, const char *newline) {
  printf("%s]%s", indent, newline);
  printf("}%s", newline);
}

static void json_escape_ssid(const char *src, char *dst, size_t dst_size) {
  size_t j = 0;
  if (dst_size == 0)
    return;
  if (!src) {
    dst[0] = '\0';
    return;
  }
  for (size_t i = 0; src[i] != '\0'; i++) {
    char c = src[i];
    if (c == '"' || c == '\\') {
      if (j + 2 >= dst_size)
        break;
      dst[j++] = '\\';
      dst[j++] = c;
    } else {
      if (j + 1 >= dst_size)
        break;
      dst[j++] = c;
    }
  }
  dst[j] = '\0';
}

static int json_print_interface_field(const Wireless *wire, const char *indent,
                                      const char *newline) {
  int printed = 0;
  if (OUTPUT_SHOW_INTERFACE_NAME) {
    printf("%s%s%s\"interface\": \"%s\"", indent, indent, indent, wire->name);
    printed = 1;
  }
#if OUTPUT_SHOW_SSID
  {
    const char *ssid_src =
        (wire && wire->ssid[0] != '\0') ? wire->ssid : "noname";
    char escaped[67];
    json_escape_ssid(ssid_src, escaped, sizeof(escaped));
    if (printed)
      printf(",%s", newline);
    printf("%s%s%s\"ssid\": \"%s\"", indent, indent, indent, escaped);
    printed = 1;
  }
#endif
#if JSON_OUTPUT_ICONS
  if (OUTPUT_SHOW_INTERFACE_NAME) {
    const char *iface_icon = get_interface_icon(wire->name);
    const char *iface_icon_color = get_interface_icon_color(wire->name);
    if (printed)
      printf(",%s", newline);
    printf("%s%s%s%s\"interface_icon\": \"%s\"", indent, indent, indent, indent,
           iface_icon);
    printed = 1;
    if (printed)
      printf(",%s", newline);
    printf("%s%s%s%s\"interface_icon_color\": \"%s\"", indent, indent, indent,
           indent, iface_icon_color ? iface_icon_color : "");
    printed = 1;
  }
#endif
  return printed;
}

static int json_print_signal_object(const Wireless *wire, const char *indent,
                                    const char *newline) {
  const char *quality = get_signal_quality(wire->level);
  int printed_field = 0;

  printf("%s%s%s\"signal\": {%s", indent, indent, indent, newline);

  if (OUTPUT_SHOW_LINK_QUALITY) {
    if (printed_field)
      printf(",%s", newline);
    printf("%s%s%s%s\"quality\": \"%s\"", indent, indent, indent, indent,
           quality);
    printed_field = 1;
  }
#if JSON_OUTPUT_ICONS
  if (OUTPUT_SHOW_LINK_QUALITY) {
    const char *icon = get_signal_icon(wire->level);
    const char *link_icon = get_link_icon((int)wire->link);
    const char *link_icon_color = get_link_icon_color((int)wire->link);
    const char *level_icon = get_level_icon(wire->level);
    const char *level_icon_color = get_level_icon_color(wire->level);
    if (printed_field)
      printf(",%s", newline);
    printf("%s%s%s%s\"icon\": \"%s\"", indent, indent, indent, indent, icon);
    printed_field = 1;
    if (printed_field)
      printf(",%s", newline);
    printf("%s%s%s%s\"link_icon\": \"%s\"", indent, indent, indent, indent,
           link_icon);
    if (printed_field)
      printf(",%s", newline);
    printf("%s%s%s%s\"link_icon_color\": \"%s\"", indent, indent, indent,
           indent, link_icon_color ? link_icon_color : "");
    if (printed_field)
      printf(",%s", newline);
    printf("%s%s%s%s\"level_icon\": \"%s\"", indent, indent, indent, indent,
           level_icon);
    if (printed_field)
      printf(",%s", newline);
    printf("%s%s%s%s\"level_icon_color\": \"%s\"", indent, indent, indent,
           indent, level_icon_color ? level_icon_color : "");
    printed_field = 1;
  }
#endif
  if (OUTPUT_SHOW_SIGNAL_LEVEL) {
    if (printed_field)
      printf(",%s", newline);
    printf("%s%s%s%s\"level_dbm\": %.1f", indent, indent, indent, indent,
           wire->level);
    printed_field = 1;
  }
  if (OUTPUT_SHOW_LINK_QUALITY) {
    if (printed_field)
      printf(",%s", newline);
    printf("%s%s%s%s\"link\": %.1f", indent, indent, indent, indent,
           wire->link);
    printed_field = 1;
  }
  if (OUTPUT_SHOW_SIGNAL_LEVEL) {
    const char *color = get_signal_color(wire->level);
    if (printed_field)
      printf(",%s", newline);
    printf("%s%s%s%s\"color\": \"%s\"", indent, indent, indent, indent,
           color ? color : "");
    printed_field = 1;
  }
  if (OUTPUT_SHOW_NOISE_LEVEL) {
    if (printed_field)
      printf(",%s", newline);
    if (is_noise_available(wire->noise)) {
      printf("%s%s%s%s\"noise_dbm\": %.1f", indent, indent, indent, indent,
             wire->noise);
    } else {
      printf("%s%s%s%s\"noise_dbm\": null", indent, indent, indent, indent);
    }
    printed_field = 1;
  }
  if (newline[0] != '\0') {
    printf("%s", newline);
  }
  printf("%s%s%s}", indent, indent, indent);
  return printed_field;
}

static void json_print_ipv4_array(const VecAddressesIPv4 *addrs,
                                  const char *indent, const char *newline) {
  printf("%s%s%s%s\"ipv4\": [%s", indent, indent, indent, indent, newline);

  if (addrs && addrs->size != 0) {
    for (size_t j = 0; j < addrs->size; j++) {
      printf("%s%s%s%s%s\"%s\"%s", indent, indent, indent, indent, indent,
             addrs->data[j].address, (j + 1 < addrs->size) ? "," : "");
      printf("%s", newline);
    }
    printf("%s%s%s%s]", indent, indent, indent, indent);
  } else {
    printf("%s%s%s%s]", indent, indent, indent, indent);
  }
  printf(",%s", newline);
}

static void json_print_ipv6_entry(const IPv6 *ip, const char *indent,
                                  const char *newline __attribute__((unused))) {
  printf("%s%s%s%s%s{\"address\": \"%s\", \"type\": \"%s\", "
         "\"scope_id\": %u}%s",
         indent, indent, indent, indent, indent, ip->address, ip->type,
         ip->scope_id, "");
}

static void json_print_ipv6_array(const VecAddressesIPv6 *addrs,
                                  const char *indent, const char *newline) {
  printf("%s%s%s%s\"ipv6\": [%s", indent, indent, indent, indent, newline);

  if (addrs && addrs->size != 0) {
    int first = 1;
    for (size_t j = 0; j < addrs->size; j++) {
      const IPv6 *ip = &addrs->data[j];
      if (!should_show_ipv6_type(ip->type) ||
          !should_show_ipv6_scope(ip->scope_id))
        continue;

      if (!first) {
        printf(",%s", newline);
      }
      first = 0;
      json_print_ipv6_entry(ip, indent, newline);
    }
    if (!first) {
      printf("%s", newline);
    }
    printf("%s%s%s%s]%s", indent, indent, indent, indent, newline);
  } else {
    printf("%s%s%s%s]%s", indent, indent, indent, indent, newline);
  }
}

static int json_print_ip_object(const Network *net, const char *indent,
                                const char *newline) {
  int printed = 0;

  if (OUTPUT_SHOW_IPV4 && net->addresses_ipv4 &&
      net->addresses_ipv4->size > 0) {
    if (!printed) {
      printf("%s%s%s\"ip\": {%s", indent, indent, indent, newline);
      printed = 1;
    }
    json_print_ipv4_array(net->addresses_ipv4, indent, newline);
  }
  if (OUTPUT_SHOW_IPV6 && net->addresses_ipv6 &&
      net->addresses_ipv6->size > 0) {
    if (!printed) {
      printf("%s%s%s\"ip\": {%s", indent, indent, indent, newline);
      printed = 1;
    }
    json_print_ipv6_array(net->addresses_ipv6, indent, newline);
  }
  if (printed) {
    if (newline[0] != '\0') {
      printf("%s", newline);
    }
    printf("%s%s%s}%s", indent, indent, indent, newline);
  }
  return printed;
}

static void json_print_network(const Network *net, const char *indent,
                               const char *newline) {
  const Wireless *wire = net->wire;
  int section_printed = 0;

  printf("%s%s{%s", indent, indent, newline);

  if (json_print_interface_field(wire, indent, newline)) {
    section_printed = 1;
  }
  if (section_printed &&
      (OUTPUT_SHOW_LINK_QUALITY || OUTPUT_SHOW_SIGNAL_LEVEL ||
       OUTPUT_SHOW_NOISE_LEVEL)) {
    printf(",%s", newline);
  }
  if (json_print_signal_object(wire, indent, newline)) {
    section_printed = 1;
  }
  if (section_printed && (OUTPUT_SHOW_IPV4 || OUTPUT_SHOW_IPV6)) {
    printf(",%s", newline);
  }
  if (json_print_ip_object(net, indent, newline)) {
    section_printed = 1;
  }

  printf("%s%s}%s", indent, indent, newline);
}

void vec_networks_json_output(VecNetwork *networks) {
  if (!networks) {
    return;
  }
  const char *indent = JSON_OUTPUT_COMPACT ? "" : "\t";
  const char *newline = JSON_OUTPUT_COMPACT ? "" : "\n";

  json_print_header(indent, newline);

  for (size_t i = 0; i < networks->size; i++) {
    json_print_network(&networks->data[i], indent, newline);
    if (i + 1 < networks->size) {
      printf("%s,%s", indent, newline);
    }
  }

  json_print_footer(indent, newline);
}

/* Terminal output helper functions */

static int __attribute__((unused)) utf8_width(const char *str) {
  int width = 0;
  if (!str)
    return 0;
  for (const unsigned char *p = (const unsigned char *)str; *p; p++) {
    if ((*p & 0xC0) != 0x80)
      width++;
  }
  return width;
}

static int __attribute__((unused))
calculate_max_width(const VecNetwork *networks) {
  int max_width = 60;
  for (size_t i = 0; i < networks->size; i++) {
    const Wireless *wire = networks->data[i].wire;
    const char *iface_icon = get_interface_icon(wire->name);
    const char *ssid = wire->ssid[0] ? wire->ssid : "noname";
    char title[160];
    snprintf(title, sizeof(title), "%s %s (%s, %s)", iface_icon, ssid,
             wire->name, get_interface_status_str(wire));
    int title_width = utf8_width(title);
    if (title_width > max_width)
      max_width = title_width;
    if (networks->data[i].addresses_ipv4) {
      for (size_t j = 0; j < networks->data[i].addresses_ipv4->size; j++) {
        int w = snprintf(NULL, 0, " %s ",
                         networks->data[i].addresses_ipv4->data[j].address);
        if (w > max_width)
          max_width = w;
      }
    }
    if (networks->data[i].addresses_ipv6) {
      for (size_t j = 0; j < networks->data[i].addresses_ipv6->size; j++) {
        int w = snprintf(NULL, 0, " %s %s (%u) ",
                         networks->data[i].addresses_ipv6->data[j].address,
                         networks->data[i].addresses_ipv6->data[j].type,
                         networks->data[i].addresses_ipv6->data[j].scope_id);
        if (w > max_width)
          max_width = w;
      }
    }
  }
  if (max_width < 60)
    max_width = 60;
  if (max_width > 80)
    max_width = 80;
  return max_width;
}

static void __attribute__((unused))
print_box_top(int width, const char *title) {
  printf("  +");
  if (title) {
    int title_len = utf8_width(title);
    int padding = (width - title_len - 2) / 2;
    for (int i = 0; i < padding; i++)
      printf("-");
    printf(" %s ", title);
    for (int i = 0; i < width - padding - title_len - 2; i++)
      printf("-");
  } else {
    for (int i = 0; i < width; i++)
      printf("-");
  }
  printf("+\n");
}

static void __attribute__((unused)) print_box_bottom(int width) {
  printf("  +");
  for (int i = 0; i < width; i++)
    printf("-");
  printf("+\n");
}

static void __attribute__((unused))
print_box_line(int width, const char *content) {
  int content_len = utf8_width(content);
  printf("  | %s", content);
  for (int i = 0; i < width - content_len - 1; i++)
    printf(" ");
  printf("|\n");
}

static void __attribute__((unused))
terminal_print_verbose_network(const Network *net, int width) {
  const Wireless *wire = net->wire;
  const char *status = get_interface_status_str(wire);
  const char *iface_icon = get_interface_icon(wire->name);
  const char *signal_icon = get_signal_icon(wire->level);
  const char *quality = get_signal_quality(wire->level);
  const char *link_icon = get_link_icon((int)wire->link);
  const char *level_icon = get_level_icon(wire->level);

  char title[160];
  const char *ssid = wire->ssid[0] ? wire->ssid : "noname";
  snprintf(title, sizeof(title), "%s %s (%s, %s)", iface_icon, ssid, wire->name,
           status);
  print_box_top(width, title);

  char line[256];
  size_t pos = 0;

  if (OUTPUT_SHOW_SIGNAL_LEVEL || OUTPUT_SHOW_LINK_QUALITY ||
      OUTPUT_SHOW_NOISE_LEVEL) {
    pos = 0;
    if (OUTPUT_SHOW_SIGNAL_LEVEL) {
      pos += (size_t)snprintf(line + pos, sizeof(line) - pos, "%s %s ",
                              signal_icon, quality);
    }
    if (OUTPUT_SHOW_LINK_QUALITY) {
      pos += (size_t)snprintf(line + pos, sizeof(line) - pos,
                              "%s Link: %.0f%%  ", link_icon, wire->link);
    }
    if (OUTPUT_SHOW_NOISE_LEVEL) {
      if (is_noise_available(wire->noise)) {
        pos += (size_t)snprintf(line + pos, sizeof(line) - pos,
                                "%s Noise: %.0fdBm", level_icon, wire->noise);
      } else {
        pos += (size_t)snprintf(line + pos, sizeof(line) - pos,
                                "%s Noise: n/a", level_icon);
      }
    }
    print_box_line(width, line);

    if (OUTPUT_SHOW_SIGNAL_LEVEL) {
      pos = (size_t)snprintf(line, sizeof(line), "%s Level:  %.1f dBm",
                             level_icon, wire->level);
      print_box_line(width, line);
    }
  }

  if (OUTPUT_SHOW_IPV4 && net->addresses_ipv4 &&
      net->addresses_ipv4->size > 0) {
    print_box_top(width, "IPv4");
    const char *ipv4_icon = get_ipv4_icon();
    for (size_t j = 0; j < net->addresses_ipv4->size; j++) {
      snprintf(line, sizeof(line), "%s  %s", ipv4_icon,
               net->addresses_ipv4->data[j].address);
      print_box_line(width, line);
    }
  }

  if (OUTPUT_SHOW_IPV6 && net->addresses_ipv6 &&
      net->addresses_ipv6->size > 0) {
    int has_visible = 0;
    for (size_t j = 0; j < net->addresses_ipv6->size; j++) {
      if (should_show_ipv6_type(net->addresses_ipv6->data[j].type) &&
          should_show_ipv6_scope(net->addresses_ipv6->data[j].scope_id)) {
        has_visible = 1;
        break;
      }
    }
    if (has_visible) {
      print_box_top(width, "IPv6");
      for (size_t j = 0; j < net->addresses_ipv6->size; j++) {
        const IPv6 *ip = &net->addresses_ipv6->data[j];
        if (!should_show_ipv6_type(ip->type) ||
            !should_show_ipv6_scope(ip->scope_id))
          continue;
        const char *ipv6_icon = get_ipv6_icon(ip);
        if (ip->scope_id > 0) {
          snprintf(line, sizeof(line), "%s  %s  %s (%u)", ipv6_icon,
                   ip->address, ip->type, ip->scope_id);
        } else {
          snprintf(line, sizeof(line), "%s  %s  %s", ipv6_icon, ip->address,
                   ip->type);
        }
        print_box_line(width, line);
      }
    }
  }

  print_box_bottom(width);
}

static void __attribute__((unused))
terminal_print_compact_network(const Network *net) {
  const Wireless *wire = net->wire;
  const char *status = get_interface_status_str(wire);
  const char *iface_icon = get_interface_icon(wire->name);
  const char *signal_icon = get_signal_icon(wire->level);
  const char *link_icon = get_link_icon((int)wire->link);
  const char *level_icon = get_level_icon(wire->level);
  const char *ipv4_icon = get_ipv4_icon();

  printf("%s %s (%s)", iface_icon, wire->name, status);
  {
    const char *ssid = wire->ssid[0] ? wire->ssid : "noname";
    printf("  %s", ssid);
  }

  if (OUTPUT_SHOW_SIGNAL_LEVEL) {
    printf("  %s %.1fdBm", signal_icon, wire->level);
  }
  if (OUTPUT_SHOW_LINK_QUALITY) {
    printf("  %s%.0f%%", link_icon, wire->link);
  }
  if (OUTPUT_SHOW_NOISE_LEVEL) {
    if (is_noise_available(wire->noise)) {
      printf("  %s%.0fdBm", level_icon, wire->noise);
    } else {
      printf("  %s n/a", level_icon);
    }
  }

  if (OUTPUT_SHOW_IPV4 && net->addresses_ipv4) {
    for (size_t j = 0; j < net->addresses_ipv4->size; j++) {
      printf("  %s %s", ipv4_icon, net->addresses_ipv4->data[j].address);
    }
  }
  if (OUTPUT_SHOW_IPV6 && net->addresses_ipv6) {
    for (size_t j = 0; j < net->addresses_ipv6->size; j++) {
      const IPv6 *ip = &net->addresses_ipv6->data[j];
      if (!should_show_ipv6_type(ip->type) ||
          !should_show_ipv6_scope(ip->scope_id))
        continue;
      const char *icon = get_ipv6_icon(ip);
      printf("  %s %s", icon, ip->address);
    }
  }
  printf("\n");
}

static void __attribute__((unused))
terminal_print_minimal_network(const Network *net) {
  const Wireless *wire = net->wire;
  const char *iface_icon = get_interface_icon(wire->name);
  const char *signal_icon = get_signal_icon(wire->level);
  const char *link_icon = get_link_icon((int)wire->link);
  const char *level_icon = get_level_icon(wire->level);
  const char *ipv4_icon = get_ipv4_icon();

  printf("%s %s", iface_icon, wire->name);
  {
    const char *ssid = wire->ssid[0] ? wire->ssid : "noname";
    printf("  %s", ssid);
  }

  if (OUTPUT_SHOW_SIGNAL_LEVEL) {
    printf("  %s", signal_icon);
  }
  if (OUTPUT_SHOW_LINK_QUALITY) {
    printf("  %s", link_icon);
  }
  if (OUTPUT_SHOW_NOISE_LEVEL) {
    printf("  %s", level_icon);
  }

  if (OUTPUT_SHOW_IPV4 && net->addresses_ipv4) {
    for (size_t j = 0; j < net->addresses_ipv4->size; j++) {
      printf("  %s %s", ipv4_icon, net->addresses_ipv4->data[j].address);
    }
  }
  if (OUTPUT_SHOW_IPV6 && net->addresses_ipv6) {
    for (size_t j = 0; j < net->addresses_ipv6->size; j++) {
      const IPv6 *ip = &net->addresses_ipv6->data[j];
      if (!should_show_ipv6_type(ip->type) ||
          !should_show_ipv6_scope(ip->scope_id))
        continue;
      const char *icon = get_ipv6_icon(ip);
      printf("  %s %s", icon, ip->address);
    }
  }
  printf("\n");
}

static void __attribute__((unused))
terminal_print_detailed_network(const Network *net) {
  if (!net || !net->wire)
    return;
  const Wireless *wire = net->wire;
  const char *signal_icon = get_signal_icon(wire->level);
  const char *ssid = wire->ssid[0] ? wire->ssid : "noname";
  const char *status = get_interface_status_str(wire);

  printf("%s %s (%s)\n", signal_icon, ssid, wire->name);

  if (OUTPUT_SHOW_INTERFACE_STATUS) {
    printf("  %s", status);
  }
  if (OUTPUT_SHOW_SIGNAL_LEVEL) {
    printf("  level %.1f dBm", wire->level);
  }
  if (OUTPUT_SHOW_LINK_QUALITY) {
    printf("  link %.0f%%", wire->link);
  }
  if (OUTPUT_SHOW_NOISE_LEVEL) {
    if (is_noise_available(wire->noise)) {
      printf("  noise %.1fdBm", wire->noise);
    } else {
      printf("  noise n/a");
    }
  }
  printf("\n");

  int header_printed = 0;
  if (OUTPUT_SHOW_IPV4 && net->addresses_ipv4 &&
      net->addresses_ipv4->size > 0) {
    printf("  ip-addresses:\n    ipv4:\n");
    header_printed = 1;
    const char *ipv4_icon = get_ipv4_icon();
    for (size_t j = 0; j < net->addresses_ipv4->size; j++) {
      printf("      %s %s\n", ipv4_icon, net->addresses_ipv4->data[j].address);
    }
  }

  if (OUTPUT_SHOW_IPV6 && net->addresses_ipv6 &&
      net->addresses_ipv6->size > 0) {
    int has_visible = 0;
    for (size_t j = 0; j < net->addresses_ipv6->size; j++) {
      if (should_show_ipv6_type(net->addresses_ipv6->data[j].type) &&
          should_show_ipv6_scope(net->addresses_ipv6->data[j].scope_id)) {
        has_visible = 1;
        break;
      }
    }
    if (has_visible) {
      if (!header_printed) {
        printf("  ip-addresses:\n");
        header_printed = 1;
      }
      printf("    ipv6:\n");
      for (size_t j = 0; j < net->addresses_ipv6->size; j++) {
        const IPv6 *ip = &net->addresses_ipv6->data[j];
        if (!should_show_ipv6_type(ip->type) ||
            !should_show_ipv6_scope(ip->scope_id))
          continue;
        const char *icon = get_ipv6_icon(ip);
        if (ip->scope_id > 0) {
          printf("      %s %s (%s, scope %u)\n", icon, ip->address, ip->type,
                 ip->scope_id);
        } else {
          printf("      %s %s (%s)\n", icon, ip->address, ip->type);
        }
      }
    }
  }
}

static void short_print_network(const Network *net) {
  if (!net || !net->wire)
    return;
  const char *icon = get_signal_icon(net->wire->level);
  const char *ssid = net->wire->ssid[0] ? net->wire->ssid : "noname";
  printf("%s %s\n", icon, ssid);
}

void vec_networks_short_output(VecNetwork *networks) {
  if (!networks || networks->size == 0)
    return;
  for (size_t i = 0; i < networks->size; i++) {
    short_print_network(&networks->data[i]);
  }
}

void vec_networks_terminal_output(VecNetwork *networks) {
  if (!networks || networks->size == 0) {
    return;
  }

#if TERMINAL_STYLE == 1
  int width = calculate_max_width(networks);
  for (size_t i = 0; i < networks->size; i++) {
    terminal_print_verbose_network(&networks->data[i], width);
    if (i + 1 < networks->size)
      printf("\n");
  }
#elif TERMINAL_STYLE == 0
  for (size_t i = 0; i < networks->size; i++) {
    terminal_print_compact_network(&networks->data[i]);
  }
#elif TERMINAL_STYLE == 2
  for (size_t i = 0; i < networks->size; i++) {
    terminal_print_minimal_network(&networks->data[i]);
  }
#elif TERMINAL_STYLE == 3
  for (size_t i = 0; i < networks->size; i++) {
    terminal_print_detailed_network(&networks->data[i]);
    if (i + 1 < networks->size)
      printf("\n");
  }
#else
  for (size_t i = 0; i < networks->size; i++) {
    terminal_print_compact_network(&networks->data[i]);
  }
#endif
}
