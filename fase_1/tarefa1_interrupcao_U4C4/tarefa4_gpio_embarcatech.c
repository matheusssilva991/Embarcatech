#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"

#include "matriz_led.pio.h"

#define LED_MATRIX_COUNT 25
#define LED_MATRIX_PIN 7
#define GREEN_LED_PIN 11
#define BLUE_LED_PIN 12
#define RED_LED_PIN 13
#define BTN_A_PIN 5
#define BTN_B_PIN 6

// Tipos de dados.
struct pixel_t
{
    uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t; // Mudança de nome de "struct pixel_t" para "npLED_t" por clareza.

void np_init(uint pin);
void init_btn(uint pin);
void init_led(uint pin);
void np_set_led(const uint index, const uint8_t r, const uint8_t g, const uint8_t b);
void np_clear();
void np_write();
void draw_number(int number_index);
static void gpio_irq_handler(uint gpio, uint32_t events);
bool turn_off_led(repeating_timer_t *t);

// Variáveis globais
npLED_t leds[LED_MATRIX_COUNT]; // Declaração do buffer de pixels que formam a matriz.
PIO np_pio;                     // Ponteiro para a máquina PIO.
uint sm;                        // Número da máquina state machine.

static volatile int current_number_index = 0; // Índice do número atual.
static volatile uint32_t last_time_btn_a = 0; // Armazena o tempo do último evento do btn A (em microssegundos)
static volatile uint32_t last_time_btn_b = 0; // Armazena o tempo do último evento do btn B(em microssegundos)
static volatile uint32_t last_time_g_led = 0; // Armazena o tempo do último evento do led verde (em microssegundos)
static volatile uint32_t last_time_b_led = 0; // Armazena o tempo do último evento do led azul (em microssegundos)

// Números de 0 a 9 representados na matriz de LEDs.
int numbers[10][LED_MATRIX_COUNT] = {
    {0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0},
    {0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0},
    {0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0},
    {0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0},
    {0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0},
    {0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0},
    {0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0},
    {0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0},
};

// Cores para os números de 0 a 9.
int number_colors[10][3] = {
    {15, 0, 0},   // Vermelho mais fraco
    {0, 15, 0},   // Verde mais fraco
    {0, 0, 15},   // Azul mais fraco
    {15, 15, 0},  // Amarelo mais fraco
    {0, 15, 15},  // Ciano mais fraco
    {15, 0, 15},  // Magenta mais fraco
    {15, 15, 15}, // Branco acinzentado
    {15, 9, 0},  // Laranja mais fraco
    {7, 0, 7},  // Roxo mais fraco
    {9, 2, 2}    // Marrom mais fraco
};


int main()
{
    int old_number_index = current_number_index;

    stdio_init_all();

    // Inicializa botões.
    init_btn(BTN_A_PIN);
    init_btn(BTN_B_PIN);

    // Inicializa LEDs.
    init_led(GREEN_LED_PIN);
    init_led(BLUE_LED_PIN);
    init_led(RED_LED_PIN);

    // Inicializa matriz de LEDs.
    np_init(LED_MATRIX_PIN);
    np_clear();

    // Habilita interrupções nos botões.
    gpio_set_irq_enabled_with_callback(BTN_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BTN_B_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Desenha o número inicial.
    draw_number(current_number_index);

    while (true)
    {
        // Desenha o número inicial.
        // Verifica se o número atual foi alterado.
        if (old_number_index != current_number_index)
        {
            old_number_index = current_number_index;
            draw_number(current_number_index);
        }

        // Pisca o LED vermelho.
        gpio_put(RED_LED_PIN, 1);
        printf("LED vermelho aceso\n");
        sleep_ms(100);

        gpio_put(RED_LED_PIN, 0);
        printf("LED vermelho apagado\n");
        sleep_ms(100);
    }
}

// Inicializa a máquina PIO para controle da matriz de LEDs.
void np_init(uint pin)
{

    // Cria programa PIO.
    uint offset = pio_add_program(pio0, &matriz_led_program);
    np_pio = pio0;

    // Toma posse de uma máquina PIO.
    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0)
    {
        np_pio = pio1;
        sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
    }

    // Inicia programa na máquina PIO obtida.
    matriz_led_program_init(np_pio, sm, offset, pin, 800000.f);

    // Limpa buffer de pixels.
    for (uint i = 0; i < LED_MATRIX_COUNT; ++i)
    {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

// Inicializa um botão.
void init_btn(uint pin)
{
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
}

// Inicializa um LED.
void init_led(uint pin)
{
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
}

// Atribui uma cor RGB a um LED.
void np_set_led(const uint index, const uint8_t r, const uint8_t g, const uint8_t b)
{
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}

// Limpa o buffer de pixels.
void np_clear()
{
    for (uint i = 0; i < LED_MATRIX_COUNT; ++i)
        np_set_led(i, 0, 0, 0);
}

// Escreve os dados do buffer nos LEDs.
void np_write()
{
    // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO.
    for (uint i = 0; i < LED_MATRIX_COUNT; ++i)
    {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100); // Espera 100us, sinal de RESET do datasheet.
}

// Desenha um número na matriz de LEDs.
void draw_number(int number_index)
{
    uint8_t r, g, b;

    // Atribui a cor do número atual.
    r = number_colors[number_index][0];
    g = number_colors[number_index][1];
    b = number_colors[number_index][2];

    // Limpa a matriz de LEDs.
    np_clear();
    np_write();

    // Desenha o número na matriz de LEDs.
    for (int i = 0; i < LED_MATRIX_COUNT; i++)
    {
        if (numbers[number_index][i] != 0)
        {
            printf("Desenhando número %d\n", number_index);
            printf("LED %d: R=%d, G=%d, B=%d\n", i, r, g, b);
            np_set_led(i, r, g, b);
        }
    }

    // Atualiza a matriz de LEDs.
    np_write();
}

// Função de callback para tratamento de interrupção de botões.
void gpio_irq_handler(uint gpio, uint32_t events)
{
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    static repeating_timer_t timer_blue, timer_green;

    // Verifica qual botão foi pressionado.
    if (gpio == BTN_A_PIN && (current_time - last_time_btn_a > 200000))
    {
        // Atualiza o tempo do último evento do botão A.
        last_time_btn_a = current_time;

         // Garante que não seja menor que zero
        if (current_number_index > 0)
        {
            // Decrementa o índice do número atual.
            current_number_index--;

            // Acende o LED verde por 200ms
            gpio_put(GREEN_LED_PIN, 1);
            add_repeating_timer_ms(200, turn_off_led, (void *)GREEN_LED_PIN, &timer_green);
        }
    }
    else if (gpio == BTN_B_PIN && (current_time - last_time_btn_b > 200000))
    {
        // Atualiza o tempo do último evento do botão B.
        last_time_btn_b = current_time;

        // Garante que não ultrapasse o limite
        if (current_number_index < 9 )
        {
            // Incrementa o índice do número atual.
            current_number_index++;

            // Acende o LED azul
            gpio_put(BLUE_LED_PIN, 1);
            add_repeating_timer_ms(200, turn_off_led, (void *)BLUE_LED_PIN, &timer_blue);
        }
    }
}

// Função para desligar o LED
bool turn_off_led(repeating_timer_t *t) {
    gpio_put((uint) t->user_data, 0);  // Desliga o LED
    return false;  // Retorna false para executar apenas uma vez
}
