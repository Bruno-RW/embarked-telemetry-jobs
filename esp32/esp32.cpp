#include <WiFi.h>
#include <PubSubClient.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <axp20x.h>

// ================== CONFIG Wi-Fi ==================
const char* ssid     = "Gelson_Wifi";
const char* password = "guilherme";

// ================== CONFIG MQTT ===================
const char* mqtt_server    = "test.mosquitto.org";
const int   mqtt_port      = 1883;
const char* mqtt_topic_out = "GPS-Boiaski12";

WiFiClient espClient;
PubSubClient client(espClient);

// ================== CONFIG GPS ====================
static const int GPS_RX_PIN = 34;     // ESP32 RX ← TX do GPS
static const int GPS_TX_PIN = 12;     // ESP32 TX → RX do GPS (opcional)
static const uint32_t GPS_BAUD = 9600;

TinyGPSPlus gps;
HardwareSerial SerialGPS(1);

// ================== PMU (AXP) =====================
AXP20X_Class pmu;

// ================== TIMERS ========================
unsigned long lastPub = 0;
const unsigned long PUB_EVERY_MS = 5000;

void logStateMQTT() {
  Serial.print("MQTT state = ");
  Serial.println(client.state());
}

void powerGPS() {
  Wire.begin(21, 22);

  // Tenta AXP192 e AXP202
  // Se não houver PMU segue sem

  int r = pmu.begin(Wire, AXP192_SLAVE_ADDRESS);
  if (r != AXP_PASS) r = pmu.begin(Wire, AXP202_SLAVE_ADDRESS);

  if (r == AXP_PASS) {
    pmu.setLDO3Mode(AXP202_LDO3_MODE_LDO);
    pmu.setPowerOutPut(AXP202_LDO3, AXP202_ON);

    Serial.println("PMU OK: LDO3 ligado (GPS ON).");

  } else {
    Serial.println("PMU não detectada (seguindo mesmo assim).");
  }
}

void setupWifi() {
  Serial.println("\n[WiFi] Iniciando...");

  WiFi.persistent(false);
  WiFi.disconnect(true, true);
  delay(100);

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setHostname("tbeam-gps");

  Serial.printf("[WiFi] Conectando a \"%s\"...\n", ssid);
  WiFi.begin(ssid, password);

  const uint32_t MAX_WAIT_MS = 30000; // 30s
  uint32_t t0 = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - t0 < MAX_WAIT_MS) {
    delay(300);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[WiFi] Falhou em conectar.");
    Serial.println("Dicas: 2.4GHz, WPA2/WPA2+WPA3, sem band steering, sem filtro de MAC.");
    return;
  }

  Serial.printf(
    "\n[WiFi] Conectado! IP: %s | RSSI: %ddBm | Canal: %d\n",
    WiFi.localIP().toString().c_str(),
    WiFi.RSSI(),
    WiFi.channel()
  );
}

void reconnectMQTT() {
  while (WiFi.status() == WL_CONNECTED && !client.connected()) {
    String clientId = "TBeamClient-" + String((uint32_t)ESP.getEfuseMac(), HEX);

    Serial.print("[MQTT] Conectando como ");
    Serial.println(clientId);

    if (client.connect(clientId.c_str())) {
      Serial.println("[MQTT] Conectado.");

    } else {
      Serial.print("[MQTT] Falhou, rc=");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void publishStatus() {
  StaticJsonDocument<256> doc;
  doc["ts"] = (uint32_t)(millis() / 1000);

  if (gps.location.isValid() && gps.location.isUpdated()) {
    doc["lat"]  = String(gps.location.lat(), 6);
    doc["lon"]  = String(gps.location.lng(), 6);

    if (gps.altitude.isValid())   doc["alt_m"] = gps.altitude.meters();
    if (gps.satellites.isValid()) doc["sats"]  = gps.satellites.value();
    if (gps.hdop.isValid())       doc["hdop"]  = gps.hdop.hdop();

    doc["status"] = "gps_ok";

  } else {
    doc["lat"]  = "00";
    doc["lon"]  = "00";
    doc["sats"] = gps.satellites.isValid() ? gps.satellites.value() : 0;

    if (gps.hdop.isValid()) {
      doc["hdop"]  = gps.hdop.hdop();
    }
    
    doc["status"] = "no_fix";
  }

  char payload[256];
  size_t n = serializeJson(doc, payload, sizeof(payload));

  bool ok = client.publish(mqtt_topic_out, payload, n);
  if (ok) {
    Serial.print("[PUB] ");
    Serial.println(payload);

  } else {
    Serial.println("[PUB] Falha na publicação");
    logStateMQTT();
  }
}


void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("\n=== T-Beam/LoRa32 GPS → MQTT ===");

  powerGPS();

  SerialGPS.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

  setupWifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  while (SerialGPS.available() > 0) {
    gps.encode(SerialGPS.read());
  }

  // Mantém Wi-Fi
  if (WiFi.status() != WL_CONNECTED) {
    static uint32_t lastTry = 0;
    if (millis() - lastTry > 5000) {
      lastTry = millis();
      setupWifi();
    }
  }

  // Mantém MQTT
  if (WiFi.status() == WL_CONNECTED && !client.connected()) {
    reconnectMQTT();
  }
  client.loop();

  // Publica a cada 5s
  if (millis() - lastPub >= PUB_EVERY_MS) {
    lastPub = millis();

    if (WiFi.status() == WL_CONNECTED && client.connected()) {
      publishStatus();

    } else {
      Serial.println("[PUB] Sem Wi-Fi/MQTT — pulando publicação.");
    }
  }

  if (millis() > 60000 && gps.charsProcessed() < 10) {
    Serial.println(F("[GPS] Sem NMEA após 60s. Cheque antena/bauds/pinos."));
  }
}