# 🌦️ Weather Station - Dual Core

## 📖 Sobre o Projeto

Este projeto implementa uma estação meteorológica que utiliza os dois núcleos do Raspberry Pi Pico W para coletar e exibir dados climáticos em tempo real. O Core 0 é responsável pela leitura dos sensores (BMP280 para temperatura, pressão e altitude; AHT20 para umidade), enquanto o Core 1 recebe os dados via FIFO e os exibe no display OLED. O sistema calcula a altitude com base na pressão atmosférica e permite reset via botão.

## 🚀 Funcionalidades

- 🌡️ **Leitura de temperatura** através do sensor BMP280
- 💧 **Leitura de umidade** através do sensor AHT20
- 📊 **Medição de pressão atmosférica** em hPa
- 🏔️ **Cálculo de altitude** baseado na pressão atmosférica
- 🖥️ **Display OLED** para visualização dos dados em tempo real
- 🔄 **Comunicação entre cores** via FIFO multicore
- 🔘 **Botão de reset** para reiniciar o sistema
- 📡 **Comunicação I2C** com múltiplos sensores

## 🛠 Tecnologias Utilizadas

- **Microcontrolador**: Raspberry Pi Pico W
- **Display**: OLED SSD1306 (I2C)
- **Sensores**:
  - BMP280 (Temperatura, Pressão e Altitude)
  - AHT20 (Umidade)
- **Componentes**:
  - Botão (Reset)
- **Protocolos de Comunicação**:
  - I2C (display OLED e sensores)
  - FIFO Multicore (comunicação entre núcleos)
- **Linguagem de Programação**:
  - C (utilizando o SDK do Raspberry Pi Pico)

## 🔧 Estrutura do Projeto

```
📂 tarefa22_dualcore_embarcatech
│── 📂 lib               # Bibliotecas auxiliares
│   ├── 📂 ssd1306       # Controle do display OLED
│   │   ├── ssd1306.c
│   │   ├── ssd1306.h
│   │   └── display.c
│   ├── 📂 bmp280        # Controle do sensor BMP280
│   │   ├── bmp280.c
│   │   └── bmp280.h
│   ├── 📂 aht20         # Controle do sensor AHT20
│   │   ├── aht20.c
│   │   └── aht20.h
│   └── 📂 button        # Controle dos botões
│       ├── button.c
│       └── button.h
│── main.c               # Código principal
│── CMakeLists.txt       # Configuração do build
└── README.md            # Documentação
```

## 🏗 Instalação e Configuração

### 📥 Pré-requisitos

- Raspberry Pi Pico W
- Display OLED SSD1306
- Sensor BMP280 (Temperatura, Pressão)
- Sensor AHT20 (Umidade)
- Botão
- Cabo micro USB
- **Software**:
  - Visual Studio Code + Extensão Raspberry Pi Pico
  - CMake
  - SDK do Raspberry Pi Pico

### ⚙️ Passos para Configuração

1. **Clone este repositório**:

```bash
git clone https://github.com/matheusssilva991/tarefa22_dualcore_embarcatech
```

2. **Compile o projeto**:

```bash
mkdir build && cd build
cmake -G Ninja ..
ninja
```

3. **Transfira o arquivo UF2** gerado para o Raspberry Pi Pico.

### 🔌 Conexões

- **I2C0**:
  - SDA: GPIO 0
  - SCL: GPIO 1
  - Sensores: BMP280 (0x76), AHT20 (0x38), Display OLED (0x3C)
- **Botão A**: GPIO (conforme definido em `button.h`)

## 🎮 Como Usar

- Ao ligar, o sistema inicia automaticamente a leitura dos sensores
- Os dados são exibidos no display OLED a cada atualização:
  - **Temperatura** em °C
  - **Umidade** em %
  - **Pressão** em hPa
  - **Altitude** em metros
- Pressione o **Botão A** para reiniciar o sistema (bootloader)
- Os dados também são enviados via USB serial para debug

### 📊 Ajuste de Altitude

Para obter leituras precisas de altitude, ajuste a constante `SEA_LEVEL_PRESSURE` no código para a pressão atmosférica local ao nível do mar (em Pa):

```c
#define SEA_LEVEL_PRESSURE 101325.0 // Ajuste conforme sua região
```

## 🧠 Arquitetura Multicore

- **Core 0**: Responsável pela leitura dos sensores e envio de dados via FIFO
- **Core 1**: Recebe os dados via interrupção FIFO e atualiza o display OLED

## Video Demonstrativo

[![Watch the video](https://img.youtube.com/vi/SEU_VIDEO_ID/maxresdefault.jpg)](https://youtu.be/SEU_VIDEO_ID)

## 📜 Licença

Este projeto é distribuído sob a licença MIT. Consulte o arquivo `LICENSE` para mais informações.

## 🤝 Contribuição

Sinta-se à vontade para contribuir! Caso tenha sugestões ou melhorias, abra uma issue ou faça um pull request.

---
✉️ Para dúvidas ou sugestões, entre em contato! 🚀

## 🤝 Equipe

Membros da equipe de desenvolvimento do projeto:
<table>
  <tr>
    <td align="center">
      <a href="https://github.com/matheusssilva991">
        <img src="https://github.com/matheusssilva991.png" width="100px;" alt="Foto de Matheus Santos Silva no GitHub"/><br>
        <b>Matheus Santos Silva (matheusssilva991)</b>
        <p>Desenvolvedor Embedded Systems</p>
      </a>
    </td>
  <tr>
</table>
