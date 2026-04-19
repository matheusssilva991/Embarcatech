# 🗑️ Classificação de Resíduos com Deep Learning

Um projeto de classificação de resíduos utilizando redes neurais convolucionais (CNN) com MobileNetV2 e TensorFlow Lite para aplicações embarcadas e IoT.

## 📋 Sobre o Projeto

Este projeto implementa um sistema de classificação automática de resíduos utilizando deep learning. O modelo é capaz de identificar diferentes tipos de materiais e componentes eletrônicos, auxiliando em processos de reciclagem e descarte adequado de lixo eletrônico.

### 🎯 Características Principais

- **Transfer Learning** com MobileNetV2 pré-treinado no ImageNet
- **Modelos otimizados** para dispositivos embarcados (Float32 e Int8)
- **Visualizações completas** de treinamento e resultados
- **Alta acurácia** na classificação de múltiplas categorias
- **Pronto para TinyML** com quantização Int8

### 📦 Categorias de Classificação

O modelo classifica resíduos em múltiplas categorias, incluindo:

**Materiais Recicláveis:**

- Cardboard (Papelão)
- Glass (Vidro)
- Metal
- Paper (Papel)
- Plastic (Plástico)

**Componentes Eletrônicos:**

- Battery (Bateria)
- Keyboard (Teclado)
- Microwave (Micro-ondas)
- Mobile (Celular)
- Mouse
- PCB (Placa de Circuito)
- Player
- Printer (Impressora)
- Television (Televisão)
- Washing Machine (Máquina de Lavar)

**Resíduos Gerais:**

- Organic (Orgânico)
- Trash (Lixo Geral)

## 🗂️ Estrutura do Projeto

```
tarefa24_ia_embarcatech/
├── data/
│   ├── balanced_waste_images/           # Dataset original
│   └── split_data/                      # Dataset dividido
│       ├── train/                       # 80% para treinamento
│       ├── val/                         # 10% para validação
│       └── test/                        # 10% para teste
├── models/
│   ├── mobilenet_weights.weights.h5     # Pesos do modelo treinado
│   ├── mobilenet_model.keras            # Modelo Keras completo
│   ├── training_history.pkl             # Histórico de treinamento
│   ├── waste_classification_float32.tflite  # Modelo TFLite Float32
│   └── waste_classification_int8.tflite     # Modelo TFLite Int8 quantizado
├── src/
│   ├── main.ipynb                       # Notebook principal
│   └── utils/
│       └── plots.py                     # Funções de visualização
├── pyproject.toml                       # Dependências do projeto
└── README.md
```

## 🚀 Começando

### Pré-requisitos

- Python 3.8+
- pip ou poetry para gerenciamento de dependências
- GPU (recomendado para treinamento, mas não obrigatório)

### 📥 Instalação

1. **Clone o repositório:**

```bash
git clone https://github.com/matheusssilva991/tarefa24_ia_embarcatech.git
cd tarefa24_ia_embarcatech
```

2. **Instale as dependências:**

```bash
# Usando pip
pip install -e .

# Ou usando poetry (recomendado)
poetry install
poetry shell
```

### 📊 Dataset

O dataset utilizado é o **Waste Classification Dataset** do Kaggle:

🔗 [Download do Dataset](https://www.kaggle.com/datasets/kaanerkez/waste-classfication-dataset/data)

**Instruções:**

1. Baixe o dataset do Kaggle
2. Extraia para `data/balanced_waste_images/`
3. Execute o notebook para dividir automaticamente em train/val/test

## 💻 Uso

### 🔧 Treinamento do Modelo

1. **Abra o notebook principal:**

```bash
jupyter notebook src/main.ipynb
# ou
code src/main.ipynb  # No VS Code
```

2. **Execute as células sequencialmente:**
   - Carregamento e preparação dos dados
   - Criação do modelo MobileNetV2
   - Treinamento (ou carregamento de pesos existentes)
   - Avaliação e métricas
   - Conversão para TensorFlow Lite

3. **O treinamento automático:**
   - Verifica se já existem pesos salvos
   - Se existir: carrega os pesos e histórico
   - Se não existir: treina um novo modelo

### 📈 Visualizações Disponíveis

O projeto inclui funções de visualização em `utils/plots.py`:

- **`plot_sample_images()`** - Visualiza amostras de cada classe
- **`plot_training_history()`** - Gráficos de acurácia e loss
- **`plot_confusion_matrix()`** - Matriz de confusão
- **`plot_image()`** - Exibe imagem com predição

### 🔍 Inferência

#### Com Modelo Keras

```python
import numpy as np
from tensorflow.keras.models import load_model

model = load_model('models/mobilenet_model.keras')
prediction = model.predict(image)
class_idx = np.argmax(prediction)
```

#### Com TensorFlow Lite Float32

```python
import tensorflow as tf

interpreter = tf.lite.Interpreter(
    model_path='models/waste_classification_float32.tflite'
)
interpreter.allocate_tensors()

input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

interpreter.set_tensor(input_details[0]['index'], input_data)
interpreter.invoke()
output = interpreter.get_tensor(output_details[0]['index'])
```

#### Com TensorFlow Lite Int8 (TinyML)

```python
# Quantizar entrada
input_scale, input_zero_point = input_details[0]['quantization']
input_int8 = (image / input_scale + input_zero_point).astype(np.int8)

# Inferência
interpreter.set_tensor(input_details[0]['index'], input_int8)
interpreter.invoke()
output_int8 = interpreter.get_tensor(output_details[0]['index'])

# Desquantizar saída
output_scale, output_zero_point = output_details[0]['quantization']
output = (output_int8 - output_zero_point) * output_scale
```

## 🎓 Arquitetura do Modelo

### Base Model: MobileNetV2

- **Input Shape:** 224x224x3
- **Pesos:** ImageNet (pré-treinado)
- **Feature Extractor:** Congelado durante treinamento

### Camadas Customizadas

```
GlobalAveragePooling2D()
Dense(256, activation='relu')
Dropout(0.3)
Dense(num_classes, activation='softmax')
```

### Hiperparâmetros

- **Optimizer:** Adam (lr=0.001)
- **Loss:** Categorical Crossentropy
- **Batch Size:** 32
- **Epochs:** 13
- **Split:** 80% train / 10% val / 10% test

## 📊 Resultados

### Métricas de Performance

- **Acurácia no conjunto de teste:** ~XX%
- **Precision, Recall e F1-Score** por classe disponíveis no notebook

### Tamanho dos Modelos

- **Modelo Keras completo:** ~XX MB
- **TFLite Float32:** ~XX MB
- **TFLite Int8:** ~XX MB (otimizado para embarcados)

## 🛠️ Tecnologias Utilizadas

- **TensorFlow 2.x** - Framework de deep learning
- **Keras** - API de alto nível para redes neurais
- **MobileNetV2** - Arquitetura eficiente de CNN
- **TensorFlow Lite** - Modelos para dispositivos embarcados
- **scikit-learn** - Métricas e avaliação
- **NumPy & Pandas** - Manipulação de dados
- **Matplotlib & Seaborn** - Visualizações

## 🎯 Aplicações Práticas

- **IoT e Sistemas Embarcados** - Classificação em tempo real
- **Lixeiras Inteligentes** - Separação automática de resíduos
- **Reciclagem Industrial** - Triagem de materiais
- **Educação Ambiental** - Apps de identificação de resíduos
- **Gestão de Resíduos Eletrônicos** - Identificação de e-waste

## 📝 Licença

Este projeto é parte da **Tarefa 24 - IA Embarcatech** e está disponível para fins educacionais.

## 👤 Autor

**Matheus**

- GitHub: [@seu-usuario](https://github.com/seu-usuario)

## 🤝 Contribuindo

Contribuições são bem-vindas! Sinta-se à vontade para:

1. Fork o projeto
2. Criar uma branch para sua feature (`git checkout -b feature/NovaFeature`)
3. Commit suas mudanças (`git commit -m 'Adiciona nova feature'`)
4. Push para a branch (`git push origin feature/NovaFeature`)
5. Abrir um Pull Request

## 📚 Referências

- [MobileNetV2 Paper](https://arxiv.org/abs/1801.04381)
- [TensorFlow Lite Documentation](https://www.tensorflow.org/lite)
- [Waste Classification Dataset](https://www.kaggle.com/datasets/kaanerkez/waste-classfication-dataset)
- [Transfer Learning Guide](https://www.tensorflow.org/tutorials/images/transfer_learning)

---

⭐ Se este projeto foi útil, considere dar uma estrela no repositório!
