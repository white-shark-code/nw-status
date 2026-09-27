# nw-status

Network status utility for Linux. Outputs wireless interface information and IP addresses in JSON format.

## Features

- Reads wireless interface data from `/proc/net/wireless`
- Retrieves IPv4 and IPv6 addresses via `getifaddrs()`
- Classifies IPv6 addresses: global, link-local, multicast, ULA
- Outputs structured JSON
- No external dependencies (standard C library only)

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

## Usage

```bash
# Run production build
./build/nw-status

# Run debug build
./build/nw-status-debug

# Run release build
./build/nw-status-release
```

## Output Example

```json
{
	"networks": [
		{
			"interface": "wlan0",
			"status": "0000",
			"level": -39.0,
			"link": 70.0,
			"noise": -256.0,
			 "ip-addresses": {
				"IPv4": [
					"192.168.1.4"
				],
				"IPv6": [
					{
						"address": "2001:ee0:5493:60f0:8191:933f:8e80:282c",
						"type": "global",
						"scope_id": 0
					},
					{
						"address": "fe80::a127:2e9e:5071:fd10",
						"type": "link-local",
						"scope_id": 3
					}
				]
			}
		}
	]
}
```

## Fields

| Field | Description |
|-------|-------------|
| `interface` | Wireless interface name (e.g., `wlan0`) |
| `status` | Interface status flags from `/proc/net/wireless` |
| `link` | Link quality (0-100) |
| `level` | Signal level (dBm) |
| `noise` | Noise level (dBm) |
| `ip-addresses.IPv4[]` | List of IPv4 addresses |
| `ip-addresses.IPv6[].address` | IPv6 address |
| `ip-addresses.IPv6[].type` | Address type: `global`, `link-local`, `multicast`, `ULA` |
| `ip-addresses.IPv6[].scope_id` | Interface scope ID for link-local addresses |

## Requirements

- Linux kernel (reads `/proc/net/wireless`)
- GCC or compatible C compiler
- Standard C library (glibc, musl, etc.)

## License

MIT