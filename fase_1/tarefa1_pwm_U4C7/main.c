#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define PWM_PIN 22
//#define PWM_PIN 12
#define PERIOD 20000
#define TOP 24999
#define DIV 100.0f
#define DELAY 5000
#define PWM_STEP TOP * 0.00025
#define PWM_STEP_DELAY 10
#define PWM_UPPER_LEVEL_LIMIT TOP * 0.12
#define PWM_LOWER_LEVEL_LIMIT TOP * 0.025

bool is_increasing = true;
int pwm_level = PWM_LOWER_LEVEL_LIMIT;

void pwm_set_gpio_level_with_delay(uint pin, uint16_t level, uint32_t delay);

int main()
{
    stdio_init_all();

    // Initialize PWM
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);

    uint slice = pwm_gpio_to_slice_num(PWM_PIN);

    pwm_set_clkdiv(slice, DIV);

    pwm_set_wrap(slice, TOP);

    pwm_set_enabled(slice, true);

    printf("180°\n");
    pwm_set_gpio_level_with_delay(PWM_PIN, PWM_UPPER_LEVEL_LIMIT, DELAY);

    printf("90°\n");
    pwm_set_gpio_level_with_delay(PWM_PIN, TOP * 0.0735, DELAY);

    printf("0°\n");
    pwm_set_gpio_level_with_delay(PWM_PIN,  PWM_LOWER_LEVEL_LIMIT, DELAY);

    while (true) {
        pwm_set_gpio_level_with_delay(PWM_PIN,  pwm_level, PWM_STEP_DELAY);

        if (is_increasing) {
            pwm_level += PWM_STEP;

            if (pwm_level > PWM_UPPER_LEVEL_LIMIT) {
                pwm_level = PWM_UPPER_LEVEL_LIMIT;
                is_increasing = false;
            }
        } else {
            pwm_level -= PWM_STEP;

            if (pwm_level < PWM_LOWER_LEVEL_LIMIT) {
                pwm_level = PWM_LOWER_LEVEL_LIMIT;
                is_increasing = true;
            }
        }
    }
}

void pwm_set_gpio_level_with_delay(uint pin, uint16_t level, uint32_t delay) {
    pwm_set_gpio_level(pin, level);
    sleep_ms(delay);
}
