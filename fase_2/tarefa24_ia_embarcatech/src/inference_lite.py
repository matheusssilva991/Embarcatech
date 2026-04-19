import numpy as np
import cv2
from PIL import Image
try:
    import tflite_runtime.interpreter as tflite
except ImportError:
    import tensorflow.lite as tflite

# Configurações
MODEL_PATH = "models/waste_classification_int8.tflite"  # Use int8 para melhor performance
IMG_SIZE = (224, 224)
CLASS_NAMES = [
    'Battery', 'Keyboard', 'Microwave', 'Mobile', 'Mouse',
    'PCB', 'Player', 'Printer', 'Television', 'Washing Machine',
    'cardboard', 'glass', 'metal', 'organic', 'paper', 'plastic', 'trash'
]

class WasteClassifier:
    def __init__(self, model_path, use_int8=True):
        """Inicializa o classificador"""
        self.interpreter = tflite.Interpreter(model_path=model_path)
        self.interpreter.allocate_tensors()

        self.input_details = self.interpreter.get_input_details()
        self.output_details = self.interpreter.get_output_details()
        self.use_int8 = use_int8

        # Parâmetros de quantização (para int8)
        if use_int8:
            self.input_scale, self.input_zero_point = self.input_details[0]['quantization']
            self.output_scale, self.output_zero_point = self.output_details[0]['quantization']

        print(f"Modelo carregado: {model_path}")
        print(f"Input shape: {self.input_details[0]['shape']}")
        print(f"Output shape: {self.output_details[0]['shape']}")

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

        # Quantizar entrada se for int8
        if self.use_int8:
            img = img / self.input_scale + self.input_zero_point
            img = np.clip(img, -128, 127).astype(np.int8)
        else:
            img = img.astype(np.float32)

        # Inferência
        self.interpreter.set_tensor(self.input_details[0]['index'], img)
        self.interpreter.invoke()
        output = self.interpreter.get_tensor(self.output_details[0]['index'])

        # Desquantizar saída se for int8
        if self.use_int8:
            output = (output.astype(np.float32) - self.output_zero_point) * self.output_scale

        # Obter classe predita e probabilidades
        pred_idx = int(np.argmax(output[0]))
        pred_class = CLASS_NAMES[pred_idx]
        confidence = np.max(output[0])

        return pred_class, confidence, output[0]

# Exemplo de uso
if __name__ == "__main__":
    # Inicializar classificador
    classifier = WasteClassifier(
        model_path=MODEL_PATH,
        use_int8=True  # True para int8, False para float32
    )

    # Fazer predição
    image_path = "bateria.jpeg"
    pred_class, confidence, probabilities = classifier.predict(image_path)

    print(f"\n{'='*50}")
    print(f"Imagem: {image_path}")
    print(f"Classe predita: {pred_class}")
    print(f"Confiança: {confidence:.2%}")
    print(f"{'='*50}\n")

    # Mostrar top 3 predições
    print("Top 3 predições:")
    top_indices = np.argsort(probabilities)[-3:][::-1]
    for idx in top_indices:
        print(f"  {CLASS_NAMES[idx]}: {probabilities[idx]:.2%}")