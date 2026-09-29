# nw-status

Network status utility for Linux. Outputs wireless interface information and IP addresses in JSON or terminal format.

## Features

- Reads wireless interface data from `/proc/net/wireless`
- Retrieves IPv4 and IPv6 addresses via `getifaddrs()`
- Classifies IPv6 addresses: global, link-local, multicast, ULA
- **Two output formats**: JSON (default) and terminal (4 styles)
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
| `TERMINAL_STYLE` | `0`=compact, `1`=verbose, `2`=minimal, `3`=detailed (default) | Terminal style |
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
| `OUTPUT_SHOW_NOISE_LEVEL` | Noise level (dBm, `n/a`/`null` if unavailable) |
| `OUTPUT_SHOW_IPV4` | IPv4 addresses |
| `OUTPUT_SHOW_IPV6` | IPv6 addresses |
| `OUTPUT_SHOW_SSID` | SSID (wireless network name, JSON `ssid` field) |

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

// Signal quality levels, worst to best (first entry with level < threshold wins)
static const signal_level_t signal_levels[] = {
    { -80.0f, "CRITICAL",  "󰤮", "#ff0000" },
    { -70.0f, "LOW",       "󰤯", "#ff5500" },
    { -60.0f, "FAIR",      "󰤟", "#ffff00" },
    { -50.0f, "GOOD",      "󰤢", "#aaff00" },
    { -30.0f, "EXCELLENT", "󰤨", "#00ff00" },
};
```

### Per-Interface/Link/Level Tables (defaults in config.def.h, edit to customize)

```c
// Interface-specific icons (exact name match)
static const interface_icon_t interface_icons[] = {
    { "wlan0", "󰖩", "#00ffff" },
    { "wlan1", "󰖩", "#00ffff" },
    { "eth0",  "󰈀", "#00ff00" },
};
#define INTERFACE_ICON_COUNT 3

// Link quality thresholds, best to worst (first entry with link >= threshold wins)
static const link_icon_t link_icons[] = {
    { 70, "󰇧", "#00ff00" },  // good
    { 40, "󰇨", "#ffff00" },  // medium
    { 0,  "󰇦", "#ff0000" },  // poor
};
#define LINK_ICON_COUNT 3

// Signal level thresholds, best to worst (first entry with level >= threshold wins)
static const level_icon_t level_icons[] = {
    { -30.0f, "󰤨", "#00ff00" },  // excellent
    { -50.0f, "󰤢", "#aaff00" },  // good
    { -65.0f, "󰤟", "#ffff00" },  // fair
    { -80.0f, "󰤯", "#ff5500" },  // weak
};
#define LEVEL_ICON_COUNT 4
```

## Usage

```bash
# Default output (format/style from config.h: OUTPUT_FORMAT_TERMINAL, TERMINAL_STYLE)
./build/nw-status

# Short output: `<signal icon> <ssid>` (overrides OUTPUT_FORMAT_TERMINAL)
./build/nw-status --short
./build/nw-status -s

# Show help
./build/nw-status --help
./build/nw-status -h
```

Short mode prints `<signal icon> <ssid>` per interface (`noname` if the SSID is empty or unavailable).
An unknown flag prints an error to stderr and exits with status `1`.
The default format/style are controlled by `config.h` (`OUTPUT_FORMAT_TERMINAL`, `TERMINAL_STYLE`); `--short` overrides the format selector.

## Output Examples

### JSON (default)

```json
{
    "networks": [
        {
            "interface": "wlan0",
            "ssid": "MyNetwork",
            "signal": {
                "quality": "EXCELLENT",
                "level_dbm": -35.0,
                "link": 70.0,
                "noise_dbm": null
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
    "ssid": "MyNetwork",
    "interface_icon": "󰖩",
    "interface_icon_color": "#ffffff",
    "signal": {
        "quality": "EXCELLENT",
        "icon": "󰤨",
        "link_icon": "󰇧",
        "link_icon_color": "#ffffff",
        "level_icon": "󰤟",
        "level_icon_color": "#ffffff",
        "level_dbm": -35.0,
        "link": 70.0,
        "noise_dbm": null
    }
}
```

### Terminal verbose (`OUTPUT_FORMAT_TERMINAL=1`, `TERMINAL_STYLE=1`)

```
  +------------------- 󰖩 MyNetwork (wlan0, UP) -------------------+
  | 󰤨 EXCELLENT 󰇧 Link: 70%  󰤢 Noise: n/a                      |
  | 󰤢 Level:  -35.0 dBm                                        |
  +--------------------------- IPv4 ---------------------------+
  | 󰈀  192.168.1.4                                             |
  +--------------------------- IPv6 ---------------------------+
  | 󰀄  2001:ee0:5493:60f0::/64  global                          |
  | 󰣇  fe80::a127:2e9e:5071:fd10  link-local (3)               |
  +------------------------------------------------------------+
```

### Terminal compact (`TERMINAL_STYLE=0`)

```
  󰖩 wlan0 (UP)  MyNetwork  󰤨 -35.0dBm  󰇧70%  󰤢 n/a  󰈀 192.168.1.4  󰀄 2001:ee0::/64  󰣇 fe80::/64
```

### Terminal minimal (`TERMINAL_STYLE=2`)

```
  󰖩 wlan0  MyNetwork  󰤨  󰇧  󰤢  󰈀 192.168.1.4  󰀄 2001:ee0::/64  󰣇 fe80::/64
```

### Terminal detailed (`TERMINAL_STYLE=3`, default)

Line-by-line output without boxes (SSID is shown in all terminal modes).
Respects `OUTPUT_SHOW_*` and the IPv6 filters.

```
󰤨 MyNetwork (wlan0)
  UP  level -35.0 dBm  link 70%  noise n/a
  ip-addresses:
    ipv4:
      󰈀 192.168.1.4
    ipv6:
      󰀄 2001:ee0:5493:60f0::/64 (global)
      󰣇 fe80::a127:2e9e:5071:fd10 (link-local, scope 3)
```

### Short output (`--short` / `-s`)

Prints `<signal icon> <ssid>` per interface, overriding `OUTPUT_FORMAT_TERMINAL`.
Empty or unavailable SSID is shown as `noname`.

```
󰤨 MyNetwork
```

## JSON Fields

| Field | Description |
|-------|-------------|
| `interface` | Wireless interface name |
| `ssid` | Wireless network name (via `ioctl(SIOCGIWESSID)`; `noname` if empty/unavailable; hidden if `OUTPUT_SHOW_SSID=0`; quotes escaped in JSON) |
| `interface_icon` | Nerd Font icon (if `JSON_OUTPUT_ICONS=1`) |
| `interface_icon_color` | Hex color (if `JSON_OUTPUT_ICONS=1`) |
| `signal.quality` | Quality label: CRITICAL/LOW/FAIR/GOOD/EXCELLENT |
| `signal.icon` | Quality icon (if `JSON_OUTPUT_ICONS=1`) |
| `signal.link_icon` | Link quality icon (if `JSON_OUTPUT_ICONS=1`) |
| `signal.link_icon_color` | Link icon color (if `JSON_OUTPUT_ICONS=1`) |
| `signal.level_icon` | Signal level icon (if `JSON_OUTPUT_ICONS=1`) |
| `signal.level_icon_color` | Level icon color (if `JSON_OUTPUT_ICONS=1`) |
| `signal.level_dbm` | Signal level in dBm |
| `signal.link` | Link quality (0-100) |
| `signal.noise_dbm` | Noise level in dBm, `null` if the driver doesn't report it |
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