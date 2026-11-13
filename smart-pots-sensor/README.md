# Smart Pots Sensor

Soil moisture node on a Beetle ESP32-C3. Reads a capacitive probe on the ADC and logs the raw value. The radio module is the send path; ESP-NOW is not on the air yet.

## Hardware

Beetle ESP32-C3 (DFR0868). The USB port shows up as a CDC device, usually `/dev/ttyACM*`. The number depends on the machine.

| Probe | Beetle |
|---|---|
| VCC | 3V3 |
| GND | GND |
| AOUT | GPIO0 |

GPIO0 is `ADC1_CH0`. Attenuation is `ADC_ATTEN_DB_12`. The sample is 12-bit, from 0 to 4095.

## Project layout

```text
.
├── CMakeLists.txt          ESP-IDF project entry
└── main
    ├── CMakeLists.txt      source files passed to the build
    ├── main.cpp            app_main: init, then read / send / wait
    ├── soil_sensor.hpp     soil_sensor_init, soil_sensor_read
    ├── soil_sensor.cpp     ADC handle, channel setup, raw sample
    ├── radio.hpp           radio_init, radio_send
    └── radio.cpp           payload; ESP-NOW send comes later
```

`main` reads a sample and passes it to the radio. The sensor code does not build packets. The radio code does not interpret the number.

## Calibration

Each probe is calibrated once: raw in air is dry, raw with the whole probe in water is wet. Those two endpoints belong to the backend, not to this firmware. The packet carries `raw` as a `uint16_t`. Percent is `(dry - raw) * 100 / (dry - wet)`. The sender MAC comes from ESP-NOW, so the payload has no sensor id.

## Build and flash

ESP-IDF v5.3. Target is `esp32c3`.

```bash
ls -l /dev/serial/by-id/
idf.py set-target esp32c3
idf.py -p /dev/ttyACM0 -b 115200 flash monitor
```

Replace `/dev/ttyACM0` with the path from `by-id`. Leave the monitor with `Ctrl+]`.

## Next

`radio_init` starts Wi-Fi and adds the broadcast peer. `radio_send` sends the raw sample with `esp_now_send`.