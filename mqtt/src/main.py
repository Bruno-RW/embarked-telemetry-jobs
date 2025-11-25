import json
from datetime import datetime

import paho.mqtt.client as mqtt
from paho.mqtt.enums import CallbackAPIVersion

from configs import (
    MQTT_BROKER_HOST, 
    MQTT_TOPIC,

    POSTGRE,

    QUERY_INSERT_DATA
)

def on_connect(client, userdata, flags, reason_code, properties):
    print(f"Conectado ao broker MQTT com código: {reason_code}")
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    payload = msg.payload.decode()

    print(f"--- MENSAGEM RECEBIDA ---")
    print(f"Tópico: {msg.topic}")
    print(f"Payload: {payload}")
    
    try:
        data = json.loads(payload)

        latitude = data.get("lat")
        longitude = data.get("lon")
        altitude = data.get("alt")
        hdop = data.get("hdop")
        satellites = data.get("sats")

        timestamp = datetime.now()

        params = (latitude, longitude, altitude, hdop, satellites, timestamp)
        POSTGRE.insert(QUERY_INSERT_DATA, params)

    except Exception as e:
        print(f"Erro ao processar dados: {e}")

def main():
    # POSTGRE.createTable(QUERY_CREATE_TABLE)

    client = mqtt.Client(callback_api_version=CallbackAPIVersion.VERSION2)
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(MQTT_BROKER_HOST, 1883, 60)
    client.loop_forever()

if __name__ == "__main__": main()