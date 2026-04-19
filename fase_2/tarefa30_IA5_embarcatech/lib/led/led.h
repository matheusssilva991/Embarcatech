/**
 * @file led.h
 * @brief Biblioteca para controle de LEDs RGB individuais
 * @author Matheus Santos Silva
 *
 * Esta biblioteca fornece funções para controlar LEDs RGB individuais
 * conectados ao RP2040. Permite controle de cores primárias e compostas.
 */

#ifndef LED_H
#define LED_H

#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

// Definição dos pinos GPIO para cada LED
#define GREEN_LED_PIN 11 ///< GPIO para LED verde
#define BLUE_LED_PIN 12  ///< GPIO para LED azul
#define RED_LED_PIN 13   ///< GPIO para LED vermelho

/**
 * @brief Inicializa um pino de LED específico
 * @param pin Número do GPIO do LED
 */
void init_led(uint8_t pin);

/**
 * @brief Inicializa todos os LEDs RGB (verde, azul e vermelho)
 *
 * Configura os três pinos como saída e os coloca em estado LOW (desligado)
 */
void init_leds();

/**
 * @brief Desliga todos os LEDs RGB
 *
 * Coloca todos os pinos de LED em estado LOW
 */
void turn_off_leds();

/**
 * @brief Acende apenas o LED verde
 *
 * Desliga os outros LEDs e acende o verde
 */
void set_led_green();

/**
 * @brief Acende apenas o LED azul
 *
 * Desliga os outros LEDs e acende o azul
 */
void set_led_blue();

/**
 * @brief Acende apenas o LED vermelho
 *
 * Desliga os outros LEDs e acende o vermelho
 */
void set_led_red();

/**
 * @brief Cria cor amarela (combinação de verde e vermelho)
 *
 * Acende os LEDs verde e vermelho simultaneamente, desliga o azul
 */
void set_led_yellow();

/**
 * @brief Cria cor laranja (verde e vermelho com ajuste)
 */
void set_led_orange();

/**
 * @brief Cria cor ciano (combinação de verde e azul)
 *
 * Acende os LEDs verde e azul simultaneamente, desliga o vermelho
 */
void set_led_cyan();

/**
 * @brief Cria cor magenta (combinação de vermelho e azul)
 */
void set_led_magenta();

/**
 * @brief Cria cor roxa (combinação de vermelho e azul)
 */
void set_led_purple();

/**
 * @brief Acende todos os LEDs (cor branca)
 *
 * Acende os LEDs verde, azul e vermelho simultaneamente
 */
void set_led_white();

/**
 * @brief Desliga o LED (mesmo que turn_off_leds)
 */
void set_led_off();

// Funções com PWM para controle de intensidade
/**
 * @brief Inicializa LEDs com modulação PWM para controle de intensidade
 *
 * Configura os três pinos para PWM (8-bit, 0-255)
 */
void init_leds_pwm();

/**
 * @brief Inicializa um pino de LED com PWM
 * @param pin Número do GPIO do LED
 */
void init_led_pwm(uint8_t pin);

/**
 * @brief Define cor RGB usando PWM
 * @param r Intensidade do vermelho (0-255)
 * @param g Intensidade do verde (0-255)
 * @param b Intensidade do azul (0-255)
 */
void pwm_set_rgb(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief LED vermelho com PWM
 */
void set_led_red_pwm();

/**
 * @brief LED azul com PWM
 */
void set_led_blue_pwm();

/**
 * @brief LED verde com PWM
 */
void set_led_green_pwm();

/**
 * @brief LED amarelo com PWM
 */
void set_led_yellow_pwm();

/**
 * @brief LED laranja com PWM
 */
void set_led_orange_pwm();

/**
 * @brief LED ciano com PWM
 */
void set_led_cyan_pwm();

/**
 * @brief LED magenta com PWM
 */
void set_led_magenta_pwm();

/**
 * @brief LED roxo com PWM
 */
void set_led_purple_pwm();

/**
 * @brief LED branco com PWM
 */
void set_led_white_pwm();

/**
 * @brief Desliga LEDs (PWM)
 */
void set_led_off_pwm();

#endif // LED_H
