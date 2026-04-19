import numpy as np
from PIL import Image
import tensorflow as tf
from tensorflow.keras.models import load_model

# Configurações
MODEL_PATH = "../models//mobilenet_model.keras"
IMG_SIZE = (224, 224)
CLASS_NAMES = [
    'Battery', 'Keyboard', 'Microwave', 'Mobile', 'Mouse',
    'PCB', 'Player', 'Printer', 'Television', 'Washing Machine',
    'cardboard', 'glass', 'metal', 'organic', 'paper', 'plastic', 'trash'
]

class WasteClassifierKeras:
    def __init__(self, model_path):
        """Inicializa o classificador com modelo Keras"""
        print("Carregando modelo Keras...")
        self.model = load_model(model_path)
        print(f"Modelo carregado: {model_path}")
        print(f"Input shape: {self.model.input_shape}")
        print(f"Output shape: {self.model.output_shape}")
        self.model.summary()

    def preprocess_image(self, image_path):
        """Pré-processa a imagem"""
        # Carregar e redimensionar
        img = Image.open(image_path).convert('RGB')
        img = img.resize(IMG_SIZE)

        # Converter para array e normalizar [0, 1]
        img_array = np.array(img, dtype=np.float32) / 255.0
        img_array = np.expand_dims(img_array, axis=0)

        return img_array

    def predict(self, image_path):
        """Realiza a predição"""
        # Pré-processar imagem
        img = self.preprocess_image(image_path)

        # Inferência
        output = self.model.predict(img, verbose=0)

        # Obter classe predita e probabilidades
        pred_idx = int(np.argmax(output[0]))
        pred_class = CLASS_NAMES[pred_idx]
        confidence = float(np.max(output[0]))

        return pred_class, confidence, output[0]

    def predict_batch(self, image_paths):
        """Realiza predição em múltiplas imagens"""
        images = []
        for path in image_paths:
            img = self.preprocess_image(path)
            images.append(img[0])

        images = np.array(images)
        outputs = self.model.predict(images, verbose=0)

        results = []
        for i, output in enumerate(outputs):
            pred_idx = int(np.argmax(output))
            pred_class = CLASS_NAMES[pred_idx]
            confidence = float(np.max(output))
            results.append({
                'image': image_paths[i],
                'class': pred_class,
                'confidence': confidence,
                'probabilities': output
            })

        return results

# Exemplo de uso
if __name__ == "__main__":
    print(f"TensorFlow version: {tf.__version__}")
    print(f"GPU disponível: {tf.config.list_physical_devices('GPU')}\n")

    # Inicializar classificador
    classifier = WasteClassifierKeras(model_path=MODEL_PATH)

    # Fazer predição única
    print("\n" + "="*60)
    print("PREDIÇÃO ÚNICA")
    print("="*60)

    base_image_path = "../data/samples/"
    image_path = f"{base_image_path}bateria.jpeg"
    pred_class, confidence, probabilities = classifier.predict(image_path)

    print(f"\nImagem: {image_path}")
    print(f"Classe predita: {pred_class}")
    print(f"Confiança: {confidence:.2%}")
    print("="*60)

    # Mostrar top 5 predições
    print("\nTop 5 predições:")
    top_indices = np.argsort(probabilities)[-5:][::-1]
    for idx in top_indices:
        print(f"  {CLASS_NAMES[idx]:.<30} {probabilities[idx]:.2%}")

    # Exemplo com múltiplas imagens
    print("\n" + "="*60)
    print("PREDIÇÃO EM LOTE")
    print("="*60)

    image_paths = [f"{base_image_path}bateria.jpeg", f"{base_image_path}mouse.jpeg",
                   f"{base_image_path}monitor.jpeg", f"{base_image_path}mouse2.jpeg",
                   f"{base_image_path}mouse3.jpeg", f"{base_image_path}teclado.jpeg",
                   f"{base_image_path}teclado2.jpeg"]  # Adicione mais imagens aqui
    results = classifier.predict_batch(image_paths)

    for result in results:
        print(f"\nImagem: {result['image']}")
        print(f"Classe: {result['class']}")
        print(f"Confiança: {result['confidence']:.2%}")