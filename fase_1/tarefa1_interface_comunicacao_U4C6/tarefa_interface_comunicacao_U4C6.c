#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "lib/ssd1306.h"
#include "lib/font.h"
#include "lib/ws2812b.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C
#define LED_MATRIX_PIN 7
#define GREEN_LED_PIN 11
#define BLUE_LED_PIN 12
#define RED_LED_PIN 13
#define BTN_A_PIN 5
#define BTN_B_PIN 6

void init_led(uint8_t led_pin);
void init_btn(uint8_t btn_pin);
void init_i2c();
void init_display(ssd1306_t *ssd);
void update_led_messages();
static void gpio_irq_handler(uint gpio, uint32_t events);

static volatile uint32_t last_time_btn_a = 0; // Armazena o tempo do último evento do btn A (em microssegundos)
static volatile uint32_t last_time_btn_b = 0; // Armazena o tempo do último evento do btn B(em microssegundos)
static volatile bool green_led_state = false; // Armazena o estado do LED verde
static volatile bool blue_led_state = false; // Armazena o estado do LED azul
static char green_led_state_message [20] = "Led Verde OFF."; // Mensagem exibida no display para o LED verde
static char blue_led_state_message [20] = "Led Azul OFF."; // Mensagem exibida no display para o LED azul

// Inicializa o display OLED
ssd1306_t ssd; // Inicializa a estrutura do display

int main()
{
    stdio_init_all();

    // Inicializa os LEDs
    init_led(GREEN_LED_PIN);
    init_led(BLUE_LED_PIN);
    init_led(RED_LED_PIN);

    // Inicializa os botões
    init_btn(BTN_A_PIN);
    init_btn(BTN_B_PIN);

    // Inicializa a comunicação I2C
    init_i2c();

    init_display(&ssd);

    // Inicializa a matriz de LEDs
    ws2812b_init(LED_MATRIX_PIN);

    // Habilita interrupções nos botões.
    gpio_set_irq_enabled_with_callback(BTN_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BTN_B_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Limpa a matriz de LED
    ws2812b_clear();
    ws2812b_write();

    bool color = true;
    char character = ' ';

    // Inicializa o display com um retângulo e as mensagens dos LEDs
    ssd1306_fill(&ssd, !color); // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 58, color, !color); // Desenha um retângulo
    ssd1306_draw_char(&ssd, character, 58, 30);
    ssd1306_draw_string(&ssd, green_led_state_message, 10, 10);
    ssd1306_draw_string(&ssd, blue_led_state_message, 10, 50);
    ssd1306_send_data(&ssd); // Atualiza o display

    while (true) {
        if(stdio_usb_connected()) {
            if (scanf("%c", &character) == 1) {
                printf("Caractere: %c\n", character);

                // Desenha na matriz de led se for um número
                if (isdigit(character)) {
                    ws2812b_draw_number(character - '0');

                // Limpa a matriz de led caso não seja.
                } else {
                    ws2812b_clear();
                    ws2812b_write();
                }
            }

             color = !color;

            // Atualiza o conteúdo do display com animações
            ssd1306_fill(&ssd, !color); // Limpa o display
            ssd1306_rect(&ssd, 3, 3, 122, 58, color, !color); // Desenha um retângulo
            ssd1306_draw_char(&ssd, character, 58, 30);
            ssd1306_draw_string(&ssd, green_led_state_message, 10, 10);
            ssd1306_draw_string(&ssd, blue_led_state_message, 10, 50);
            ssd1306_send_data(&ssd); // Atualiza o display
        }

        sleep_ms(100);
    }
}

// Inicializa um led em um pino específico
void init_led(uint8_t led_pin)
{
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);
}

// Inicializa um botão em um pino específico
void init_btn(uint8_t btn_pin) {
    gpio_init(btn_pin);
    gpio_set_dir(btn_pin, GPIO_IN);
    gpio_pull_up(btn_pin);
}

// Inicializa a comunicação I2C
void init_i2c() {
    i2c_init(I2C_PORT, 400*1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
}

// Inicializa o display OLED
void init_display(ssd1306_t *ssd) {
    ssd1306_init(ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(ssd);
    ssd1306_send_data(ssd);

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(ssd, false);
    ssd1306_send_data(ssd);
}

// Atualiza as mensagens exibidas no display
void update_led_messages() {
    if (green_led_state)
        strcpy(green_led_state_message, "Led Verde ON.");
    else
        strcpy(green_led_state_message, "Led Verde OFF.");

    if (blue_led_state)
        strcpy(blue_led_state_message, "Led Azul ON.");
    else
        strcpy(blue_led_state_message, "Led Azul OFF.");
}

// Função de callback para as interrupções dos botões
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Verifica qual botão foi pressionado.
    if (gpio == BTN_A_PIN && (current_time - last_time_btn_a > 200000))
    {
        // Atualiza o tempo do último evento do botão A.
        last_time_btn_a = current_time;
        green_led_state = !gpio_get(GREEN_LED_PIN);

        update_led_messages();

        // Acende o LED verde por 200ms
        gpio_put(GREEN_LED_PIN, green_led_state);
        printf("%s\n", green_led_state_message);
    }
    else if (gpio == BTN_B_PIN && (current_time - last_time_btn_b > 200000))
    {
        // Atualiza o tempo do último evento do botão B.
        last_time_btn_b = current_time;
        blue_led_state = !gpio_get(BLUE_LED_PIN);

        update_led_messages();

        // Acende o LED azul
        gpio_put(BLUE_LED_PIN, !gpio_get(BLUE_LED_PIN));
        printf("%s\n", blue_led_state_message);
    }

    ssd1306_draw_string(&ssd, green_led_state_message, 10, 10);
    ssd1306_draw_string(&ssd, blue_led_state_message, 10, 50);
    ssd1306_send_data(&ssd); // Atualiza o display
}

