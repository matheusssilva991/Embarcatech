/**
 * @file led.h
 * @brief Biblioteca para controle de LEDs RGB individuais
 * @author Embarcatech
 *
 * Esta biblioteca fornece funções para controlar LEDs RGB individuais
 * conectados ao RP2040. Permite controle de cores primárias e compostas.
 */

#ifndef LED_H
#define LED_H

#include <stdlib.h>
#include "pico/stdlib.h"

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

#endif // LED_H
