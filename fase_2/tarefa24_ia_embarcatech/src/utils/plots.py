import numpy as np
import matplotlib.pyplot as plt


def plot_sample_images(generator, class_names, n=8):
    """Visualiza amostras do dataset em linhas de até 6 imagens"""
    images, labels = next(generator)
    max_per_row = 6
    rows = (n + max_per_row - 1) // max_per_row
    plt.figure(figsize=(max_per_row * 3, rows * 3))
    for i in range(n):
        row = i // max_per_row
        col = i % max_per_row
        plt.subplot(rows, max_per_row, i + 1)
        img = (images[i] * 255).astype("uint8")
        plt.imshow(img)
        label_idx = np.argmax(labels[i])
        plt.title(class_names[label_idx])
        plt.axis("off")
    plt.tight_layout()
    plt.show()


def plot_training_history(history):
    """Plota o histórico de treinamento do modelo com gráficos empilhados"""
    acc = history.history['accuracy']
    val_acc = history.history['val_accuracy']
    loss = history.history['loss']
    val_loss = history.history['val_loss']
    epochs_range = range(1, len(acc) + 1)

    plt.figure(figsize=(10, 10))

    # Gráfico de Acurácia
    plt.subplot(2, 1, 1)
    plt.plot(epochs_range, acc, 'o-', label='Acurácia de Treino', linewidth=2, markersize=6)
    plt.plot(epochs_range, val_acc, 's-', label='Acurácia de Validação', linewidth=2, markersize=6)
    plt.xlabel('Épocas', fontsize=12)
    plt.ylabel('Acurácia', fontsize=12)
    plt.title('Acurácia de Treino e Validação', fontsize=14, fontweight='bold')
    plt.legend(loc='lower right', fontsize=10)
    plt.grid(True, alpha=0.3)

    # Gráfico de Perda
    plt.subplot(2, 1, 2)
    plt.plot(epochs_range, loss, 'o-', label='Perda de Treino', linewidth=2, markersize=6)
    plt.plot(epochs_range, val_loss, 's-', label='Perda de Validação', linewidth=2, markersize=6)
    plt.xlabel('Épocas', fontsize=12)
    plt.ylabel('Perda', fontsize=12)
    plt.title('Perda de Treino e Validação', fontsize=14, fontweight='bold')
    plt.legend(loc='upper right', fontsize=10)
    plt.grid(True, alpha=0.3)

    plt.tight_layout()
    plt.show()


def plot_confusion_matrix(cm, class_names):
    """Plota a matriz de confusão"""
    plt.figure(figsize=(8, 6))
    plt.imshow(cm, interpolation='nearest', cmap=plt.cm.Blues)
    plt.title('Matriz de Confusão', fontsize=16, fontweight='bold')
    plt.colorbar()
    tick_marks = np.arange(len(class_names))
    plt.xticks(tick_marks, class_names, rotation=45, fontsize=10)
    plt.yticks(tick_marks, class_names, fontsize=10)

    thresh = cm.max() / 2.
    for i, j in np.ndindex(cm.shape):
        plt.text(j, i, format(cm[i, j], 'd'),
                 horizontalalignment="center",
                 color="white" if cm[i, j] > thresh else "black",
                 fontsize=12)

    plt.ylabel('Rótulo Verdadeiro', fontsize=12)
    plt.xlabel('Rótulo Previsto', fontsize=12)
    plt.tight_layout()
    plt.show()


def plot_image(img, title):
    plt.figure(figsize=(4,4))
    plt.imshow(img)
    plt.title(title)
    plt.axis("off")
    plt.show()
