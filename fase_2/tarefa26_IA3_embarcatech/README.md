# TinyML Breast Cancer - Raspberry Pi Pico

Sistema de inferência de Machine Learning em tempo real para classificação de câncer de mama usando TensorFlow Lite Micro no Raspberry Pi Pico.

## 📋 Descrição

Este projeto implementa um modelo de rede neural MLP (Multi-Layer Perceptron) treinado no dataset Breast Cancer Wisconsin para classificar tumores como malignos ou benignos diretamente em um microcontrolador Raspberry Pi Pico. O modelo é otimizado para execução em dispositivos embarcados com recursos limitados usando TensorFlow Lite Micro.

### Características

- **Classificação binária**: Maligno (0) vs Benigno (1)
- **30 features**: Características do tumor extraídas de imagens digitalizadas
- **Inferência em tempo real**: Processamento local no microcontrolador
- **Validação completa**: Matriz de confusão e métricas de acurácia

## 🛠️ Tecnologias Utilizadas

- **Hardware**: Raspberry Pi Pico / Pico W
- **SDK**: Pico SDK 2.2.0
- **Framework ML**: TensorFlow Lite Micro (pico-tflmicro)
- **Linguagens**: C/C++, Python
- **Build System**: CMake + Ninja
- **Treinamento**: TensorFlow/Keras

## 📁 Estrutura do Projeto

```
.
├── main.c                          # Código principal de inferência
├── tflm_wrapper.cpp/h              # Wrapper C para TensorFlow Lite Micro
├── model_data.h                    # Modelo convertido para array C
├── dados_teste_norm.h              # Dados de teste normalizados
├── dados_teste_pca.h               # Dados de teste com PCA
├── modelo_breast_cancer_mlp.keras  # Modelo Keras original
├── main.ipynb                      # Notebook para treinamento
├── CMakeLists.txt                  # Configuração de build
└── lib/
    └── pico-tflmicro/              # TensorFlow Lite Micro para Pico
```

## 🚀 Como Usar

### Pré-requisitos

- Raspberry Pi Pico SDK instalado
- Pico VS Code Extension configurada
- Cabo USB para programação
- Python 3.8+ (para retreinamento do modelo)

### Compilação

1. **Clone o repositório**:

```bash
git clone <url-do-repositorio>
cd tarefa26_IA3_embarcatech
```

1. **Configure o projeto** (automático via Pico VS Code Extension)

2. **Compile o projeto**:
   - Use o comando da paleta: `Raspberry Pi Pico: Compile Project`
   - Ou via terminal:

```bash
~/.pico-sdk/ninja/v1.12.1/ninja -C build
```

### Programação do Pico

**Método 1 - Bootsel Mode** (recomendado):

1. Pressione e segure o botão BOOTSEL no Pico
2. Conecte o USB ao computador
3. Solte o botão
4. Use o comando `Raspberry Pi Pico: Run Project`
5. Ou copie manualmente: `build/main.uf2` para o drive USB mass storage

**Método 2 - Debug Probe**:

```bash
# Usar o task "Flash" da VS Code Extension
```

### Visualização dos Resultados

1. Abra um monitor serial (115200 baud, UART)
2. O sistema irá:
   - Inicializar o modelo TFLite
   - Executar inferência em todas as amostras de teste
   - Exibir detalhes das primeiras 10 amostras
   - Apresentar matriz de confusão
   - Calcular acurácia final

**Exemplo de saída**:

```
=== TinyML Breast Cancer - Validacao ===
Modelo inicializado com sucesso!
Iniciando inferencia em 114 amostras de teste...
Amostra 00 | Real: 1 | Pred: 1 | Prob(0-Mal): 0.012  Prob(1-Ben): 0.988
Amostra 01 | Real: 0 | Pred: 0 | Prob(0-Mal): 0.956  Prob(1-Ben): 0.044
...

=== Matriz de Confusao ===
Legenda: 0 = Maligno, 1 = Benigno

         Pred 0   Pred 1
Real 0       40        3
Real 1        2       69

Acuracia final: 95.61% (109 / 114)
```

## 🧠 Sobre o Modelo

### Arquitetura

- **Input**: 30 features (características do tumor)
- **Hidden Layers**: Camadas densas com ativação ReLU
- **Output**: 2 neurônios com Softmax (probabilidades de cada classe)
- **Formato**: Float32 (conversão para int8 opcional)

### Dataset

**Breast Cancer Wisconsin Dataset**:

- 569 amostras totais
- 30 features por amostra (raio, textura, perímetro, área, etc.)
- 2 classes: Maligno (212) e Benigno (357)
- Split: 80% treino, 20% teste

### Pré-processamento

Os dados são normalizados usando StandardScaler antes da inferência:

```python
scaler = StandardScaler()
X_train_scaled = scaler.fit_transform(X_train)
X_test_scaled = scaler.transform(X_test)
```

## 📊 Treinamento do Modelo

Para retreinar o modelo, use o notebook `main.ipynb`:

1. Instale as dependências:

```bash
pip install tensorflow numpy pandas scikit-learn
```

1. Execute as células do notebook sequencialmente

2. Converta o modelo para TFLite:

```python
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()
```

1. Converta para array C usando `xxd`:

```bash
xxd -i model.tflite > model_data.h
```

## ⚙️ Configuração

### Memória

O projeto usa uma arena de 8KB para tensores:

```cpp
constexpr int kTensorArenaSize = 8 * 1024;
```

Ajuste este valor se necessário para modelos maiores.

### Operações TFLite

Operações registradas no `tflm_wrapper.cpp`:

- FullyConnected
- ReLU
- Softmax
- Reshape

Adicione outras operações se modificar a arquitetura do modelo.

### UART/USB

Configurado em [CMakeLists.txt](CMakeLists.txt#L46):

```cmake
pico_enable_stdio_uart(${PROJECT_NAME} 1)
pico_enable_stdio_usb(${PROJECT_NAME} 0)
```

## 🔧 Troubleshooting

### Erro: "Modelo não inicializa"

- Verifique se `model_data.h` está atualizado
- Confirme que a versão do schema TFLite é compatível
- Aumente `kTensorArenaSize` se necessário

### Acurácia baixa

- Verifique se os dados de teste estão normalizados corretamente
- Confirme que o modelo foi treinado adequadamente
- Valide que as 30 features estão na ordem correta

### Erro de compilação

- Verifique se o Pico SDK está instalado corretamente
- Confirme que a biblioteca pico-tflmicro está no diretório `lib/`
- Limpe e recompile: `rm -rf build && mkdir build`

## 📈 Métricas de Performance

- **Acurácia esperada**: ~95-97%
- **Tempo de inferência**: ~5-10ms por amostra
- **Memória RAM**: ~10KB
- **Memória Flash**: ~150KB

## 🤝 Contribuindo

Contribuições são bem-vindas! Por favor:

1. Fork o projeto
2. Crie uma branch para sua feature (`git checkout -b feature/AmazingFeature`)
3. Commit suas mudanças (`git commit -m 'Add some AmazingFeature'`)
4. Push para a branch (`git push origin feature/AmazingFeature`)
5. Abra um Pull Request

## 📝 Licença

Este projeto faz parte do programa Embarcatech.

## 👥 Autores

- **Matheus Silva** - [@matheusssilva991](https://github.com/matheusssilva991)

## 🙏 Agradecimentos

- Dataset: [UCI Machine Learning Repository - Breast Cancer Wisconsin](https://archive.ics.uci.edu/ml/datasets/Breast+Cancer+Wisconsin+(Diagnostic))
- TensorFlow Lite Micro para Pico: [pico-tflmicro](https://github.com/raspberrypi/pico-tflmicro)
- Raspberry Pi Foundation pelo Pico SDK

## 📚 Referências

- [TensorFlow Lite Micro Documentation](https://www.tensorflow.org/lite/microcontrollers)
- [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk)
- [Breast Cancer Wisconsin Dataset Paper](https://doi.org/10.1016/S0090-8258(99)90021-2)

---

**Nota**: Este projeto é para fins educacionais e de pesquisa. Não deve ser usado para diagnóstico médico real sem validação clínica apropriada.
