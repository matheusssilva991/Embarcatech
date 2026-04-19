#include "ws2812b.h"
#include "ws2812b.pio.h"
#include "led_matrix_numbers.h"

/// Buffer de pixels da matriz de LEDs
ws2812b_LED_t led_matrix[LED_MATRIX_SIZE];

/// Ponteiro para a máquina PIO(0 ou 1) em uso
PIO led_matrix_pio;

/// Número da state machine PIO utilizada
uint sm;

// Inicializa a máquina PIO para controle da matriz de LEDs.
// Tenta usar PIO0, caso não haja state machines disponíveis, utiliza PIO1.
void ws2812b_init()
{
    // Cria programa PIO na memória da instrução
    uint offset = pio_add_program(pio0, &led_matrix_program);
    led_matrix_pio = pio0;

    // Toma posse de uma máquina PIO disponível
    sm = pio_claim_unused_sm(led_matrix_pio, false);
    if (sm < 0)
    {
        // Se nenhuma máquina estiver livre em PIO0, tenta PIO1
        led_matrix_pio = pio1;
        sm = pio_claim_unused_sm(led_matrix_pio, true); // Causa panic se nenhuma estiver disponível
    }

    // Inicia o programa na state machine da PIO
    // Configura o pino de saída e a frequência de clock (800 kHz para WS2812B)
    led_matrix_program_init(led_matrix_pio, sm, offset, LED_MATRIX_PIN, 800000.f);

    // Limpa todos os LEDs (inicializa com preto)
    ws2812b_clear();
}

// Atribui uma cor RGB a um LED no buffer.
// Os dados são armazenados em ordem GRB conforme exigido pelo protocolo WS2812B.
void ws2812b_set_led(const uint index, const int color[3])
{
    led_matrix[index].R = color[0];
    led_matrix[index].G = color[1];
    led_matrix[index].B = color[2];
}

// Limpa todos os LEDs do buffer (define como preto)
void ws2812b_clear()
{
    int black[3] = {0, 0, 0};
    for (uint i = 0; i < LED_MATRIX_SIZE; ++i)
        ws2812b_set_led(i, black);
}

// Escreve os dados do buffer para os LEDs físicos via PIO.
// Envia cada componente de cor (G, R, B) de forma sequencial para cada LED.
void ws2812b_write()
{
    // Escreve cada dado de 8-bits dos componentes de cor (G, R, B) em sequência
    // para cada LED no buffer da máquina PIO.
    for (uint i = 0; i < LED_MATRIX_SIZE; ++i)
    {
        pio_sm_put_blocking(led_matrix_pio, sm, led_matrix[i].G << 24);
        pio_sm_put_blocking(led_matrix_pio, sm, led_matrix[i].R << 24);
        pio_sm_put_blocking(led_matrix_pio, sm, led_matrix[i].B << 24);
    }
    // Aguarda 100us para que o sinal de RESET seja reconhecido pelo WS2812B
    sleep_us(100);
}

// Desenha um ponto (LED) individual com cor específica.
// Esta função atualiza imediatamente a saída dos LEDs.
void ws2812b_draw_point(uint8_t point_index, const int color[3])
{
    // Define a cor do LED no buffer
    ws2812b_set_led(point_index, color);

    // Atualiza imediatamente a matriz de LEDs
    ws2812b_write();
    sleep_us(100);
}

/// Mapeia o índice de LED correto considerando o padrão "snake" (zig-zag) da matriz
/// @param row Linha da matriz (0-4)
/// @param col Coluna da matriz (0-4)
/// @return Índice do LED correspondente no buffer
static uint16_t get_snake_index(uint8_t row, uint8_t col)
{
    if (row % 2 == 0)
    {
        // Linha par: esquerda para direita
        return row * LED_MATRIX_COL + col;
    }
    else
    {
        // Linha ímpar: direita para esquerda (invertida)
        return row * LED_MATRIX_COL + (LED_MATRIX_COL - 1 - col);
    }
}

// Preenche uma coluna da matriz de LEDs com uma cor específica.
void ws2812b_fill_column(uint8_t column, const int color[3])
{
    if (column >= LED_MATRIX_COL)
        return;

    // Itera por todas as linhas e define a cor da coluna
    for (int row = 0; row < LED_MATRIX_ROW; row++)
    {
        uint16_t led_index = get_snake_index(row, column);
        ws2812b_set_led(led_index, color);
    }
}

// Preenche uma linha da matriz de LEDs com uma cor específica.
void ws2812b_fill_row(uint8_t row, const int color[3])
{
    if (row >= LED_MATRIX_ROW)
        return;

    // Itera por todas as colunas e define a cor da linha
    for (int col = 0; col < LED_MATRIX_COL; col++)
    {
        uint16_t led_index = get_snake_index(row, col);
        ws2812b_set_led(led_index, color);
    }
}

// Preenche toda a matriz de LEDs com uma cor específica.
// Esta função atualiza imediatamente a saída dos LEDs.
void ws2812b_fill_matrix(const int color[3])
{
    for (uint i = 0; i < LED_MATRIX_SIZE; ++i)
    {
        ws2812b_set_led(i, color);
    }

    // Atualiza imediatamente a matriz de LEDs
    ws2812b_write();
}

void ws2812b_draw_number(uint8_t number_index)
{
    // Validação do índice
    if (number_index > 9)
    {
        printf("ERRO: Número inválido %d. Deve ser entre 0-9.\n", number_index);
        return;
    }

    uint8_t r, g, b;

    // Atribui a cor do número atual.
    r = led_matrix_number_colors[number_index][0];
    g = led_matrix_number_colors[number_index][1];
    b = led_matrix_number_colors[number_index][2];

    // Limpa a matriz de LEDs.
    ws2812b_clear();
    ws2812b_write();

    // Desenha o número na matriz de LEDs.
    printf("Desenhando número %d\n", number_index);
    int color[3] = {r, g, b};
    for (int i = 0; i < LED_MATRIX_SIZE; i++)
    {
        if (led_matrix_numbers[number_index][i] != 0)
        {
            ws2812b_set_led(i, color);
        }
    }

    // Atualiza a matriz de LEDs.
    ws2812b_write();
}
