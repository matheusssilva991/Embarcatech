#!/usr/bin/env python3
"""
Script para testar a comunicação serial com o Raspberry Pi Pico W
executando o modelo CNN MNIST.

Uso:
    python test_serial_mnist.py /dev/ttyACM0 samples/mnist_sample_00.h

Argumentos:
    porta_serial: Porta serial do Pico (ex: /dev/ttyACM0 no Linux, COM3 no Windows)
    arquivo_sample: Caminho para o arquivo .h contendo a amostra MNIST

O script:
    1. Lê os dados do arquivo de amostra .h
    2. Envia via serial para o Pico no protocolo esperado
    3. Recebe e exibe o resultado da inferência
"""

import serial
import time
import sys
import re
import json


def parse_mnist_sample(file_path):
    """
    Extrai os dados de pixel e label de um arquivo de amostra MNIST (.h).

    Args:
        file_path: Caminho para o arquivo .h

    Returns:
        tuple: (label, pixels) onde pixels é uma lista de 784 valores uint8
    """
    with open(file_path, "r") as f:
        content = f.read()

    # Extrair label
    label_match = re.search(r"mnist_sample_label\s*=\s*(\d+)", content)
    if not label_match:
        raise ValueError("Label não encontrado no arquivo")
    label = int(label_match.group(1))

    # Extrair array de pixels
    array_match = re.search(
        r"mnist_sample_28x28\[.*?\]\s*=\s*\{([^}]+)\}", content, re.DOTALL
    )
    if not array_match:
        raise ValueError("Array de pixels não encontrado no arquivo")

    # Parsear valores
    pixel_str = array_match.group(1)
    pixels = [int(x.strip()) for x in pixel_str.split(",") if x.strip()]

    if len(pixels) != 784:
        raise ValueError(f"Esperado 784 pixels, encontrado {len(pixels)}")

    return label, pixels


def send_mnist_to_pico(serial_port, pixels):
    """
    Envia uma imagem MNIST para o Pico via serial.

    Args:
        serial_port: Objeto serial.Serial
        pixels: Lista de 784 valores uint8
    """
    print("Enviando comando START...")
    serial_port.write(b"START\n")
    serial_port.flush()
    time.sleep(0.1)

    print("Enviando 784 bytes de pixel...")
    data = bytes(pixels)
    serial_port.write(data)
    serial_port.flush()
    time.sleep(0.1)

    print("Enviando comando END...")
    serial_port.write(b"END\n")
    serial_port.flush()


def receive_result(serial_port, timeout=10):
    """
    Recebe o resultado da inferência do Pico.

    Args:
        serial_port: Objeto serial.Serial
        timeout: Timeout em segundos

    Returns:
        dict: Resultado parseado em formato JSON
    """
    print("\nAguardando resultado...")
    start_time = time.time()
    result_lines = []
    in_result = False

    while time.time() - start_time < timeout:
        try:
            if serial_port.in_waiting > 0:
                line = serial_port.readline().decode("utf-8", errors="ignore").strip()
                print(f"< {line}")

                if "--- RESULTADO ---" in line:
                    in_result = True
                    result_lines = []
                elif "--- FIM RESULTADO ---" in line:
                    break
                elif in_result:
                    result_lines.append(line)
            else:
                time.sleep(0.1)  # Pequena pausa se não há dados
        except serial.SerialException as e:
            print(f"\n✗ Erro de comunicação serial: {e}")
            print("O dispositivo pode ter sido desconectado.")
            return None
        except Exception as e:
            print(f"\n✗ Erro inesperado: {e}")
            return None

    # Parsear JSON
    if result_lines:
        try:
            json_str = "\n".join(result_lines)
            result = json.loads(json_str)
            return result
        except json.JSONDecodeError as e:
            print(f"Erro ao parsear JSON: {e}")
            print("Conteúdo recebido:", json_str)
            return None

    return None


def main():
    if len(sys.argv) < 3:
        print("Uso: python test_serial_mnist.py <porta_serial> <arquivo_sample>")
        print(
            "Exemplo: python test_serial_mnist.py /dev/ttyACM0 samples/mnist_sample_00.h"
        )
        sys.exit(1)

    port = sys.argv[1]
    sample_file = sys.argv[2]

    print("=" * 60)
    print("  Teste de Inferência MNIST via Serial")
    print("=" * 60)
    print(f"Porta serial: {port}")
    print(f"Arquivo de amostra: {sample_file}")
    print()

    # Ler amostra
    print("Carregando amostra MNIST...")
    try:
        label, pixels = parse_mnist_sample(sample_file)
        print(f"✓ Label esperado: {label}")
        print(f"✓ Pixels carregados: {len(pixels)}")
        print()
    except Exception as e:
        print(f"✗ Erro ao carregar amostra: {e}")
        sys.exit(1)

    # Conectar à porta serial
    print(f"Conectando à porta serial {port}...")
    try:
        ser = serial.Serial(port, 115200, timeout=1)
        time.sleep(2)  # Aguardar inicialização
        print("✓ Conectado")
        print()
    except Exception as e:
        print(f"✗ Erro ao conectar: {e}")
        sys.exit(1)

    try:
        # Ler mensagens iniciais do Pico
        print("Mensagens do Pico:")
        print("-" * 60)
        time.sleep(1)
        try:
            while ser.in_waiting > 0:
                line = ser.readline().decode("utf-8", errors="ignore").strip()
                if line:
                    print(f"< {line}")
        except Exception as e:
            print(f"Aviso: erro ao ler mensagens iniciais: {e}")
        print("-" * 60)
        print()

        # Enviar dados
        try:
            send_mnist_to_pico(ser, pixels)
        except serial.SerialException as e:
            print(f"\n✗ Erro ao enviar dados: {e}")
            print("Verifique se o Pico está conectado e programado corretamente.")
            return

        # Receber resultado
        result = receive_result(ser, timeout=15)

        if result:
            print("\n" + "=" * 60)
            print("  RESULTADO DA INFERÊNCIA")
            print("=" * 60)
            print(f"Label esperado:     {label}")
            print(f"Dígito predito:     {result['predicted_digit']}")
            print(
                f"Correto:            {'✓ SIM' if result['predicted_digit'] == label else '✗ NÃO'}"
            )
            print()
            print("Scores por classe:")
            for score_info in result["scores"]:
                c = score_info["class"]
                score = score_info["score"]
                bar = "█" * int(score * 50) if score > 0 else ""
                marker = " ← PREDIÇÃO" if c == result["predicted_digit"] else ""
                print(f"  {c}: {score:7.4f} {bar}{marker}")
            print("=" * 60)
        else:
            print("\n✗ Não foi possível obter o resultado")
            print("\nDicas de troubleshooting:")
            print("  - Verifique se o Pico está executando o programa main.c")
            print("  - Tente desconectar e reconectar o Pico")
            print("  - Verifique se nenhum outro programa está usando a porta serial")
            print(
                "  - Tente usar um monitor serial (minicom, screen) para verificar as mensagens"
            )

    except KeyboardInterrupt:
        print("\n\nInterrompido pelo usuário.")
    except Exception as e:
        print(f"\n✗ Erro inesperado: {e}")
    finally:
        if ser.is_open:
            ser.close()
        print("\nConexão serial fechada.")


if __name__ == "__main__":
    main()
