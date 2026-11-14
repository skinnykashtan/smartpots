# SmartPots Gateway

SmartPots Gateway is the network-facing component of the SmartPots system. It runs on an ESP32 microcontroller, is built with the ESP-IDF framework and written in C++. The gateway connects to a local Wi-Fi network in station (STA) mode, providing the network link for the rest of the system.

## Features

- Wi-Fi connection in station mode with WPA2-PSK security
- Automatic reconnection with a configurable retry limit
- Network credentials configurable through `menuconfig`, with no changes to the source code
- Event-driven connection handling based on the ESP-IDF event loop and FreeRTOS event groups
- Automatic recovery of the NVS partition on startup

## Project Structure

```
main/
├── CMakeLists.txt          # Component registration and source list
├── idf_component.yml       # Component Manager dependencies
├── Kconfig.projbuild       # "SmartPots configuration" menu for menuconfig
├── main.cpp                # Application entry point (app_main)
├── wifi_connection.h       # Wi-Fi module public interface
└── wifi_connection.cpp     # Wi-Fi station mode implementation
```

## Architecture

### Application Entry Point – `main.cpp`

`app_main()` performs the startup sequence:

1. **NVS initialization** - initializes non-volatile storage, which the Wi-Fi driver requires. If the partition has no free pages or uses an outdated format version, it is erased and initialized again.
2. **Network connection** - calls `wifi::wifi_init_sta()` to connect the device to the configured access point.

### Wi-Fi Module – `wifi_connection.h` / `wifi_connection.cpp`

The module lives in the `wifi` namespace. Its internal state and the event handler are kept in an anonymous namespace, so they are not visible to other translation units.

#### Public API

```cpp
namespace wifi {
    bool wifi_init_sta();
}
```

| Return value | Meaning |
| --- | --- |
| `true` | The device connected to the access point and obtained an IP address |
| `false` | The connection failed after the maximum number of retries |

The call is **blocking**: it returns only once the connection either succeeds or definitively fails.

#### Initialization Sequence

1. Creates a FreeRTOS event group that signals the connection result through two bits: `WIFI_CONNECTED_BIT` and `WIFI_FAIL_BIT`.
2. Initializes the TCP/IP stack (`esp_netif_init`), the default event loop and the default station network interface.
3. Initializes the Wi-Fi driver with the default configuration (`WIFI_INIT_CONFIG_DEFAULT`).
4. Registers the event handler for all `WIFI_EVENT` events and for `IP_EVENT_STA_GOT_IP`.
5. Applies the configured SSID and password and sets WPA2-PSK as the minimum accepted security mode.
6. Switches the driver to station mode and starts it.
7. Waits until the connection succeeds or fails.

#### Event Handling

| Event | Action |
| --- | --- |
| `WIFI_EVENT_STA_START` | Starts connecting to the access point |
| `WIFI_EVENT_STA_DISCONNECTED` | Retries the connection until the retry limit is reached, then sets `WIFI_FAIL_BIT` |
| `IP_EVENT_STA_GOT_IP` | Logs the assigned IP address and netmask, resets the retry counter and sets `WIFI_CONNECTED_BIT` |

## Configuration

Network settings are defined in `main/Kconfig.projbuild` and set with `idf.py menuconfig` under **SmartPots configuration**:

| Option | Kconfig symbol | Default | Description |
| --- | --- | --- | --- |
| WiFi SSID | `CONFIG_ESP_WIFI_SSID` | `""` | Name of the Wi-Fi network |
| WiFi Password | `CONFIG_ESP_WIFI_PASSWORD` | `""` | Password of the Wi-Fi network |
| Maximum retry | `CONFIG_ESP_MAXIMUM_RETRY` | `5` | Number of reconnection attempts before giving up |

> **Note:** The values are stored in plain text in `sdkconfig`. Do not publish this file with real credentials.

## Requirements

- ESP32 development board
- ESP-IDF v5.3 or newer
- USB cable for flashing and power supply

## Build and Flash

```bash
idf.py set-target esp32
idf.py menuconfig
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Replace `/dev/ttyUSB0` with the serial port of your board.

## Logging

| Tag | Source |
| --- | --- |
| `MAIN` | `main.cpp` |
| `WIFI` | `wifi_connection.cpp` |

Example output after a successful connection:

```
I (...) MAIN: ESP_WIFI_MODE_STA
I (...) WIFI: wifi_init_sta finished
I (...) WIFI: got ip:192.168.1.42 | Mask:255.255.255.0
I (...) WIFI: connected to AP SSID: <your-ssid>
```
