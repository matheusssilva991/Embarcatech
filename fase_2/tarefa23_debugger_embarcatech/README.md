# 🚀 Sistema de Controle de Ventilador com RP2040

## 📖 Sobre o Projeto

Sistema embarcado desenvolvido para Raspberry Pi Pico (RP2040/RP2350) que controla um ventilador através de módulos relés, permitindo ligar/desligar e selecionar entre 3 velocidades diferentes. O projeto utiliza interrupções de GPIO para leitura dos botões e comunicação UART para debug.

## 🎯 Funcionalidades

- ✨ **Liga/Desliga**: Controle on/off do sistema através de botão dedicado
- ✨ **3 Velocidades**: Seleção entre velocidade 1, 2 ou 3 via botões individuais
- ✨ **Controle via Relés**: Acionamento de módulos relés com lógica invertida (ativo em nível baixo)
- ✨ **Debounce em Hardware**: Implementação de debounce por software nas interrupções
- ✨ **Debug UART**: Saída serial para monitoramento do estado do sistema

## 🛠 Tecnologias Utilizadas

- **Linguagem**: C
- **SDK**: Raspberry Pi Pico SDK
- **Hardware**:
  - Raspberry Pi Pico (RP2040/RP2350)
  - Módulos Relés (4 canais)
  - Botões tácteis (4 unidades)
- **Bibliotecas**:
  - pico/stdlib.h
  - hardware/gpio.h
  - hardware/uart.h
  - hardware/irq.h
- **Ferramentas**:
  - CMake
  - GCC ARM Embedded
  - OpenOCD (debug)
  - Visual Studio Code

## 📁 Estrutura do Projeto

```
📂 tarefa23_debugger_embarcatech
│── 📄 main.c           # Código fonte principal
│── 📄 CMakeLists.txt   # Configuração CMake
│── 📄 pico_sdk_import.cmake
│── 📂 build            # Arquivos compilados
│── README.md           # Documentação principal
└── .vscode             # Configurações do VS Code
```

## 🔌 Pinagem

### Saídas (Relés)

- **GP6**: Relé Liga/Desliga
- **GP7**: Relé Velocidade 1
- **GP8**: Relé Velocidade 2
- **GP9**: Relé Velocidade 3

### Entradas (Botões)

- **GP10**: Botão Liga/Desliga
- **GP11**: Botão Velocidade 3
- **GP12**: Botão Velocidade 2
- **GP13**: Botão Velocidade 1

### UART (Debug)

- **GP16**: TX (Transmissão)
- **GP17**: RX (Recepção)
- **Baud Rate**: 115200

## 🏗 Instalação e Configuração

### 📥 Pré-requisitos

- Raspberry Pi Pico SDK instalado
- CMake (versão 3.13 ou superior)
- GCC ARM Embedded Toolchain
- OpenOCD (para debug)
- Git

### ⚙️ Passos para Configuração

1. **Clone este repositório**:

```bash
git clone https://github.com/matheusssilva991/tarefa23_debugger_embarcatech
cd tarefa23_debugger_embarcatech
```

2. **Configure o Pico SDK**:

```bash
export PICO_SDK_PATH=/caminho/para/pico-sdk
```

3. **Compile o projeto**:

```bash
mkdir build
cd build
cmake ..
make
```

4. **Faça upload para o Pico**:

```bash
# Segure o botão BOOTSEL e conecte o Pico via USB
cp main.uf2 /media/$USER/RPI-RP2/
```

## 🚀 Como Usar

1. **Conecte os componentes** conforme a pinagem especificada
2. **Alimentação dos relés**: Conecte fonte externa (5V recomendado)
   - COM → GND da fonte
   - NO → LED/Carga através de resistor apropriado
3. **Monitor Serial** (Linux):

   ```bash
   screen /dev/ttyACM0 115200
   ```

4. **Operação**:
   - Pressione o botão de Liga/Desliga (GP10) para ativar o sistema
   - Pressione os botões de velocidade (GP11-GP13) para selecionar a velocidade
   - Pressionar botão de velocidade automaticamente liga o sistema

## 🔍 Debug

Para debug via OpenOCD:

```bash
openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg
```

No VS Code, use a configuração de debug já incluída (F5).

## 🎥 Demonstração

[Adicione aqui vídeo ou imagens do projeto funcionando]

## 🤝 Contribuição

Contribuições são bem-vindas! Para contribuir:

1. Faça um fork do projeto
2. Crie uma branch para sua feature (`git checkout -b feature/MinhaFeature`)
3. Commit suas mudanças (`git commit -m 'Adiciona MinhaFeature'`)
4. Push para a branch (`git push origin feature/MinhaFeature`)
5. Abra um Pull Request

## 📜 Licença

Este projeto é de código aberto para fins educacionais.

## 👥 Equipe

Projeto desenvolvido como parte do programa Embarcatech.

## 📞 Contato

Para dúvidas ou sugestões sobre o projeto, abra uma issue no repositório.

---

⚡ Desenvolvido com Raspberry Pi Pico SDK
