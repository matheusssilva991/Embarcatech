#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"

#include "lib/ws2812b/ws2812b.h"
#include "lib/button/button.h"

#define SERVO_PIN 8
#define PULSE_TO_STOP 1500
#define PULSE_AHEAD 1600
#define PULSE_BACK 1400
#define DELAY_PER_DEGREE 50

typedef enum gate_control_state {
    CLOSED,
    OPENING,
    OPEN,
    CLOSING
} gate_control_state_t;

typedef struct gate_control {
    gate_control_state_t state;
    bool manual_control;
    uint32_t last_state_change;
    bool obstacle_detected;
    int gate_position;
} gate_control_t;

long map(long x, long in_min, long in_max, long out_min, long out_max);
void gpio_irq_callback(uint gpio, uint32_t events);
void init_servo();
void open_parking_gate();
void close_parking_gate();
void stop_servo();
void forward_servo();
void backward_servo();

static gate_control_t gate = {CLOSED, true, 0, false, 0};
static volatile int32_t last_time_btn_a_pressed = 0;
static volatile int32_t last_time_btn_b_pressed = 0;
static volatile bool btn_a_pressed = false;

int main()
{
    stdio_init_all();

    //ws2812b_init();
    init_btns();
    init_btn(BTN_SW_PIN);
    init_servo();

    gpio_set_irq_enabled_with_callback(
        BTN_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_callback);
    gpio_set_irq_enabled(
        BTN_B_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(
        BTN_SW_PIN, GPIO_IRQ_EDGE_FALL, true);


    while (true) {
        if (gate.manual_control) {
            // Controle manual do portão
            if (btn_a_pressed) {
                if (gate.state == CLOSED) {
                    open_parking_gate();
                } else if (gate.state == OPEN) {
                    close_parking_gate();
                }
                btn_a_pressed = false;
            }
        } else {
            // Controle automático do portão (a ser implementado)
            // Exemplo: fechar o portão após 10 segundos aberto
            if (gate.state == OPEN &&
                to_ms_since_boot(get_absolute_time()) - gate.last_state_change > 10000) {
                close_parking_gate();
            }
        }
        sleep_ms(100);
    }
}

void init_servo() {
    gpio_set_function(SERVO_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 125.0f);
    pwm_config_set_wrap(&config, 19999);
    pwm_init(slice_num, &config, true);
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void stop_servo() {
    pwm_set_gpio_level(SERVO_PIN, PULSE_TO_STOP);
}

void forward_servo() {
    pwm_set_gpio_level(SERVO_PIN, PULSE_AHEAD);
}

void backward_servo() {
    pwm_set_gpio_level(SERVO_PIN, PULSE_BACK);
}

void open_parking_gate() {
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);

    // Move o servo de 0 a 90 graus
    gate.state = OPENING;
    for (int angle = 0; angle <= 90; angle+=10) {
        forward_servo();
        sleep_ms(DELAY_PER_DEGREE);
    }
    stop_servo();


    gate.state = OPEN;
    gate.last_state_change = to_ms_since_boot(get_absolute_time());
}

void close_parking_gate() {
    uint slice_num = pwm_gpio_to_slice_num(SERVO_PIN);

    // Move o servo de 90 a 0 graus
    gate.state = CLOSING;
    for (int angle = 90; angle >= 0; angle-=10) {
        backward_servo();
        sleep_ms(DELAY_PER_DEGREE);
    }
    stop_servo();

    gate.state = CLOSED;
    gate.last_state_change = to_ms_since_boot(get_absolute_time());
}

void gpio_irq_callback(uint gpio, uint32_t events)
{
    int32_t current_time = to_ms_since_boot(get_absolute_time());

    if (gpio == BTN_A_PIN) {
        if (current_time - last_time_btn_a_pressed > 250) {
            last_time_btn_a_pressed = current_time;
            btn_a_pressed = true;
        }
    } else if (gpio == BTN_B_PIN) {
        if (current_time - last_time_btn_b_pressed > 250) {
            last_time_btn_b_pressed = current_time;
            gate.manual_control = !gate.manual_control;
        }
    } else if (gpio == BTN_SW_PIN) {
        reset_usb_boot(0, 0); // Reset the Pico to bootloader mode
    }
}
