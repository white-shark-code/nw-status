#pragma once

#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>

/* ===== Interface icons ===== */
typedef struct {
  const char *iface;
  const char *icon;
  const char *color; /* Optional: hex color "#ff0000" or NULL */
} interface_icon_t;

/* Default interface icon for all interfaces */
#define DEFAULT_INTERFACE_ICON "󰖩"
#define DEFAULT_INTERFACE_ICON_COLOR "#ffffff"

/* Per-interface icon overrides (uncomment and customize):
 * #if 0
 * static const interface_icon_t interface_icons[] = {
 *     { "wlan0", "󰖩", "#00ffff" },
 *     { "eth0",  "󰈀", "#00ff00" },
 * };
 * #endif
 */

/* Number of custom interface icons (0 = use default for all) */
#define INTERFACE_ICON_COUNT 0

/* ===== Link quality icons ===== */
typedef struct {
  int threshold;     /* Link quality threshold (0-100). Higher = better.
                        Array ordered from worst to best (ascending thresholds). */
  const char *icon;  /* Nerd Font icon */
  const char *color; /* Optional: hex color or NULL */
} link_icon_t;

/* Default link icon */
#define DEFAULT_LINK_ICON "󰇧"
#define DEFAULT_LINK_ICON_COLOR "#ffffff"

/* Per-link quality icons (uncomment and customize):
 * #if 0
 * static const link_icon_t link_icons[] = {
 *     { 70, "󰇧", "#00ff00" },  // good
 *     { 40, "󰇨", "#ffff00" },  // medium
 *     { 0,  "󰇦", "#ff0000" },  // poor
 * };
 * #endif
 */

/* Number of custom link icons (0 = use default for all) */
#define LINK_ICON_COUNT 0

/* ===== Signal level icons ===== */
typedef struct {
  float
      threshold;     /* Signal level threshold in dBm. Higher = better.
                        Array ordered from worst to best (ascending thresholds). */
  const char *icon;  /* Nerd Font icon */
  const char *color; /* Optional: hex color or NULL */
} level_icon_t;

/* Default level icon */
#define DEFAULT_LEVEL_ICON "󰤟"
#define DEFAULT_LEVEL_ICON_COLOR "#ffffff"

/* Per-level icons (uncomment and customize):
 * #if 0
 * static const level_icon_t level_icons[] = {
 *     { -80.0f, "󰤯", "#ff0000" },
 *     { -60.0f, "󰤟", "#ffff00" },
 *     { -40.0f, "󰤢", "#00ff00" },
 *     { -30.0f, "󰤨", "#00ff00" },
 * };
 * #endif
 */

/* Number of custom level icons (0 = use default for all) */
#define LEVEL_ICON_COUNT 0

/* ===== Signal quality levels ===== */
typedef struct {
  float
      threshold;     /* Noise threshold in dBm (float). Lower = better.
                        Array ordered from worst to best (ascending thresholds). */
  const char *label; /* Text label: "LOW", "MEDIUM", "HIGH", etc. */
  const char *icon;  /* Nerd Font icon: "󰤯", "󰤟", "󰤨" */
  const char *color; /* Optional: hex color "#ff0000" or NULL */
} signal_level_t;

/* Signal levels - worst to best */
static const signal_level_t signal_levels[] = {
    {-70.0f, "LOW", "󰤯", "#ff0000"},
    {-50.0f, "MEDIUM", "󰤟", "#ffff00"},
    {-30.0f, "HIGH", "󰤨", "#00ff00"},
};

#define SIGNAL_LEVEL_COUNT (sizeof(signal_levels) / sizeof(signal_levels[0]))

/* ===== JSON Output Configuration ===== */
#define JSON_OUTPUT_ICONS 0   /* 1 = include "icon" fields in JSON */
#define JSON_OUTPUT_COMPACT 0 /* 1 = compact, 0 = pretty printed */

/* ===== Field Visibility Flags ===== */
#define OUTPUT_SHOW_INTERFACE_NAME 1
#define OUTPUT_SHOW_INTERFACE_STATUS 1
#define OUTPUT_SHOW_LINK_QUALITY 1
#define OUTPUT_SHOW_SIGNAL_LEVEL 1
#define OUTPUT_SHOW_NOISE_LEVEL 1
#define OUTPUT_SHOW_IPV4 1
#define OUTPUT_SHOW_IPV6 1
#ifndef OUTPUT_SHOW_SSID
#define OUTPUT_SHOW_SSID 1
#endif
#define OUTPUT_IPV6_TYPE_MULTICAST 1
#define OUTPUT_IPV6_TYPE_LINK_LOCAL 1
#define OUTPUT_IPV6_TYPE_ULA 1
#define OUTPUT_IPV6_TYPE_GLOBAL 1
#define OUTPUT_IPV6_MAX_SCOPE_ID 0

/* ===== Terminal Output Configuration ===== */
#define OUTPUT_FORMAT_TERMINAL 1
#define TERMINAL_STYLE 3 /* 0=compact, 1=verbose, 2=minimal, 3=detailed */
#define TERMINAL_USE_ASCII_BOXES 1

/* ===== IP Version Icons ===== */
#define DEFAULT_IPV4_ICON "󰈀"
#define DEFAULT_IPV4_ICON_COLOR "#00ffff"
#define DEFAULT_IPV6_ICON "󰈁"
#define DEFAULT_IPV6_ICON_COLOR "#ff8800"
#define IPV6_ICON_MULTICAST "󰍜"
#define IPV6_ICON_LINK_LOCAL "󰣇"
#define IPV6_ICON_ULA "󰖩"
#define IPV6_ICON_GLOBAL "󰀄"

#endif
