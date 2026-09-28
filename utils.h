#ifndef __UTILS_H__
#define __UTILS_H__

#include "vec.h"
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STATIC_PATH_PROC_WIRELES "/proc/net/wireless"

#ifndef IN6_IS_ADDR_ULA
#define IN6_IS_ADDR_ULA(a) (((a)->s6_addr[0] & 0xfe) == 0xfc) /* RFC 4193 */
#endif

typedef struct {
  char name[64];
  char status[5];
  float link, level, noise;
} Wireless;

typedef struct {
  char address[INET_ADDRSTRLEN];
} IPv4;

typedef struct {
  char address[INET6_ADDRSTRLEN];
  char type[11];
  uint32_t scope_id;
} IPv6;

_Static_assert(sizeof("link-local") <= 11,
               "IPv6 type buffer too small for 'link-local'");
_Static_assert(sizeof("multicast") <= 11,
               "IPv6 type buffer too small for 'multicast'");
_Static_assert(sizeof("ULA") <= 11, "IPv6 type buffer too small for 'ULA'");
_Static_assert(sizeof("global") <= 11,
               "IPv6 type buffer too small for 'global'");

VECTOR_DECLARE(char, Char, char)
VECTOR_DECLARE(Wireless, Wirelesses, wirelesses)
VECTOR_DECLARE(IPv4, AddressesIPv4, addresses_ipv4)
VECTOR_DECLARE(IPv6, AddressesIPv6, addresses_ipv6)

typedef struct {
  Wireless *wire;
  VecAddressesIPv4 *addresses_ipv4;
  VecAddressesIPv6 *addresses_ipv6;
} Network;

typedef struct {
  Network *data;
  size_t capacity;
  size_t size;
} VecNetwork;

/* Функции */
VecChar *slice_char(const char *str, size_t len);
FILE *read_file(const char *path);
Wireless get_wire_iface(const char *row);
VecWirelesses *get_all_wire_ifaces(FILE *file);
int vec_networks_push(VecNetwork *networks, Network *network);
VecNetwork *prepair_networks_using_wireless(VecWirelesses *wirelesses);
ssize_t check_exist_wire(struct ifaddrs *ifa, VecNetwork *networks);
int get_addr(VecNetwork *networks);
void vec_networks_json_output(VecNetwork *networks);

/* Signal quality functions */
const char *get_signal_quality(float noise);
const char *get_signal_icon(float noise);
const char *get_signal_color(float noise);

/* Interface/Link/Level icons */
const char *get_interface_icon(const char *iface);
const char *get_interface_icon_color(const char *iface);
const char *get_link_icon(int link);
const char *get_link_icon_color(int link);
const char *get_level_icon(float level);
const char *get_level_icon_color(float level);

/* Конструкторы/деструкторы */
Network *network_init();
Network *network_init_with_capacity(size_t capacity);
void network_free(Network *network);
VecNetwork *vec_networks_init();
void vec_networks_free(VecNetwork *networks);

/* Terminal output */
void vec_networks_terminal_output(VecNetwork *networks);

/* Shared helpers for field filtering */
const char *get_interface_status_str(const Wireless *wire);
int should_show_ipv6_type(const char *type);
int should_show_ipv6_scope(uint32_t scope);

/* IPv4/IPv6 icons for terminal output */
const char *get_ipv4_icon(void);
const char *get_ipv4_icon_color(void);
const char *get_ipv6_icon(const IPv6 *ip);
const char *get_ipv6_icon_color(const IPv6 *ip);

#endif /* __UTILS_H__ */
