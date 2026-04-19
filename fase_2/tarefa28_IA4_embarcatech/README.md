# 🧠 CNN MNIST - Raspberry Pi Pico W com TensorFlow Lite Micro

[![RP2040](https://img.shields.io/badge/RP2040-Raspberry%20Pi%20Pico-C51A4A?logo=raspberrypi)](https://www.raspberrypi.com/products/raspberry-pi-pico/)
[![C](https://img.shields.io/badge/C-00599C?logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Python](https://img.shields.io/badge/Python-3776AB?logo=python&logoColor=white)](https://www.python.org/)
[![TensorFlow](https://img.shields.io/badge/TensorFlow%20Lite-FF6F00?logo=tensorflow&logoColor=white)](https://www.tensorflow.org/lite/microcontrollers)
[![Embarcatech](https://img.shields.io/badge/Embarcatech-Project-orange)](https://embarcatech.softex.br/)

Sistema de reconhecimento de dígitos manuscritos (MNIST) usando uma Rede Neural Convolucional (CNN) quantizada em INT8, executada em tempo real no microcontrolador RP2040 com TensorFlow Lite Micro.

<p align="center">
  <img src="https://img.shields.io/badge/Acurácia-98.7%25-success" alt="Accuracy">
  <img src="https://img.shields.io/badge/Modelo-23KB-blue" alt="Model Size">
  <img src="https://img.shields.io/badge/RAM-120KB-yellow" alt="RAM Usage">
  <img src="https://img.shields.io/badge/Inferência-~250ms-informational" alt="Inference Time">
</p>

---

## 📖 Sobre o Projeto

Este projeto implementa um sistema completo de **Machine Learning embarcado** para reconhecimento de dígitos manuscritos do dataset MNIST. O modelo CNN foi treinado, quantizado para INT8 e embarcado no Raspberry Pi Pico W, permitindo inferência em tempo real sem necessidade de conectividade com a nuvem.

### 🎯 Características Principais

- 🧠 **Rede Neural Convolucional** quantizada INT8
- ⚡ **Inferência em tempo real** no RP2040 (133 MHz)
- 📡 **Comunicação USB Serial** para entrada/saída de dados
- 🎨 **Matriz de LEDs WS2812B** para visualização do dígito previsto
- 🖥️ **Display OLED SSD1306** para informações adicionais
- 🔘 **Botão B** com interrupção para modo BOOTSEL
- 🎯 **Acurácia > 98%** mantida após quantização
- 🔧 **20 amostras de teste** incluídas no projeto

### 🏗️ Arquitetura da CNN

```
Input (28×28×1) → Conv2D(32) → MaxPool → Conv2D(64) → MaxPool →
Flatten → Dense(128) → Dropout → Dense(10) → Softmax
```

**Especificações:**

- **Parâmetros**: ~93.000
- **Tamanho**: ~23 KB (quantizado INT8)
- **Arena TFLite**: ~100 KB
- **Acurácia float32**: 98.9%
- **Acurácia INT8**: 98.7%
- **Degradação**: 0.2% ✅

---

## 🚀 Começando

### 📦 Pré-requisitos

#### Hardware

- Raspberry Pi Pico W (ou Pico padrão)
- Matriz de LEDs WS2812B 5x5 (conectada ao GPIO 7)
- Display OLED SSD1306 128x64 (I2C)
- Botão conectado ao GPIO 6 (Botão B)
- Cabo USB para programação e comunicação serial

#### Software

- **Raspberry Pi Pico SDK** (versão 2.0+)
- **CMake** (versão 3.13 ou superior)
- **ARM GCC Compiler** (arm-none-eabi-gcc)
- **Python 3.8+** (para scripts de teste)
- **Git**

### 🔧 Instalação

#### 1. Clone o repositório

```bash
git clone https://github.com/matheusssilva991/tarefa28_IA4_embarcatech.git
cd tarefa28_IA4_embarcatech
```

#### 2. Clone a biblioteca pico-tflmicro

A biblioteca TensorFlow Lite Micro para Pico deve ser clonada dentro do diretório `lib/`:

```bash
cd lib
git clone https://github.com/raspberrypi/pico-tflmicro.git
cd ..
```

#### 3. Configure o Pico SDK

Se ainda não tiver o Pico SDK instalado:

```bash
# Linux/macOS
export PICO_SDK_PATH=/caminho/para/pico-sdk

# Adicionar permanentemente ao ~/.bashrc ou ~/.zshrc
echo 'export PICO_SDK_PATH=/caminho/para/pico-sdk' >> ~/.bashrc
source ~/.bashrc
```

#### 4. Instale dependências Python

```bash
# Criar ambiente virtual (opcional, mas recomendado)
python3 -m venv .venv
source .venv/bin/activate  # Linux/macOS
# .venv\Scripts\activate   # Windows

# Instalar dependências
pip install pyserial numpy
```

#### 5. Compile o projeto

```bash
mkdir -p build
cd build
cmake ..
make -j4
```

O arquivo `main.uf2` será gerado no diretório `build/`.

#### 6. Carregue o firmware no Pico

**Método 1: BOOTSEL Manual**

```bash
# 1. Desconecte o Pico do USB
# 2. Segure o botão BOOTSEL no Pico
# 3. Conecte o Pico ao USB (mantendo BOOTSEL pressionado)
# 4. O Pico aparecerá como drive USB (RPI-RP2)
# 5. Copie o arquivo main.uf2 para o drive
cp build/main.uf2 /media/$USER/RPI-RP2/
```

**Método 2: Usando Picotool**

```bash
picotool load -v -x build/main.uf2
picotool reboot
```

---

## 💻 Como Usar

### 🧪 Teste Rápido com Script Python

O projeto inclui um script completo para testar a inferência:

```bash
# Sintaxe
python3 src/test_serial_mnist.py <porta_serial> <arquivo_sample>

# Exemplo Linux
python3 src/test_serial_mnist.py /dev/ttyACM0 samples/mnist_sample_05.h

# Exemplo Windows
python3 src/test_serial_mnist.py COM3 samples/mnist_sample_05.h

# Exemplo macOS
python3 src/test_serial_mnist.py /dev/cu.usbmodem14201 samples/mnist_sample_05.h
```

### 📊 Exemplo de Saída

```
============================================================
  RESULTADO DA INFERÊNCIA
============================================================
Label esperado:     5
Dígito predito:     5
Correto:            ✓ SIM

Scores por classe:
  0: -4.2345
  1: -3.1234
  2: -2.4567
  3: -1.8901
  4: -0.9876
  5:  5.6789 ████████████████████████████ ← PREDIÇÃO
  6: -3.4567
  7: -2.1234
  8: -1.5678
  9: -2.8901
============================================================
```

### 📡 Protocolo de Comunicação Serial

**Entrada:**

```
START\n
[784 bytes de pixels (valores 0-255)]
END\n
```

**Saída (JSON):**

```json
{
  "predicted_digit": 5,
  "scores": [
    {"class": 0, "score": -4.234567},
    {"class": 1, "score": -3.123456},
    ...
    {"class": 9, "score": -2.890123}
  ]
}
```

### 🎨 Visualização na Matriz de LEDs

Após cada inferência, o dígito previsto é exibido automaticamente na matriz WS2812B:

- ✅ **Display imediato** após inferência
- ⏱️ **Mantém por 2 segundos** antes de limpar
- 🎨 **Cada número tem sua cor** característica
- 🔄 **Atualiza imediatamente** se receber nova imagem

**Cores dos números:**

- 0: Vermelho | 1: Verde | 2: Azul | 3: Amarelo | 4: Ciano
- 5: Magenta | 6: Branco | 7: Laranja | 8: Roxo | 9: Marrom

### 🔘 Modo BOOTSEL via Botão B

Pressione o **Botão B (GPIO 6)** a qualquer momento para entrar em modo BOOTSEL:

- ⚡ **Interrupção por hardware** - funciona durante qualquer operação
- 🔄 **Atualização facilitada** - sem precisar desconectar o Pico
- 🛡️ **Debounce automático** - evita acionamentos acidentais

```
Pressionar Botão B → Mensagem "*** BOTÃO B PRESSIONADO ***"
                    → Pico reinicia em modo BOOTSEL
                    → Drive USB aparece
                    → Arraste novo firmware
```

---

## 📁 Estrutura do Projeto

```
tarefa28_IA4_embarcatech/
├── 📂 src/                          # Código fonte
│   ├── main.c                       # Sistema principal de inferência
│   ├── tflm_wrapper.cpp             # Wrapper C++ para TFLite Micro
│   ├── tflm_wrapper.h               # Header do wrapper
│   ├── test_serial_mnist.py         # Script de teste via serial
│   ├── example_manual_serial.py     # Exemplo de comunicação manual
│   └── CNN_MNIST.ipynb              # Notebook de treinamento
├── 📂 models/                        # Modelos treinados
│   ├── mnist_cnn_float.keras        # Modelo original float32
│   ├── mnist_cnn_int8.tflite        # Modelo quantizado INT8
│   └── mnist_cnn_int8_model.h       # Modelo embarcado (array C)
├── 📂 samples/                       # 20 amostras de teste
│   ├── mnist_sample_00.h            # Amostra 0 (dígito 7)
│   ├── mnist_sample_01.h            # Amostra 1 (dígito 2)
│   └── ... (até mnist_sample_19.h)
├── 📂 lib/                           # Bibliotecas
│   ├── pico-tflmicro/               # TensorFlow Lite Micro (git clone)
│   ├── ws2812b/                     # Controle matriz de LEDs
│   │   ├── ws2812b.c
│   │   ├── ws2812b.h
│   │   ├── number/                  # Padrões dos números
│   │   │   ├── led_matrix_numbers.c
│   │   │   └── led_matrix_numbers.h
│   │   └── pio/
│   │       └── ws2812b.pio
│   ├── ssd1306/                     # Display OLED
│   ├── button/                      # Controle de botões
│   └── ... (outras bibliotecas)
├── 📂 build/                         # Arquivos de compilação
│   └── main.uf2                     # Firmware final
├── CMakeLists.txt                   # Configuração CMake
├── pico_sdk_import.cmake            # Importação Pico SDK
└── README.md                        # Este arquivo
```

---

## 🔬 Detalhes Técnicos

### Quantização INT8

O modelo foi quantizado de float32 para INT8:

**Benefícios:**

- 📉 Redução de ~4x no tamanho do modelo
- ⚡ Operações inteiras mais rápidas no RP2040
- 💾 Menor consumo de RAM durante inferência

**Fórmulas:**

```c
// Quantização: float → int8
int8_t q = round(float_value / scale) + zero_point

// Dequantização: int8 → float
float f = (int8_value - zero_point) * scale
```

### Requisitos de Memória

| Componente | Tamanho |
|------------|---------|
| **Código do programa** | ~50 KB |
| **Biblioteca TFLite Micro** | ~80 KB |
| **Modelo CNN** | ~23 KB |
| **Arena TFLite (RAM)** | ~100 KB |
| **Stack/Heap (RAM)** | ~20 KB |
| **Total Flash** | ~150 KB / 2 MB (7.5%) |
| **Total RAM** | ~120 KB / 264 KB (45%) |

### Performance

| Métrica | Valor |
|---------|-------|
| **Tempo de inferência** | 200-300ms |
| **Clock RP2040** | 133 MHz |
| **Throughput** | 3-5 inferências/s |
| **Acurácia Float32** | 98.9% |
| **Acurácia INT8** | 98.7% |
| **Degradação** | 0.2% |

---

## 🎓 Treinamento do Modelo

O notebook [CNN_MNIST.ipynb](src/CNN_MNIST.ipynb) contém todo o processo:

### 1. Preparação dos Dados

```python
(x_train, y_train), (x_test, y_test) = keras.datasets.mnist.load_data()
x_train = x_train.astype('float32') / 255.0
```

### 2. Arquitetura do Modelo

```python
model = keras.Sequential([
    layers.Conv2D(32, (3,3), activation='relu', input_shape=(28,28,1)),
    layers.MaxPooling2D((2,2)),
    layers.Conv2D(64, (3,3), activation='relu'),
    layers.MaxPooling2D((2,2)),
    layers.Flatten(),
    layers.Dense(128, activation='relu'),
    layers.Dropout(0.5),
    layers.Dense(10, activation='softmax')
])
```

### 3. Treinamento

```python
model.compile(optimizer='adam',
              loss='sparse_categorical_crossentropy',
              metrics=['accuracy'])
model.fit(x_train, y_train, epochs=10, validation_split=0.1)
```

### 4. Quantização e Conversão

```python
converter = tf.lite.TFLiteConverter.from_keras_model(model)
converter.optimizations = [tf.lite.Optimize.DEFAULT]
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS_INT8]
converter.inference_input_type = tf.int8
converter.inference_output_type = tf.int8
tflite_model = converter.convert()
```

---

## 🐛 Solução de Problemas

### Porta serial não encontrada

```bash
# Linux: verificar dispositivos
ls -l /dev/ttyACM*
dmesg | tail -20

# Adicionar usuário ao grupo dialout
sudo usermod -a -G dialout $USER
# Fazer logout e login novamente
```

### Dispositivo desconecta durante comunicação

```bash
# Fechar outros programas que usam a porta
sudo fuser -k /dev/ttyACM0

# Verificar cabo USB
# Tentar outra porta USB
```

### Erro de compilação

```bash
# Verificar PICO_SDK_PATH
echo $PICO_SDK_PATH

# Limpar e recompilar
cd build
make clean
rm -rf *
cmake ..
make -j4
```

### Matriz de LEDs não funciona

- Verificar conexão no GPIO 7
- Verificar alimentação 5V dos LEDs
- Verificar se `led_matrix_numbers.c` está no CMakeLists.txt

### Botão B não responde

- Verificar conexão no GPIO 6
- Botão deve conectar GPIO ao GND quando pressionado
- Verificar se há ruído elétrico (adicionar capacitor de 100nF)

---

## 📚 Bibliotecas e Dependências

### TensorFlow Lite Micro

- **Repositório**: [raspberrypi/pico-tflmicro](https://github.com/raspberrypi/pico-tflmicro)
- **Função**: Framework de ML para microcontroladores
- **Licença**: Apache 2.0

### Raspberry Pi Pico SDK

- **Repositório**: [raspberrypi/pico-sdk](https://github.com/raspberrypi/pico-sdk)
- **Função**: SDK oficial para desenvolvimento no RP2040
- **Licença**: BSD 3-Clause

### Python Libraries

```bash
pip install pyserial  # Comunicação serial
pip install numpy     # Manipulação de arrays
```

---

## 🎯 Casos de Uso

Este projeto demonstra aplicações práticas de:

1. **Edge AI / TinyML** - Machine learning em dispositivos com recursos limitados
2. **Reconhecimento de padrões** - OCR de dígitos manuscritos
3. **Sistemas embarcados inteligentes** - Microcontroladores com capacidade de inferência
4. **Prototipagem rápida** - Interface serial para testes e desenvolvimento
5. **IoT sem nuvem** - Processamento local sem necessidade de conectividade

**Aplicações práticas:**

- 🔢 Reconhecimento de dígitos em displays/medidores
- 🤖 Robótica com visão computacional
- 🏭 Inspeção de qualidade industrial
- 🎓 Educação em IA/ML embarcado
- 🔬 Prototipagem de sistemas inteligentes

---

## 🤝 Contribuindo

Contribuições são bem-vindas! Para contribuir:

1. Fork o projeto
2. Crie uma branch para sua feature (`git checkout -b feature/MinhaFeature`)
3. Commit suas mudanças (`git commit -m 'Adiciona MinhaFeature'`)
4. Push para a branch (`git push origin feature/MinhaFeature`)
5. Abra um Pull Request

---

## 📄 Licença

Este projeto é parte do programa **Embarcatech** e está disponível para fins educacionais.

---

## 👨‍💻 Desenvolvedor

<table>
  <tr>
    <td align="center">
      <a href="https://github.com/matheusssilva991">
        <img src="https://github.com/matheusssilva991.png" width="100px;" alt="Matheus Santos Silva"/><br>
        <sub>
          <b>Matheus Santos Silva</b>
        </sub>
      </a>
      <br>
      <sub>Desenvolvedor Full Stack</sub>
    </td>
  </tr>
</table>

### 📫 Contato

- GitHub: [@matheusssilva991](https://github.com/matheusssilva991)
- LinkedIn: [Matheus Santos Silva](https://linkedin.com/in/matheusssilva991)
- Issues: [Relatar problema](https://github.com/matheusssilva991/tarefa28_IA4_embarcatech/issues)

---

## 🎓 Programa Embarcatech

Este projeto foi desenvolvido como parte do **Programa Embarcatech**, uma iniciativa da Softex focada em capacitação em sistemas embarcados e IoT.

- 🌐 Website: [embarcatech.softex.br](https://embarcatech.softex.br/)
- 📚 Conteúdo: Sistemas embarcados, IoT, Machine Learning
- 🎯 Objetivo: Formação de profissionais em tecnologias embarcadas

---

## 📖 Referências

- [Raspberry Pi Pico SDK Documentation](https://raspberrypi.github.io/pico-sdk-doxygen/)
- [TensorFlow Lite Micro](https://www.tensorflow.org/lite/microcontrollers)
- [MNIST Database](http://yann.lecun.com/exdb/mnist/)
- [RP2040 Datasheet](https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf)
- [Pico Datasheet](https://datasheets.raspberrypi.com/pico/pico-datasheet.pdf)

---

<p align="center">
  <b>Desenvolvido com ❤️ para o Programa Embarcatech</b>
  <br>
  <sub>Reconhecimento de Dígitos MNIST com CNN no Raspberry Pi Pico W</sub>
</p>

<p align="center">
  ⭐ Se este projeto foi útil, considere dar uma estrela no repositório!
</p>
