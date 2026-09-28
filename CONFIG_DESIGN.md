# nw-status Configuration Design Document

## Overview
Suckless-style configuration with `config.def.h` (defaults, in repo) and `config.h` (user overrides, gitignored).

## Configuration Structure

### config.def.h (in repository)
```c
#pragma once

#ifndef CONFIG_H
#define CONFIG_H

#include <stddef.h>

/* Signal quality levels - fully configurable */
typedef struct {
    float threshold;       /* Noise threshold in dBm (float). Lower = better.
                              Array ordered from worst to best (ascending thresholds). */
    const char *label;     /* Text label: "LOW", "MEDIUM", "HIGH", etc. */
    const char *icon;      /* Nerd Font icon: "󰤯", "󰤟", "󰤨" */
    const char *color;     /* Optional: hex color "#ff0000" or NULL */
} signal_level_t;

/* Default signal levels - worst to best */
static const signal_level_t signal_levels[] = {
    { -70.0f, "LOW",    "󰤯", "#ff0000" },
    { -50.0f, "MEDIUM", "󰤟", "#ffff00" },
    { -30.0f, "HIGH",   "󰤨", "#00ff00" },
};

#define SIGNAL_LEVEL_COUNT (sizeof(signal_levels) / sizeof(signal_levels[0]))

/* JSON Output Configuration */
#ifndef JSON_OUTPUT_ICONS
#define JSON_OUTPUT_ICONS 0  /* 1 = include "icon" field in JSON */
#endif

#ifndef JSON_OUTPUT_COMPACT
#define JSON_OUTPUT_COMPACT 0  /* 1 = compact, 0 = pretty printed */
#endif

/* Future extensibility examples (commented)
#define ENABLE_LINK_QUALITY 1
static const config_kv_t custom_fields[] = {
    { "location", "home" },
    { "owner", "admin" },
};
*/

#endif
```

### config.h (user-created, gitignored)
```c
/* Example custom configuration */
static const signal_level_t signal_levels[] = {
    { -80.0f, "TERRIBLE", "󰤮", "#ff0000" },
    { -65.0f, "WEAK",     "󰤯", "#ff5500" },
    { -50.0f, "FAIR",     "󰤟", "#ffff00" },
    { -40.0f, "GOOD",     "󰤢", "#00ff00" },
    { -30.0f, "EXCELLENT","󰤨", "#00aa00" },
};

#define JSON_OUTPUT_ICONS 1
#define JSON_OUTPUT_COMPACT 0
```

## JSON Output Format

### Without icons (JSON_OUTPUT_ICONS 0)
```json
{
  "networks": [
    {
      "interface": "wlan0",
      "signal": {
        "quality": "HIGH",
        "level_dbm": -39.0,
        "link": 70.0,
        "noise_dbm": -256.0
      },
      "ip": {
        "ipv4": ["192.168.1.4"],
        "ipv6": [
          {"address": "2001:ee0:...", "type": "global", "scope_id": 0},
          {"address": "fe80::...", "type": "link-local", "scope_id": 3}
        ]
      }
    }
  ]
}
```

### With icons (JSON_OUTPUT_ICONS 1)
```json
{
  "networks": [
    {
      "interface": "wlan0",
      "signal": {
        "quality": "HIGH",
        "icon": "󰤨",
        "level_dbm": -39.0,
        "link": 70.0,
        "noise_dbm": -256.0
      },
      "ip": {
        "ipv4": ["192.168.1.4"],
        "ipv6": [
          {"address": "2001:ee0:...", "type": "global", "scope_id": 0},
          {"address": "fe80::...", "type": "link-local", "scope_id": 3}
        ]
      }
    }
  ]
}
```

## Signal Classification Logic

```c
const char *get_signal_quality(float noise) {
    for (size_t i = 0; i < SIGNAL_LEVEL_COUNT; i++) {
        if (noise < signal_levels[i].threshold) {
            return signal_levels[i].label;
        }
    }
    return signal_levels[SIGNAL_LEVEL_COUNT - 1].label; /* best level */
}

const char *get_signal_icon(float noise) {
    for (size_t i = 0; i < SIGNAL_LEVEL_COUNT; i++) {
        if (noise < signal_levels[i].threshold) {
            return signal_levels[i].icon;
        }
    }
    return signal_levels[SIGNAL_LEVEL_COUNT - 1].icon;
}

const char *get_signal_color(float noise) {
    for (size_t i = 0; i < SIGNAL_LEVEL_COUNT; i++) {
        if (noise < signal_levels[i].threshold) {
            return signal_levels[i].color; /* may be NULL */
        }
    }
    return signal_levels[SIGNAL_LEVEL_COUNT - 1].color;
}
```

## Implementation Files

| File | Purpose |
|------|---------|
| `config.def.h` | Default configuration (in repo) |
| `config.h` | User config (gitignored) |
| `.gitignore` | Add `config.h` |
| `Makefile` | Include config logic (`-include`) |
| `utils.h` | Prototypes for `get_signal_*` functions |
| `utils.c` | Implementation + new JSON output |

## Makefile Config Logic
```makefile
ifeq ("$(wildcard config.h)","")
    CFLAGS += -include config.def.h
else
    CFLAGS += -include config.h
endif
```

## Color Field
- Optional in struct (`const char *color` can be NULL)
- Not output to JSON (per requirements)
- Available for terminal UI usage

## JSON Output Fields
- `quality` (label) - always present
- `icon` - only if `JSON_OUTPUT_ICONS = 1`
- `color` - never in JSON (optional in struct for terminal use)
- `level_dbm`, `link`, `noise_dbm` - raw values
- IPv4/IPv6 arrays unchanged

## Future Extensibility
```c
typedef struct {
    const char *key;
    const char *value;
} config_kv_t;

static const config_kv_t custom_fields[] = {
    { "location", "home" },
    { "owner", "admin" },
};
```