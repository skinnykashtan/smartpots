# SmartPots Gateway

SmartPots Gateway is the network-facing component of the SmartPots system. It runs on an ESP32 microcontroller, is built with the ESP-IDF framework and written in C++. The gateway connects to a local Wi-Fi network in station (STA) mode and forwards plant measurements to an MQTT broker.

## Features

- Wi-Fi connection in station mode with WPA2-PSK security
- Automatic reconnection with a configurable retry limit
- Network credentials configurable through `menuconfig`, with no changes to the source code
- Event-driven connection handling based on the ESP-IDF event loop and FreeRTOS event groups
- Automatic recovery of the NVS partition on startup
- MQTT client connected to a broker over TCP
- Queue-based publishing: measurements are buffered in a FreeRTOS queue and sent by a dedicated publisher task
- Publishing is paused while the broker is unreachable and resumes automatically after reconnection

## Project Structure

```
main/
├── CMakeLists.txt          # Component registration and source list
├── idf_component.yml       # Component Manager dependencies
├── Kconfig.projbuild       # "SmartPots configuration" menu for menuconfig
├── main.cpp                # Application entry point (app_main)
├── measurement.h           # Message structure shared between modules
├── mqtt_connection.h       # MQTT module public interface
├── mqtt_connection.cpp     # MQTT client, measurement queue and publisher task
├── wifi_connection.h       # Wi-Fi module public interface
└── wifi_connection.cpp     # Wi-Fi station mode implementation
```

## Architecture

### Data Flow

```
 producer                 measurement_queue           mqtt_publisher task              broker
(app_main)  ──enqueue──▶  [ Message × 5 ]  ──receive──▶  wait for MQTT_CONNECTED_BIT  ──publish──▶  MQTT
                                                              ▲
                                     MQTT event handler ──────┘ sets / clears the bit
```

Producers never talk to the MQTT client directly. They place a `Message` in the queue and return immediately; the publisher task takes care of delivery once the connection is up.

### Application Entry Point - `main.cpp`

`app_main()` performs the startup sequence:

1. **NVS initialization** - initializes non-volatile storage, which the Wi-Fi driver requires. If the partition has no free pages or uses an outdated format version, it is erased and initialized again.
2. **Network connection** - calls `wifi::wifi_init_sta()` to connect the device to the configured access point.
3. **Measurement queue** - calls `mqtt::measurement_queue_init()` to create the queue.
4. **MQTT client** - calls `mqtt::mqtt_init()` to start the client and connect to the broker.
5. **Publisher task** - calls `mqtt::mqtt_publisher_init()` to start the task that drains the queue.
6. **Sample data** - enqueues three test messages with `mqtt::enqueue_measurement()`.

> **Note:** The queue must be created before the publisher task, because the task receives the queue handle as its argument.

### Message – `measurement.h`

```cpp
struct Message {
    uint8_t plant_id;
    char data[20];
};
```

| Field | Meaning |
| --- | --- |
| `plant_id` | Identifier of the plant the measurement belongs to |
| `data` | Null-terminated payload sent to the broker (max. 19 characters) |

The structure has a fixed size, so it is copied into the queue by value and needs no dynamic allocation.

### Wi-Fi Module - `wifi_connection.h` / `wifi_connection.cpp`

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

### MQTT Module - `mqtt_connection.h` / `mqtt_connection.cpp`

The module lives in the `mqtt` namespace and is built on the ESP-IDF `mqtt_client` component. The client handle, the queue, the event group and the publisher task are kept in an anonymous namespace.

#### Public API

```cpp
namespace mqtt {
    void measurement_queue_init();
    bool mqtt_init();
    void mqtt_publisher_init();
    bool enqueue_measurement(const Message& message);
}
```

| Function | Description |
| --- | --- |
| `measurement_queue_init()` | Creates a queue for up to 5 `Message` items |
| `mqtt_init()` | Creates the event group, initializes the client, registers the event handler and starts the client. Returns `false` if the client could not be created |
| `mqtt_publisher_init()` | Starts the `mqtt_publisher` task (4096-byte stack, priority 5) |
| `enqueue_measurement()` | Copies a message into the queue. **Non-blocking**: returns `false` immediately if the queue is full |

`mqtt_init()` does not wait for the broker. The connection is established in the background and reported through the event group.

#### Connection State - Event Group

The connection state is stored in a FreeRTOS event group with a single bit, `MQTT_CONNECTED_BIT`, managed by the MQTT event handler:

| Event | Action |
| --- | --- |
| `MQTT_EVENT_CONNECTED` | Sets `MQTT_CONNECTED_BIT` |
| `MQTT_EVENT_DISCONNECTED` | Clears `MQTT_CONNECTED_BIT` |
| `MQTT_EVENT_PUBLISHED` | Logs the ID of the acknowledged message |
| Other events | Logged as not handled |

#### Publisher Task

The `mqtt_publisher` task runs an endless loop:

1. Blocks on `xQueueReceive` until a message is available.
2. Blocks on `xEventGroupWaitBits` until `MQTT_CONNECTED_BIT` is set. The bit is not cleared on exit, so as long as the connection is up, the call returns immediately.
3. Publishes the message payload with QoS 1 and the retain flag set.

Both waits use `portMAX_DELAY`, so the task consumes no CPU time while idle or while the broker is unreachable. Messages enqueued in the meantime stay in the queue until the connection is restored.

## Configuration

Network settings are defined in `main/Kconfig.projbuild` and set with `idf.py menuconfig` under **SmartPots configuration**:

| Option | Kconfig symbol | Default | Description |
| --- | --- | --- | --- |
| WiFi SSID | `CONFIG_ESP_WIFI_SSID` | `""` | Name of the Wi-Fi network |
| WiFi Password | `CONFIG_ESP_WIFI_PASSWORD` | `""` | Password of the Wi-Fi network |
| Maximum retry | `CONFIG_ESP_MAXIMUM_RETRY` | `5` | Number of reconnection attempts before giving up |

> **Note:** The values are stored in plain text in `sdkconfig`. Do not publish this file with real credentials.

MQTT settings are currently defined in `mqtt_connection.cpp`:

| Setting | Value |
| --- | --- |
| Broker URI | `mqtt://192.168.100.5:1883` |
| Topic | `smartpots/saloon/plant1` |
| QoS / retain | `1` / `true` |
| Queue length | `5` messages |

## Requirements

- ESP32 development board
- ESP-IDF v5.3 or newer
- MQTT broker reachable from the local network (e.g. Mosquitto)
- USB cable for flashing and power supply

## Build and Flash

```bash
idf.py set-target esp32
idf.py menuconfig
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

Replace `/dev/ttyUSB0` with the serial port of your board.

To watch the published messages, subscribe to the topic on any machine in the network:

```bash
mosquitto_sub -h 192.168.100.5 -t 'smartpots/#' -v
```

## Logging

| Tag | Source |
| --- | --- |
| `MAIN` | `main.cpp` |
| `WIFI` | `wifi_connection.cpp` |
| `MQTT` | `mqtt_connection.cpp` |

Example output after a successful start:

```
I (...) MAIN: ESP_WIFI_MODE_STA
I (...) WIFI: wifi_init_sta finished
I (...) WIFI: got ip:192.168.1.42 | Mask:255.255.255.0
I (...) WIFI: connected to AP SSID: <your-ssid>
I (...) MQTT: client started
I (...) MQTT: event handler call
I (...) MQTT: MQTT_EVENT_CONNECTED
I (...) MQTT: event handler call
I (...) MQTT: MQTT_EVENT_PUBLISHED, msg_id=...
```
