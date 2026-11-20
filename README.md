# 🌱 SmartPots

## Overview

SmartPots is a distributed IoT system designed to help users monitor soil moisture and maintain suitable growing conditions for their plants. ESP32-based sensor nodes collect measurements and transmit them wirelessly to a central gateway, which forwards the data to an MQTT broker.

The project is under active development. Its longer-term goal is to provide plant-specific monitoring and an event-driven backend for storing and presenting measurements.

## System Architecture

### Sensor nodes

Sensor nodes use ESP-NOW, a connectionless communication protocol based on IEEE 802.11 vendor-specific action frames. Devices identify communication peers using MAC addresses. The intended operating cycle is to wake up, measure soil moisture, transmit a reading, and return to deep sleep to reduce power consumption.

### Gateway

The ESP32 gateway receives measurements over ESP-NOW and forwards them over Wi-Fi using MQTT. The firmware uses FreeRTOS queues to transfer messages between processing stages.

The planned pairing mechanism will allow the gateway and sensors to discover each other and establish authenticated, encrypted communication. This security functionality is not yet complete.

### Cloud and MQTT

The MQTT broker distributes sensor measurements to subscribed services. A future cloud integration will consume messages by topic and pass them to the application backend.

### Backend

The planned backend will process incoming events without continuously polling sensors, store measurements in a database, and expose the information in a format suitable for a user-facing application. The backend and frontend technology stacks have not been selected yet.

### Data flow

```text
ESP32-C3 Sensor Node
    | Soil moisture measurement (ADC)
    | ESP-NOW
    v
ESP32 Gateway
    | Message processing / FreeRTOS queue
    | Wi-Fi + MQTT
    v
MQTT Broker
    |
    v
Cloud Consumer / Backend (planned)
    |
    v
Database + Frontend (planned)
```

## Technologies

**Embedded firmware:** C++, ESP-IDF, ESP32 / ESP32-C3, FreeRTOS, ESP-NOW, Wi-Fi, ADC

**Messaging and build tools:** MQTT, CMake

**Testing:** Unity Test Framework

**Planned / not selected:** Cloud services, backend framework, database, frontend framework

## Features

### Implemented / in progress

- Soil moisture acquisition using an ESP32-based sensor node.
- Wireless sensor-to-gateway communication over ESP-NOW.
- Gateway-side message processing and inter-task communication using FreeRTOS queues.
- Publishing sensor measurements to an MQTT broker over Wi-Fi.
- Unit tests for message parsing using the Unity Test Framework.

### Planned

- Secure sensor pairing and encrypted communication.
- Low-power sensor operation using deep sleep.
- Plant-specific moisture monitoring based on a plant type selected by the user.
- Cloud-based message consumption, measurement storage, and a user-facing interface.
- Optional monitoring of soil nutrients for selected plant types (additional hardware required).
- Grow-light intensity and lighting-cycle control based on growth stage (additional hardware required).

## Project Structure

The project is divided into two firmware applications with separate documentation:

```text
SmartPots/
├── smart-pots-gateway/       # ESP32 gateway: ESP-NOW reception, processing and MQTT
├── smart-pots-sensor/        # ESP32-C3 node: soil moisture acquisition and ESP-NOW
└── README.md      # System overview
```

## Roadmap

- [x] Read soil moisture measurements using ADC.
- [x] Establish ESP-NOW communication between sensor and gateway.
- [x] Forward received measurements through a FreeRTOS queue.
- [x] Publish measurements to an MQTT broker.
- [x] Add initial Unity tests for message parsing.
- [ ] Implement secure pairing with Proof of Possession (PoP) and application-key provisioning.
- [ ] Persist pairing information in NVS and handle reconnects / restarts.
- [ ] Implement and measure sensor deep-sleep operation.
- [ ] Add cloud message consumption and persistent storage.
- [ ] Build a user interface for plant-specific monitoring.
- [ ] Explore soil nutrient monitoring and grow-light automation.

---

**Status:** Work in progress. Features and architecture may evolve as the project develops.
