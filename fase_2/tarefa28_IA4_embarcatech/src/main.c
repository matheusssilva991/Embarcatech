/**
 * @file main.c
 * @brief Sistema de inferência CNN MNIST com comunicação serial no Raspberry Pi Pico W
 *
 * Este programa implementa um sistema de reconhecimento de dígitos MNIST usando uma
 * Rede Neural Convolucional (CNN) quantizada em INT8 executada no RP2040 com
 * TensorFlow Lite Micro.
 *
 * Funcionalidades:
 *  - Recebe 784 bytes via USB-serial representando uma imagem 28x28 do MNIST
 *  - Pré-processa e quantiza os dados para o formato INT8 exigido pelo modelo
 *  - Executa inferência usando o modelo CNN treinado
 *  - Retorna via serial: dígito predito (0-9) e scores de todas as classes
 *
 * Protocolo Serial:
 *  - Entrada: "START\n" seguido de 784 bytes (valores 0-255) e "END\n"
 *  - Saída: JSON com resultado da predição
 *
 * @note Modelo quantizado INT8 localizado em models/mnist_cnn_int8_model.h
 * @note Comunicação USB-serial configurada para 115200 baud
 */

// Bibliotecas padrão
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/gpio.h"
#include "lib/ssd1306/ssd1306.h"
#include "lib/ssd1306/display.h"
#include "lib/ws2812b/ws2812b.h"
#include "lib/button/button.h"

// Headers do projeto
#include "tflm_wrapper.h"

// Buffer para receber dados via serial (28x28 = 784 pixels)
#define MNIST_SIZE 784
#define LED_DISPLAY_TIME_US 2000000 // 2 segundos em microsegundos
static uint8_t received_image[MNIST_SIZE];
static volatile int8_t last_displayed_digit = -1;         // Último dígito exibido (-1 = nenhum)
static volatile absolute_time_t last_display_update_time; // Tempo da última atualização

// PROTÓTIPOS DE FUNÇÕES
static void btn_b_irq_callback(uint gpio, uint32_t events);
static int argmax_i8(const int8_t *v, int n);
static int8_t quantize_f32_to_i8(float x, float scale, int zp);
static int read_line(char *buffer, int max_len);
static bool receive_mnist_image(uint8_t *image_buffer);
ssd1306_t display;

int main()
{
    // Inicialização do stdio (USB-serial)
    stdio_init_all();

    // Inicializar botões
    init_btn(BTN_B_PIN);

    // Configurar interrupção para o botão B (modo BOOTSEL)
    gpio_set_irq_enabled_with_callback(BTN_B_PIN, GPIO_IRQ_EDGE_FALL, true, &btn_b_irq_callback);

    // Inicializar periféricos
    ws2812b_init();
    init_display(&display);

    // Inicializar tempo de atualização da matriz de LEDs
    last_display_update_time = get_absolute_time();

    // Aguardar estabelecimento da conexão serial
    sleep_ms(2000);

    printf("\n========================================\n");
    printf("  MNIST CNN INT8 - Raspberry Pi Pico W\n");
    printf("  TensorFlow Lite Micro\n");
    printf("========================================\n");
    printf("  Pressione Botão B para modo BOOTSEL  \n");
    printf("========================================\n\n");

    // Inicialização do TensorFlow Lite Micro
    printf("Inicializando TensorFlow Lite Micro...\n");
    int rc = tflm_init();
    if (rc != 0)
    {
        printf("ERRO: tflm_init() falhou com código: %d\n", rc);
        printf("Códigos de erro:\n");
        printf("  1 - Falha ao carregar modelo\n");
        printf("  2 - Versão do schema incompatível\n");
        printf("  3 - Falha na alocação de tensores\n");
        printf("  4 - Ponteiros de entrada/saída inválidos\n");
        printf("  5 - Tipo de entrada não é INT8\n");
        printf("  6 - Tipo de saída não é INT8\n");
        while (1)
            tight_loop_contents();
    }

    printf("TensorFlow Lite Micro inicializado com sucesso!\n");
    printf("Memória da arena utilizada: %d bytes\n\n", tflm_arena_used_bytes());

    // Obter ponteiros para os tensores de entrada e saída
    int in_bytes = 0;
    int8_t *in = tflm_input_ptr(&in_bytes);

    int out_bytes = 0;
    int8_t *out = tflm_output_ptr(&out_bytes);

    // Verificação básica dos ponteiros
    if (!in || !out)
    {
        printf("ERRO: Ponteiros de entrada/saída nulos\n");
        while (1)
            tight_loop_contents();
    }

    printf("Configuração dos tensores:\n");
    printf("  Entrada:  %d bytes (esperado 784 para 28x28)\n", in_bytes);
    printf("  Saída:    %d bytes (esperado 10 para classes 0-9)\n\n", out_bytes);

    // Obter parâmetros de quantização
    float in_scale = tflm_input_scale();
    int in_zp = tflm_input_zero_point();
    float out_scale = tflm_output_scale();
    int out_zp = tflm_output_zero_point();

    printf("Parâmetros de quantização:\n");
    printf("  Entrada:  scale=%.6f, zero_point=%d\n", in_scale, in_zp);
    printf("  Saída:    scale=%.6f, zero_point=%d\n\n", out_scale, out_zp);

    // Validar tamanho do buffer de entrada
    if (in_bytes < MNIST_SIZE)
    {
        printf("ERRO: Buffer de entrada muito pequeno (%d < %d)\n", in_bytes, MNIST_SIZE);
        while (1)
            tight_loop_contents();
    }

    printf("Sistema pronto! Protocolo de comunicação:\n");
    printf("  1. Envie 'START'\n");
    printf("  2. Envie 784 bytes (valores 0-255)\n");
    printf("  3. Envie 'END'\n");
    printf("========================================\n\n");

    // Loop principal: receber dados e processar inferências
    while (true)
    {
        // Receber imagem MNIST via serial
        // (A verificação de timeout está dentro da função receive_mnist_image)
        if (!receive_mnist_image(received_image))
        {
            printf("Falha ao receber imagem. Reiniciando...\n\n");
            continue;
        }

        // Pré-processar e quantizar os dados
        printf("Quantizando dados de entrada...\n");
        for (int i = 0; i < MNIST_SIZE; i++)
        {
            // Normalizar pixel: 0-255 -> 0.0-1.0 (igual ao treinamento)
            float normalized = (float)received_image[i] / 255.0f;
            // Quantizar para INT8
            in[i] = quantize_f32_to_i8(normalized, in_scale, in_zp);
        }

        // Executar inferência
        printf("Executando inferência...\n");
        rc = tflm_invoke();
        if (rc != 0)
        {
            printf("ERRO: Inferência falhou com código: %d\n\n", rc);
            continue;
        }

        // Processar resultado
        int predicted_digit = argmax_i8(out, 10);

        // Exibir dígito previsto na matriz de LEDs
        printf("Exibindo dígito %d na matriz de LEDs...\n", predicted_digit);
        ws2812b_draw_number((uint8_t)predicted_digit);

        // Atualizar controle de tempo e dígito exibido
        last_displayed_digit = predicted_digit;
        last_display_update_time = get_absolute_time();

        // Enviar resultado via serial em formato JSON
        printf("\n--- RESULTADO ---\n");
        printf("{\n");
        printf("  \"predicted_digit\": %d,\n", predicted_digit);
        printf("  \"scores\": [\n");

        for (int c = 0; c < 10; c++)
        {
            // Dequantizar score: float = (int8 - zero_point) * scale
            int8_t q = out[c];
            float score = (float)(q - out_zp) * out_scale;

            printf("    {\"class\": %d, \"score\": %.6f}", c, score);
            if (c < 9)
                printf(",");
            printf("\n");
        }

        printf("  ]\n");
        printf("}\n");
        printf("--- FIM RESULTADO ---\n\n");

        // Pequeno delay antes de processar próxima requisição
        sleep_ms(100);
    }

    // Nunca deve chegar aqui
    return 0;
}

// Retorna o índice do maior valor em um array int8_t
static int argmax_i8(const int8_t *v, int n)
{
    int best = 0;
    int8_t bestv = v[0];
    for (int i = 1; i < n; i++)
    {
        if (v[i] > bestv)
        {
            bestv = v[i];
            best = i;
        }
    }
    return best;
}

// Quantiza um valor float para int8_t usando escala e ponto zero
static int8_t quantize_f32_to_i8(float x, float scale, int zp)
{
    // q = round(x/scale) + zp
    long q = lroundf(x / scale) + zp;
    if (q < -128)
        q = -128;
    if (q > 127)
        q = 127;
    return (int8_t)q;
}

// Lê uma linha da entrada serial com timeout
static int read_line(char *buffer, int max_len)
{
    int idx = 0;
    while (idx < max_len - 1)
    {
        int c = getchar_timeout_us(100000); // Timeout de 100ms
        if (c == PICO_ERROR_TIMEOUT)
            return idx; // Retorna o que foi lido até agora
        if (c < 0)
            break;

        if (c == '\n' || c == '\r')
        {
            break;
        }
        buffer[idx++] = (char)c;
    }
    buffer[idx] = '\0';
    return idx;
}

// Recebe uma imagem MNIST via serial (784 bytes entre "START" e "END")
static bool receive_mnist_image(uint8_t *image_buffer)
{
    char line[32];

    // Aguardar comando START
    printf("Aguardando dados... Envie 'START' para iniciar.\n");
    while (true)
    {
        // Verificar timeout da matriz de LEDs durante a espera
        int64_t time_since_update = absolute_time_diff_us(last_display_update_time, get_absolute_time());
        if (last_displayed_digit >= 0 && time_since_update >= LED_DISPLAY_TIME_US)
        {
            // Limpar matriz após 2 segundos
            ws2812b_clear();
            ws2812b_write();
            last_displayed_digit = -1;
            printf("Matriz de LEDs limpa após 2 segundos.\n");
        }

        int len = read_line(line, sizeof(line));
        if (len > 0 && strcmp(line, "START") == 0)
        {
            printf("Recebendo imagem...\n");
            break;
        }

        // Pequeno delay para não consumir 100% da CPU
        sleep_ms(50);
    }

    // Receber 784 bytes
    int received = 0;
    absolute_time_t start_time = get_absolute_time();
    while (received < MNIST_SIZE)
    {
        int c = getchar_timeout_us(100000); // Timeout de 100ms
        if (c == PICO_ERROR_TIMEOUT)
        {
            // Verificar timeout total (30 segundos)
            if (absolute_time_diff_us(start_time, get_absolute_time()) > 30000000)
            {
                printf("ERRO: Timeout ao receber dados\n");
                return false;
            }
            continue;
        }
        if (c < 0)
            break;

        image_buffer[received++] = (uint8_t)c;
    }

    if (received != MNIST_SIZE)
    {
        printf("ERRO: Recebidos %d bytes, esperado %d\n", received, MNIST_SIZE);
        return false;
    }

    // Aguardar comando END
    read_line(line, sizeof(line));
    if (strcmp(line, "END") != 0)
    {
        printf("AVISO: Esperado 'END', recebido '%s'\n", line);
    }

    printf("Imagem recebida com sucesso (%d bytes)\n", received);
    return true;
}

// Callback de interrupção do botão B - Ativa modo BOOTSEL
static void btn_b_irq_callback(uint gpio, uint32_t events)
{
    // Verificar se é o botão B e se foi pressionado (borda de descida)
    if (gpio == BTN_B_PIN && (events & GPIO_IRQ_EDGE_FALL))
    {
        // Pequeno debounce por software
        busy_wait_us(50000); // 50ms usando busy_wait ao invés de sleep (mais seguro em IRQ)

        // Verificar se o botão ainda está pressionado (LOW)
        if (!gpio_get(BTN_B_PIN))
        {
            // Desabilitar interrupções para evitar chamadas múltiplas
            gpio_set_irq_enabled(BTN_B_PIN, GPIO_IRQ_EDGE_FALL, false);

            printf("\n\n*** BOTÃO B PRESSIONADO ***\n");
            printf("Entrando em modo BOOTSEL...\n");
            busy_wait_us(100000); // 100ms para mensagem ser enviada

            // Reiniciar em modo BOOTSEL
            reset_usb_boot(0, 0);
        }
    }
}
