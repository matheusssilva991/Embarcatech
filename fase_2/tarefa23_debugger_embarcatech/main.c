#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/irq.h"
#include "hardware/uart.h"

#define UART_TX_PIN 16
#define UART_RX_PIN 17
#define UART_BAUD_RATE 115200

// Definições dos pinos dos relés
#define RELAY_SWITCH_PIN 6
#define RELAY_SPEED1_PIN 7
#define RELAY_SPEED2_PIN 8
#define RELAY_SPEED3_PIN 9

// Definições dos pinos dos botões
#define BUTTON_SWITCH_PIN 10
#define BUTTON_SPEED1_PIN 13
#define BUTTON_SPEED2_PIN 12
#define BUTTON_SPEED3_PIN 11

volatile bool system_on = false;
volatile int last_speed = 1;

// Callback única para todos os botões
void button_callback(uint gpio, uint32_t events)
{
    static uint32_t last_press_switch = 0;
    static uint32_t last_press_speed1 = 0;
    static uint32_t last_press_speed2 = 0;
    static uint32_t last_press_speed3 = 0;

    uint32_t now = time_us_32();

    if (gpio == BUTTON_SWITCH_PIN)
    {
        if ((now - last_press_switch) > 200000)
        { // debounce
            system_on = !system_on;
            printf("Switch %s\n", system_on ? "ON" : "OFF");
            last_press_switch = now;
        }
    }
    else if (gpio == BUTTON_SPEED1_PIN)
    {
        if ((now - last_press_speed1) > 200000)
        {
            last_speed = 1;
            system_on = true; // Liga o sistema se não estiver ligado
            printf("Speed set to 1\n");
            last_press_speed1 = now;
        }
    }
    else if (gpio == BUTTON_SPEED2_PIN)
    {
        if ((now - last_press_speed2) > 200000)
        {
            last_speed = 2;
            system_on = true;
            printf("Speed set to 2\n");
            last_press_speed2 = now;
        }
    }
    else if (gpio == BUTTON_SPEED3_PIN)
    {
        if ((now - last_press_speed3) > 200000)
        {
            last_speed = 3;
            system_on = true;
            printf("Speed set to 3\n");
            last_press_speed3 = now;
        }
    }
}

int main()
{
    stdio_init_all();
    stdio_uart_init_full(uart0, UART_BAUD_RATE, UART_TX_PIN, UART_RX_PIN);

    // Inicializa GPIOs dos relés como saída
    gpio_init(RELAY_SWITCH_PIN);
    gpio_set_dir(RELAY_SWITCH_PIN, GPIO_OUT);
    gpio_init(RELAY_SPEED1_PIN);
    gpio_set_dir(RELAY_SPEED1_PIN, GPIO_OUT);
    gpio_init(RELAY_SPEED2_PIN);
    gpio_set_dir(RELAY_SPEED2_PIN, GPIO_OUT);
    gpio_init(RELAY_SPEED3_PIN);
    gpio_set_dir(RELAY_SPEED3_PIN, GPIO_OUT);

    // Inicializa GPIOs dos botões como entrada com pull-up
    gpio_init(BUTTON_SWITCH_PIN);
    gpio_set_dir(BUTTON_SWITCH_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_SWITCH_PIN);
    gpio_init(BUTTON_SPEED1_PIN);
    gpio_set_dir(BUTTON_SPEED1_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_SPEED1_PIN);
    gpio_init(BUTTON_SPEED2_PIN);
    gpio_set_dir(BUTTON_SPEED2_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_SPEED2_PIN);
    gpio_init(BUTTON_SPEED3_PIN);
    gpio_set_dir(BUTTON_SPEED3_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_SPEED3_PIN);

    // Configura interrupções nos botões (bordas de descida)
    // Registra o callback apenas uma vez
    gpio_set_irq_enabled_with_callback(BUTTON_SWITCH_PIN, GPIO_IRQ_EDGE_FALL, true, &button_callback);

    // Para os outros pinos, use apenas gpio_set_irq_enabled
    gpio_set_irq_enabled(BUTTON_SPEED1_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BUTTON_SPEED2_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BUTTON_SPEED3_PIN, GPIO_IRQ_EDGE_FALL, true);

    while (true)
    {
        // printf("System is %s, Last Speed: %d\n", system_on ? "ON" : "OFF", last_speed);
        // Controle dos relés conforme estado atualizado pelas interrupções
        gpio_put(RELAY_SWITCH_PIN, !system_on);
        gpio_put(RELAY_SPEED1_PIN, !(system_on && last_speed == 1));
        gpio_put(RELAY_SPEED2_PIN, !(system_on && last_speed == 2));
        gpio_put(RELAY_SPEED3_PIN, !(system_on && last_speed == 3));

        sleep_ms(50);
    }
}
