#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"

// Pinos dos leds
#define RED_LED_PIN 11
#define YELLOW_LED_PIN 12
#define GREEN_LED_PIN 13

const int LEDS_DELAY = 3000; // Delay entre os leds
static volatile int led_id = 1; // Identificador do led

bool repeating_timer_callback(repeating_timer_t *rt);
void init_led(uint led_pin);

int main()
{
    stdio_init_all();

    // Inicializa os leds
    init_led(RED_LED_PIN);
    init_led(YELLOW_LED_PIN);
    init_led(GREEN_LED_PIN);

    // Inicializa o temporizador
    struct repeating_timer timer;

    add_repeating_timer_ms(LEDS_DELAY, repeating_timer_callback, NULL, &timer);

    while (true)
    {
        // Tempo absoluto em ms
        uint32_t start_ms = to_ms_since_boot(get_absolute_time());

        // Aguarda 1 segundo
        sleep_ms(1000);

        // Imprime o tempo absoluto em ms
        printf("Tempo absoluto em ms: %d \n", start_ms);
    }
}

// Função de callback do temporizador
bool repeating_timer_callback(repeating_timer_t *rt)
{
    // Alterna entre os leds
    switch (led_id)
    {
    case 1:
        // Led Vermelho
        printf("Led Vermelho.\n");
        gpio_put(RED_LED_PIN, 1);
        gpio_put(YELLOW_LED_PIN, 0);
        gpio_put(GREEN_LED_PIN, 0);
        break;
    case 2:
        // Led Amarelo
        printf("Led Amarelo.\n");
        gpio_put(RED_LED_PIN, 0);
        gpio_put(YELLOW_LED_PIN, 1);
        gpio_put(GREEN_LED_PIN, 0);
        break;
    case 3:
        // Led Verde
        printf("Led Verde.\n");
        gpio_put(RED_LED_PIN, 0);
        gpio_put(YELLOW_LED_PIN, 0);
        gpio_put(GREEN_LED_PIN, 1);
        break;
    }

    // Altera o led
    led_id = (led_id % 3) + 1;

    return true;
}

// Inicializa o pino do led
void init_led(uint led_pin)
{
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);
}
