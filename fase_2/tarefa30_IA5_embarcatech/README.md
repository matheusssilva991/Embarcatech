# 🌦️ Estação Meteorológica com IA - RP2040 (Pico W)

[![RP2040](https://img.shields.io/badge/RP2040-Raspberry%20Pi%20Pico%20W-C51A4A?logo=raspberrypi)](https://www.raspberrypi.com/products/raspberry-pi-pico-w/)
[![C/C++](https://img.shields.io/badge/C%2FC%2B%2B-00599C?logo=c%2B%2B&logoColor=white)](https://en.wikipedia.org/wiki/C%2B%2B)
[![TensorFlow Lite Micro](https://img.shields.io/badge/TFLM-FF6F00?logo=tensorflow&logoColor=white)](https://www.tensorflow.org/lite/microcontrollers)
[![Embarcatech](https://img.shields.io/badge/Embarcatech-Project-orange)](https://embarcatech.softex.br/)

## 📖 Visão Geral

Projeto de estação meteorológica embarcada com **RP2040 (Pico W)** que mede temperatura, umidade e pressão, calcula altitude e disponibiliza um **dashboard web** via HTTP. O sistema integra **IA embarcada** (TensorFlow Lite Micro) para estimar **probabilidade de chuva na próxima hora** a partir de séries temporais locais.

## ✨ Funcionalidades

- Leitura de sensores **AHT20** (umidade/temperatura) e **BMP280** (pressão/temperatura).
- **Dashboard web** responsivo com histórico, limites e ajustes de offset.
- **API HTTP** para leitura e configuração de limites de temperatura.
- **IA embarcada** (modelo CNN 1D INT8) para previsão de chuva.
- Alertas com **buzzer** e sinais visuais em **LED RGB** e **matriz WS2812B**.
- Reconexão automática ao Wi-Fi e exibição do IP no console serial.

## 🧩 Hardware Necessário

- Raspberry Pi Pico W (RP2040)
- Sensor AHT20 (I2C)
- Sensor BMP280 (I2C)
- Matriz de LEDs WS2812B 5x5 (PIO)
- LED RGB (PWM)
- 2 buzzers (PWM)
- Botões A/B (GPIO)
- Fonte 3.3V e GND comuns

## 🔌 Pinagem Utilizada

- **I2C0 (BMP280)**: SDA GPIO 0, SCL GPIO 1
- **I2C1 (AHT20)**: SDA GPIO 2, SCL GPIO 3
- **WS2812B (5x5)**: GPIO 7
- **LED RGB (PWM)**: Verde GPIO 11, Azul GPIO 12, Vermelho GPIO 13
- **Buzzer A**: GPIO 21
- **Buzzer B**: GPIO 10
- **Botão A**: GPIO 5
- **Botão B**: GPIO 6

## 🧠 Modelo de IA (Previsão de Chuva)

- **Tipo**: CNN 1D quantizada INT8 (TensorFlow Lite Micro)
- **Entrada**: janela temporal com 24 amostras e 6 features (temperatura, umidade, pressão e deltas históricos).
- **Saída**: probabilidade de chuva (0-1). Limiar padrão: **0.5**.
- **Arquivos**:
  - Modelo compilado: [models/model_int8.tflite](models/model_int8.tflite)
  - Header embutido: [models/model_int8.h](models/model_int8.h)
  - Wrapper TFLM: [src/tflm_wrapper.cpp](src/tflm_wrapper.cpp)

## 📊 Dados e Treinamento

- **Fonte dos dados**: INMET (Kaggle)
  - <https://www.kaggle.com/datasets/gregoryoliveira/brazil-weather-information-by-inmet/data>
- **Notebooks**:
  - [src/notebooks/prepare_weather_ba.ipynb](src/notebooks/prepare_weather_ba.ipynb) (tratamento e preparo)
  - [src/notebooks/train_ia.ipynb](src/notebooks/train_ia.ipynb) (treino do modelo)
- **Recorte de treino**: séries de **Ilhéus-BA** para previsão binária (chuva/sem chuva).
- **Dataset consolidado**: [data/weather_ba.parquet](data/weather_ba.parquet)

## 🌐 Dashboard e API HTTP

- **UI embutida**: servida pela Pico W na porta **80**.
- **Endpoints**:
  - `GET /api/weather` → JSON com temperatura, umidade, pressão, altitude e probabilidade de chuva.
  - `POST /api/limits` → ajusta limites e offset.

Exemplo de payload para `POST /api/limits`:

```json
{ "min": 10, "max": 70, "offset": 0 }
```

## 🛠️ Como Compilar e Gravar

### Pré-requisitos

- Pico SDK configurado
- CMake e Ninja
- ARM GCC
- Python 3

### Passos

1) Configure o Wi-Fi em [config/wifi_config.h](config/wifi_config.h). Se preferir, copie o modelo:

```bash
cp config/wifi_config_example.h config/wifi_config.h
```

1) Compile:

```bash
cmake -B build -G Ninja
ninja -C build
```

1) Grave no Pico W:

- **BOOTSEL**: copie o `main.uf2` gerado em `build/` para o drive RPI-RP2.
- **Picotool** (opcional): `picotool load -f build/main.uf2`.

## 💻 Servidor Local (Opcional)

Para testar o dashboard no PC com dados simulados:

```bash
cd public
npm install
npm start
```

## 📁 Estrutura do Repositório

```
📂 tarefa30_IA5_embarcatech
├── config/              # Wi-Fi e configs
├── data/                # Dataset consolidado
├── lib/                 # Drivers (AHT20, BMP280, WS2812B, etc.)
├── models/              # Modelos TFLite (INT8)
├── public/              # UI web e servidor local
├── src/                 # Firmware principal + wrapper TFLM
├── CMakeLists.txt
└── README.md
```

## 📌 Observações

- O botão **A** entra em modo BOOTSEL.
- O botão **B** reseta limites e offset de temperatura.
- O LED RGB indica nível de umidade; a matriz WS2812B indica faixa térmica.

## 👥 Autor

Desenvolvido por **Matheus Santos Silva** no programa Embarcatech.

## 📜 Licença

Este projeto é open source e está disponível para fins educacionais.
