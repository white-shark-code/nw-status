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

/* Per-interface icons (exact name match, edit to customize) */
static const interface_icon_t interface_icons[] = {
    {"wlan0", "󰖩", "#00ffff"},
    {"wlan1", "󰖩", "#00ffff"},
    {"eth0", "󰈀", "#00ff00"},
};

/* Number of per-interface icons (keep in sync with the table above) */
#define INTERFACE_ICON_COUNT 3

/* ===== Link quality icons ===== */
typedef struct {
  int threshold;     /* Link quality threshold (0-100). Higher = better.
                        Array ordered from best to worst (descending thresholds):
                        first entry with link >= threshold wins. */
  const char *icon;  /* Nerd Font icon */
  const char *color; /* Optional: hex color or NULL */
} link_icon_t;

/* Default link icon */
#define DEFAULT_LINK_ICON "󰇧"
#define DEFAULT_LINK_ICON_COLOR "#ffffff"

/* Link quality icons by threshold */
static const link_icon_t link_icons[] = {
    {70, "󰇧", "#00ff00"}, /* good */
    {40, "󰇨", "#ffff00"}, /* medium */
    {0, "󰇦", "#ff0000"},  /* poor */
};

/* Number of link quality icons (keep in sync with the table above) */
#define LINK_ICON_COUNT 3

/* ===== Signal level icons ===== */
typedef struct {
  float
      threshold;     /* Signal level threshold in dBm. Higher = better.
                        Array ordered from best to worst (descending thresholds):
                        first entry with level >= threshold wins. */
  const char *icon;  /* Nerd Font icon */
  const char *color; /* Optional: hex color or NULL */
} level_icon_t;

/* Default level icon */
#define DEFAULT_LEVEL_ICON "󰤟"
#define DEFAULT_LEVEL_ICON_COLOR "#ffffff"

/* Signal level icons by threshold */
static const level_icon_t level_icons[] = {
    {-30.0f, "󰤨", "#00ff00"}, /* excellent */
    {-50.0f, "󰤢", "#aaff00"}, /* good */
    {-65.0f, "󰤟", "#ffff00"}, /* fair */
    {-80.0f, "󰤯", "#ff5500"}, /* weak */
};

/* Number of signal level icons (keep in sync with the table above) */
#define LEVEL_ICON_COUNT 4

/* ===== Signal quality levels ===== */
typedef struct {
  float
      threshold;     /* Signal level threshold in dBm (float). Higher = better.
                        Array ordered from worst to best (ascending thresholds). */
  const char *label; /* Text label: "LOW", "MEDIUM", "HIGH", etc. */
  const char *icon;  /* Nerd Font icon: "󰤯", "󰤟", "󰤨" */
  const char *color; /* Optional: hex color "#ff0000" or NULL */
} signal_level_t;

/* Signal levels - worst to best (ascending thresholds:
   first entry with level < threshold wins) */
static const signal_level_t signal_levels[] = {
    {-80.0f, "CRITICAL", "󰤮", "#ff0000"},
    {-70.0f, "LOW", "󰤯", "#ff5500"},
    {-60.0f, "FAIR", "󰤟", "#ffff00"},
    {-50.0f, "GOOD", "󰤢", "#aaff00"},
    {-30.0f, "EXCELLENT", "󰤨", "#00ff00"},
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
