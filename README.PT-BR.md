# Jobs - Telemetria Embarcada

Este projeto é um sistema de telemetria GPS que coleta dados de localização de um dispositivo ESP32, transmite via MQTT, armazena em um banco de dados PostgreSQL e realiza geocodificação reversa para obter informações de endereço.

## Arquitetura

```
┌─────────────┐       ┌─────────────┐       ┌─────────────┐       ┌─────────────┐
│   ESP32     │       │    MQTT     │       │   Serviço   │       │ PostgreSQL  │
│   + GPS     │ ───── │   Broker    │ ───── │    MQTT     │ ───── │   Banco     │
└─────────────┘       └─────────────┘       └─────────────┘       └─────────────┘
                                                                         │
                                                                         │
                                                                  ┌──────┴──────┐
                                                                  │   Serviço   │
                                                                  │  Endereço   │
                                                                  └─────────────┘
```

## Estrutura do Projeto

### `/esp32` - Código do Hardware ESP32

Código C++ projetado para rodar no hardware ESP32 (T-Beam/LoRa32) com módulo GPS.

**Funcionalidades:**

- Conecta à rede WiFi
- Lê coordenadas GPS (latitude, longitude, altitude) usando a biblioteca TinyGPSPlus
- Publica dados de localização em um tópico MQTT a cada 5 segundos
- Envia payload JSON contendo: `lat`, `lon`, `alt_m`, `sats`, `hdop`, `status`
- Suporta PMU AXP192/AXP202 para gerenciamento de energia do GPS

**Requisitos de Hardware:**

- ESP32 (T-Beam ou LoRa32)
- Módulo GPS (conectado via UART)
- Conectividade WiFi

**Bibliotecas Necessárias:**

- WiFi
- PubSubClient
- TinyGPSPlus
- ArduinoJson
- axp20x

---

### `/mqtt` - Serviço Consumidor MQTT

Serviço Python que se inscreve no tópico MQTT e armazena os dados GPS recebidos em um banco de dados PostgreSQL.

**Funcionalidades:**

- Se inscreve no tópico MQTT configurado
- Analisa mensagens JSON recebidas com dados GPS
- Insere registros de localização na tabela `location` no PostgreSQL
- Gerencia conexões e registra erros

**Execução:**

```bash
cd mqtt
py -m venv .venv
.venv\scripts\activate
pip install -r requirements.txt
cd src
py main.py
```

---

### `/address` - Serviço de Geocodificação de Endereços

Serviço Python que realiza geocodificação reversa para converter coordenadas GPS em endereços legíveis.

**Funcionalidades:**

- Lê registros da tabela `location` no PostgreSQL
- Usa Nominatim (OpenStreetMap) para geocodificação reversa via biblioteca `geopy`
- Extrai componentes do endereço: país, estado, região, cidade, CEP, rua, número
- Insere os endereços resolvidos em lote na tabela `address`

**Execução:**

```bash
cd address
py -m venv .venv
.venv\scripts\activate
pip install -r requirements.txt
cd src
py main.py
```

---

## Fluxo de Dados

1. **ESP32** lê as coordenadas GPS e as publica no broker MQTT
2. **Serviço MQTT** se inscreve no tópico, recebe os dados e os armazena na tabela `location`
3. **Serviço de Endereços** lê da tabela `location`, realiza geocodificação reversa e insere os dados de endereço na tabela `address`

## Tabelas do Banco de Dados

- **location** - Armazena dados GPS brutos (latitude, longitude, altitude, hdop, satélites, timestamp)
- **address** - Armazena informações de endereço geocodificadas (país, estado, cidade, CEP, rua, etc.)

## Configuração

Cada serviço requer configuração adequada para:

- Host e tópico do broker MQTT
- Credenciais de conexão do PostgreSQL

Verifique a pasta `configs/` em cada serviço para os arquivos de configuração.
