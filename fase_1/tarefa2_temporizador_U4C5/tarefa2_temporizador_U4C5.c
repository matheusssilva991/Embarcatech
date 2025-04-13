#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"

#define GREEN_LED_PIN 11
#define BLUE_LED_PIN 12
#define RED_LED_PIN 13
#define BTN_A_PIN 5
#define LEDS_DELAY 3000
#define DEBOUNCE_DELAY 200

static volatile bool btn_disabled = false;
static volatile bool btn_pressed = false;
static absolute_time_t last_press_time;
static volatile int led_state = 0;

void init_led(uint led_pin);
void init_btn(uint btn_pin);
int64_t alarm_callback(alarm_id_t id, void *user_data);
void btn_callback(uint gpio, uint32_t events);

int main()
{
    stdio_init_all();

    // Inicializa os LEDs
    init_led(RED_LED_PIN);
    init_led(BLUE_LED_PIN);
    init_led(GREEN_LED_PIN);

    // Inicializa o botão
    init_btn(BTN_A_PIN);

    while (true)
    {
        if (btn_pressed && !btn_disabled)
        {
            btn_disabled = true;
            btn_pressed = false;

            add_alarm_in_ms(1, alarm_callback, NULL, false);
        }
        sleep_ms(10);
    }

    return 0;
}

void init_led(uint led_pin)
{
    gpio_init(led_pin);
    gpio_set_dir(led_pin, GPIO_OUT);
}

void init_btn(uint btn_pin)
{
    gpio_init(btn_pin);
    gpio_set_dir(btn_pin, GPIO_IN);
    gpio_pull_up(btn_pin);
    gpio_set_irq_enabled_with_callback(btn_pin, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
}

int64_t alarm_callback(alarm_id_t id, void *user_data)
{
    printf("Alarme disparado\n");
    switch (led_state)
    {
    case 0:
        printf("Ligando todos os LEDs\n");
        gpio_put(GREEN_LED_PIN, 1);
        gpio_put(BLUE_LED_PIN, 1);
        gpio_put(RED_LED_PIN, 1);
        break;
    case 1:
        printf("Desligando LED verde\n");
        gpio_put(GREEN_LED_PIN, 0);
        gpio_put(BLUE_LED_PIN, 1);
        gpio_put(RED_LED_PIN, 1);
        break;
    case 2:
        printf("Desligando LED azul\n");
        gpio_put(GREEN_LED_PIN, 0);
        gpio_put(BLUE_LED_PIN, 0);
        gpio_put(RED_LED_PIN, 1);
        break;
    case 3:
        printf("Desligando LED vermelho\n");
        gpio_put(GREEN_LED_PIN, 0);
        gpio_put(BLUE_LED_PIN, 0);
        gpio_put(RED_LED_PIN, 0);
        btn_disabled = false;
        led_state = 0;
        return 0;
    }

    led_state = (led_state + 1) % 4;

    add_alarm_in_ms(LEDS_DELAY, alarm_callback, NULL, false); // Reagendar próxima transição
}

void btn_callback(uint gpio, uint32_t events)
{
    absolute_time_t current_time = get_absolute_time();

    // Verificar se o tempo decorrido desde a última pressão é maior que o debounce
    if (absolute_time_diff_us(last_press_time, current_time) > DEBOUNCE_DELAY * 1000)
    {
        if (btn_disabled)
            return;

        printf("Botão pressionado\n");
        btn_pressed = true;
        last_press_time = current_time;  // Atualiza o tempo da última pressão
    }
}
