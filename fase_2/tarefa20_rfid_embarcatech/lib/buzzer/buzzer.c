#include "buzzer.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/time.h"

// Inicializa o PWM no pino do buzzer
int init_buzzer(uint pin, float clk_div)
{
    gpio_set_function(pin, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, clk_div); // Ajusta divisor de clock
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(pin, 0); // Desliga o PWM inicialmente

    return slice_num; // Retorna o número do slice PWM
}

// Toca uma nota com a frequência especificada
void play_tone(uint pin, uint frequency)
{
    if (frequency == 0) return; // Evita divisão por zero

    uint slice_num = pwm_gpio_to_slice_num(pin);
    uint32_t clock_freq = clock_get_hz(clk_sys);
    uint32_t top = clock_freq / frequency - 1;

    // Verifica limite do hardware (16 bits)
    if (top > 65535) top = 65535;

    pwm_set_wrap(slice_num, top);
    pwm_set_gpio_level(pin, top / 2); // 50% de duty cycle
}

// Desliga o tom no pino do buzzer
void stop_tone(uint pin)
{
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_gpio_level(pin, 0); // Desliga o PWM
}

// Nova função: Toca uma nota pela duração especificada em ms
void play_tone_with_duration(uint pin, uint frequency, uint duration_ms)
{
    play_tone(pin, frequency);
    sleep_ms(duration_ms);
    stop_tone(pin);
}
