# nw-status

Network status utility for Linux. Outputs wireless interface information and IP addresses in JSON or terminal format.

## Features

- Reads wireless interface data from `/proc/net/wireless`
- Retrieves IPv4 and IPv6 addresses via `getifaddrs()`
- Classifies IPv6 addresses: global, link-local, multicast, ULA
- **Two output formats**: JSON (default) and terminal (3 styles)
- **Suckless-style configuration**: `config.def.h` (defaults) + `config.h` (user, gitignored)
- **Field visibility control**: toggle any field via config
- **Nerd Font icons** with per-interface/link/level colors
- No external dependencies (standard C library only)
- Security hardened: ASan clean, zero warnings, integer overflow protection

## Build

```bash
# Production build (optimized for current CPU)
make

# Portable release build (runs on any x86_64)
make release

# Debug build (with AddressSanitizer and UBSan)
make debug

# Clean build artifacts
make clean
```

## Install

```bash
# Install production build
sudo make install

# Install release build
sudo make install-release

# Custom prefix
sudo make install PREFIX=/opt/local

# For packaging (DESTDIR staging)
make install DESTDIR=/tmp/pkg PREFIX=/usr
```

## Uninstall

```bash
sudo make uninstall
# or with custom prefix
sudo make uninstall PREFIX=/opt/local
```

## Configuration

Copy `config.def.h` to `config.h` and edit:

```bash
cp config.def.h config.h
$EDITOR config.h
```

### Output Format

| Macro | Values | Description |
|-------|--------|-------------|
| `OUTPUT_FORMAT_TERMINAL` | `0` (JSON), `1` (terminal) | Output format selector |
| `TERMINAL_STYLE` | `0`=compact, `1`=verbose, `2`=minimal | Terminal style |
| `TERMINAL_USE_ASCII_BOXES` | `1` | Use ASCII box drawing |

### JSON Options

| Macro | Values | Description |
|-------|--------|-------------|
| `JSON_OUTPUT_ICONS` | `0`/`1` | Include icon/color fields in JSON |
| `JSON_OUTPUT_COMPACT` | `0`/`1` | Compact vs pretty-printed JSON |

### Field Visibility (all `0`/`1`)

| Macro | Controls |
|-------|----------|
| `OUTPUT_SHOW_INTERFACE_NAME` | Interface name |
| `OUTPUT_SHOW_INTERFACE_STATUS` | UP/DOWN status |
| `OUTPUT_SHOW_LINK_QUALITY` | Link quality % |
| `OUTPUT_SHOW_SIGNAL_LEVEL` | Signal level (dBm) |
| `OUTPUT_SHOW_NOISE_LEVEL` | Noise level (dBm) |
| `OUTPUT_SHOW_IPV4` | IPv4 addresses |
| `OUTPUT_SHOW_IPV6` | IPv6 addresses |

### IPv6 Filters

| Macro | Values | Description |
|-------|--------|-------------|
| `OUTPUT_IPV6_TYPE_MULTICAST` | `0`/`1` | Show multicast |
| `OUTPUT_IPV6_TYPE_LINK_LOCAL` | `0`/`1` | Show link-local |
| `OUTPUT_IPV6_TYPE_ULA` | `0`/`1` | Show ULA |
| `OUTPUT_IPV6_TYPE_GLOBAL` | `0`/`1` | Show global |
| `OUTPUT_IPV6_MAX_SCOPE_ID` | `0`=all, `N` | Max scope_id to show |

### Icons & Colors (all configurable)

```c
// Interface icons
#define DEFAULT_INTERFACE_ICON "󰖩"
#define DEFAULT_INTERFACE_ICON_COLOR "#ffffff"

// Link quality icons
#define DEFAULT_LINK_ICON "󰇧"
#define DEFAULT_LINK_ICON_COLOR "#ffffff"

// Signal level icons
#define DEFAULT_LEVEL_ICON "󰤟"
#define DEFAULT_LEVEL_ICON_COLOR "#ffffff"

// IPv4/IPv6 icons
#define DEFAULT_IPV4_ICON "󰈀"
#define DEFAULT_IPV4_ICON_COLOR "#00ffff"
#define DEFAULT_IPV6_ICON "󰈁"
#define DEFAULT_IPV6_ICON_COLOR "#ff8800"

// IPv6 type icons
#define IPV6_ICON_MULTICAST "󰍜"
#define IPV6_ICON_LINK_LOCAL "󰣇"
#define IPV6_ICON_ULA "󰖩"
#define IPV6_ICON_GLOBAL "󰀄"

// Signal quality levels (per-level icons/colors)
static const signal_level_t signal_levels[] = {
    { -70.0f, "LOW",    "󰤯", "#ff0000" },
    { -50.0f, "MEDIUM", "󰤟", "#ffff00" },
    { -30.0f, "HIGH",   "󰤨", "#00ff00" },
};
```

### Per-Interface/Link/Level Overrides (uncomment in config.h)

```c
// Interface-specific icons
static const interface_icon_t interface_icons[] = {
    { "wlan0", "󰖩", "#00ffff" },
    { "eth0",  "󰈀", "#00ff00" },
};
#define INTERFACE_ICON_COUNT 2

// Link quality thresholds
static const link_icon_t link_icons[] = {
    { 70, "󰇧", "#00ff00" },  // good
    { 40, "󰇨", "#ffff00" },  // medium
    { 0,  "󰇦", "#ff0000" },  // poor
};
#define LINK_ICON_COUNT 3

// Signal level thresholds
static const level_icon_t level_icons[] = {
    { -80.0f, "󰤯", "#ff0000" },
    { -60.0f, "󰤟", "#ffff00" },
    { -40.0f, "󰤢", "#00ff00" },
    { -30.0f, "󰤨", "#00ff00" },
};
#define LEVEL_ICON_COUNT 4
```

## Usage

```bash
# JSON output (default, uses config.h OUTPUT_FORMAT_TERMINAL=0)
./build/nw-status

# Terminal verbose (boxes, requires OUTPUT_FORMAT_TERMINAL=1, TERMINAL_STYLE=1)
./build/nw-status

# Terminal compact (one-liner)
./build/nw-status

# Terminal minimal (icons only)
./build/nw-status
```

Format/style controlled entirely by `config.h` — no CLI arguments.

## Output Examples

### JSON (default)

```json
{
    "networks": [
        {
            "interface": "wlan0",
            "signal": {
                "quality": "LOW",
                "level_dbm": -35.0,
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

### JSON with icons (`JSON_OUTPUT_ICONS=1`)

```json
{
    "interface": "wlan0",
    "interface_icon": "󰖩",
    "interface_icon_color": "#ffffff",
    "signal": {
        "quality": "LOW",
        "icon": "󰤯",
        "link_icon": "󰇧",
        "link_icon_color": "#ffffff",
        "level_icon": "󰤟",
        "level_icon_color": "#ffffff",
        "level_dbm": -35.0,
        "link": 70.0,
        "noise_dbm": -256.0
    }
}
```

### Terminal verbose (`OUTPUT_FORMAT_TERMINAL=1`, `TERMINAL_STYLE=1`)

```
  +----------------------- 󰖩 wlan0 (UP) -----------------------+
  | 󰤯 LOW 󰇧 Link: 70%  󰤟 Noise: -256dBm                        |
  | 󰤟 Level:  -35.0 dBm                                        |
  +--------------------------- IPv4 ---------------------------+
  | 󰈀  192.168.1.4                                             |
  +--------------------------- IPv6 ---------------------------+
  | 󰀄  2001:ee0:5493:60f0::/64  global                          |
  | 󰣇  fe80::a127:2e9e:5071:fd10  link-local (3)               |
  +------------------------------------------------------------+
```

### Terminal compact (`TERMINAL_STYLE=0`)

```
  󰖩 wlan0 (UP)  󰤯 -35.0dBm  󰇧70%  󰤟-256dBm  󰈀 192.168.1.4  󰀄 2001:ee0::/64  󰣇 fe80::/64
```

### Terminal minimal (`TERMINAL_STYLE=2`)

```
  󰖩 wlan0  󰤯  󰇧  󰤟  󰈀 192.168.1.4  󰀄 2001:ee0::/64  󰣇 fe80::/64
```

## JSON Fields

| Field | Description |
|-------|-------------|
| `interface` | Wireless interface name |
| `interface_icon` | Nerd Font icon (if `JSON_OUTPUT_ICONS=1`) |
| `interface_icon_color` | Hex color (if `JSON_OUTPUT_ICONS=1`) |
| `signal.quality` | Quality label: LOW/MEDIUM/HIGH |
| `signal.icon` | Quality icon (if `JSON_OUTPUT_ICONS=1`) |
| `signal.link_icon` | Link quality icon (if `JSON_OUTPUT_ICONS=1`) |
| `signal.link_icon_color` | Link icon color (if `JSON_OUTPUT_ICONS=1`) |
| `signal.level_icon` | Signal level icon (if `JSON_OUTPUT_ICONS=1`) |
| `signal.level_icon_color` | Level icon color (if `JSON_OUTPUT_ICONS=1`) |
| `signal.level_dbm` | Signal level in dBm |
| `signal.link` | Link quality (0-100) |
| `signal.noise_dbm` | Noise level in dBm |
| `ip.ipv4[]` | IPv4 addresses |
| `ip.ipv6[].address` | IPv6 address |
| `ip.ipv6[].type` | global/link-local/multicast/ULA |
| `ip.ipv6[].scope_id` | Interface scope ID |

## Requirements

- Linux kernel (reads `/proc/net/wireless`)
- GCC or compatible C compiler (C99)
- Standard C library (glibc, musl, etc.)
- Nerd Font for icon display

## License

MIT