# Jobs - Embedded Telemetry

This project is a GPS telemetry system that collects location data from an ESP32 device, transmits it via MQTT, stores it in a PostgreSQL database, and performs reverse geocoding to obtain address information.

## Architecture

```
┌─────────────┐       ┌─────────────┐       ┌─────────────┐       ┌─────────────┐
│   ESP32     │       │    MQTT     │       │    MQTT     │       │ PostgreSQL  │
│   + GPS     │ ───── │   Broker    │ ───── │   Service   │ ───── │  Database   │
└─────────────┘       └─────────────┘       └─────────────┘       └─────────────┘
                                                                         │
                                                                         │
                                                                  ┌──────┴──────┐
                                                                  │   Address   │
                                                                  │   Service   │
                                                                  └─────────────┘
```

## Project Structure

### `/esp32` - ESP32 Hardware Code

C++ code designed to run on ESP32 hardware (T-Beam/LoRa32) with a GPS module.

**Features:**

- Connects to WiFi network
- Reads GPS coordinates (latitude, longitude, altitude) using TinyGPSPlus library
- Publishes location data to an MQTT topic every 5 seconds
- Sends JSON payload containing: `lat`, `lon`, `alt_m`, `sats`, `hdop`, `status`
- Supports AXP192/AXP202 PMU for GPS power management

**Hardware Requirements:**

- ESP32 (T-Beam or LoRa32)
- GPS Module (connected via UART)
- WiFi connectivity

**Libraries Required:**

- WiFi
- PubSubClient
- TinyGPSPlus
- ArduinoJson
- axp20x

---

### `/mqtt` - MQTT Consumer Service

Python service that subscribes to the MQTT topic and stores received GPS data in a PostgreSQL database.

**Features:**

- Subscribes to the configured MQTT topic
- Parses incoming JSON messages with GPS data
- Inserts location records into the `location` table in PostgreSQL
- Handles connection management and error logging

**Running:**

```bash
cd mqtt
py -m venv .venv
.venv\scripts\activate
pip install -r requirements.txt
cd src
py main.py
```

---

### `/address` - Address Geocoding Service

Python service that performs reverse geocoding to convert GPS coordinates into human-readable addresses.

**Features:**

- Reads records from the `location` table in PostgreSQL
- Uses Nominatim (OpenStreetMap) for reverse geocoding via `geopy` library
- Extracts address components: country, state, region, city, postcode, road, house number
- Batch inserts the resolved addresses into the `address` table

**Running:**

```bash
cd address
py -m venv .venv
.venv\scripts\activate
pip install -r requirements.txt
cd src
py main.py
```

---

## Data Flow

1. **ESP32** reads GPS coordinates and publishes them to the MQTT broker
2. **MQTT Service** subscribes to the topic, receives the data, and stores it in the `location` table
3. **Address Service** reads from the `location` table, performs reverse geocoding, and inserts the address data into the `address` table

## Database Tables

- **location** - Stores raw GPS data (latitude, longitude, altitude, hdop, satellites, timestamp)
- **address** - Stores geocoded address information (country, state, city, postcode, road, etc.)

## Configuration

Each service requires proper configuration for:

- MQTT broker host and topic
- PostgreSQL connection credentials

Check the `configs/` folder in each service for configuration files.
