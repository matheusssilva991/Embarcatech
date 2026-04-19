/**
 * @file buzzer.h
 * @brief Biblioteca para controle de buzzer com PWM
 * @author Embarcatech
 *
 * Esta biblioteca permite gerar tons audíveis em um buzzer usando
 * modulação por largura de pulso (PWM). Suporta frequências variáveis
 * para criação de melodias e efeitos sonoros.
 */

#ifndef BUZZER_H
#define BUZZER_H

#include <stdlib.h>
#include "pico/stdlib.h"

// Definição dos pinos GPIO para os buzzers
#define BUZZER_A_PIN 21 ///< GPIO para buzzer A
#define BUZZER_B_PIN 10 ///< GPIO para buzzer B

/**
 * @brief Inicializa o PWM no pino do buzzer
 * @param pin Número do GPIO conectado ao buzzer
 * @param clk_div Divisor de clock para ajustar a frequência base (tipicamente 64.0)
 * @return Número da fatia PWM configurada, ou -1 em caso de erro
 *
 * Configura o pino como saída PWM e define o divisor de clock.
 * Um clk_div maior resulta em frequências mais baixas disponíveis.
 */
int init_buzzer(uint pin, float clk_div);

/**
 * @brief Toca um tom com frequência específica
 * @param pin Número do GPIO do buzzer
 * @param frequency Frequência do tom em Hertz (Hz)
 *
 * Configura o PWM para gerar um tom audível na frequência especificada.
 * Frequências típicas:
 * - Nota Dó (C4): 262 Hz
 * - Nota Lá (A4): 440 Hz
 * - Nota Dó (C5): 523 Hz
 */
void play_tone(uint pin, uint frequency);

/**
 * @brief Para a reprodução do tom no buzzer
 * @param pin Número do GPIO do buzzer
 *
 * Desativa o PWM, silenciando o buzzer
 */
void stop_tone(uint pin);

#endif // BUZZER_H
