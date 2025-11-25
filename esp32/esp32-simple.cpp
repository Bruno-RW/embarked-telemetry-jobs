#include <WiFi.h>
#include <PubSubClient.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>

// ================== CONFIG Wi-Fi ==================
const char* WIFI_NAME       = "Redmi 9";
const char* WIFI_PASSWORD   = "usguri32";


// ================== CONFIG MQTT ===================
const char* MQTT_SERVER    = "test.mosquitto.org";
const int   MQTT_PORT      = 1883;
const char* MQTT_TOPIC = "GPS-Boiaski12";

WiFiClient espClient;
PubSubClient client(espClient);


// ================== CONFIG GPS ====================
static const int GPS_RX_PIN = 34;
static const uint32_t GPS_BAUD = 9600;

TinyGPSPlus gps;
HardwareSerial SerialGPS(1);


// ================== TIMERS ========================
unsigned long lastPub = 0;
const unsigned long PUB_EVERY_MS = 5000;


// ================== FUNCTIONS =====================
void setupWifi() {
  Serial.println("\n[WiFi] Connecting...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.printf(
    "\n[WiFi] Connected! IP: %s\n",
    WiFi.localIP().toString().c_str()
  );
}

void reconnectMQTT() {
  while (WiFi.status() == WL_CONNECTED && !client.connected()) {
    String clientId = "ESP32-" + String((uint32_t)ESP.getEfuseMac(), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("[MQTT] Connected.");
    } else {
      Serial.print("[MQTT] Failed, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void printParsedData() {
  Serial.print("GPS Status: ");
  Serial.println(gps.location.isValid() ? "FIX ACQUIRED" : "NO FIX");

  if (!gps.location.isValid()) {
      Serial.printf(
        "Data: %d, HDOP: %.2f\n",
        gps.satellites.value(),
        gps.hdop.hdop()
      );
      return;
    }
    
    Serial.printf(
      "Lat: %.6f, Lon: %.6f, Alt: %.2f Sats: %d, HDOP: %.2f\n",
      gps.location.lat(),
      gps.location.lng(),
      gps.altitude.meters(),
      gps.satellites.value(),
      gps.hdop.hdop()
    );
}

void publishStatus() {
  StaticJsonDocument<256> doc;
  bool isFixed = gps.location.isValid();

  doc["ts"]      = (uint32_t)(millis() / 1000);
  doc["lat"]     = isFixed ? gps.location.lat() : 0.0;
  doc["lon"]     = isFixed ? gps.location.lng() : 0.0;
  doc["alt"]     = isFixed ? gps.altitude.meters() : 0.0;
  doc["sats"]    = gps.satellites.isValid() ? gps.satellites.value() : 0;
  doc["hdop"]    = gps.hdop.isValid() ? gps.hdop.hdop() : 99.99;
  doc["status"]  = isFixed ? "gps_ok" : "no_fix";

  char payload[256];
  size_t n = serializeJson(doc, payload, sizeof(payload));

  if (client.publish(MQTT_TOPIC, payload, n)) {
    Serial.print("[PUB] Sent: ");
    Serial.println(payload);

  } else {
    Serial.println("[PUB] Failed to publish.");
  }
}


// ================== SETUP & LOOP ==================
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Simplified GPS → MQTT ===");

  // 1. Start GPS Serial
  SerialGPS.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN); 

  // 2. Setup Wi-Fi and MQTT server
  setupWifi();
  client.setServer(MQTT_SERVER, MQTT_PORT);
}

void loop() {
  // 1. Process GPS data
  while (SerialGPS.available() > 0) {
    char c = SerialGPS.read();
    gps.encode(c);
  }

  // Check if location data is updated
  if (gps.location.isUpdated()) {
    printParsedData();
  }

  // 2. Maintain MQTT connection
  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    reconnectMQTT();
  }
  
  // 3. Maintain MQTT loop
  client.loop();

  // 4. Publish data on timer
  if (WiFi.status() == WL_CONNECTED && client.connected() && (millis() - lastPub >= PUB_EVERY_MS)) {
    lastPub = millis();
    publishStatus();
  }
}