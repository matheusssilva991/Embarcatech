/**
 * @file button.h
 * @brief Biblioteca para leitura de botões com pull-up interno
 * @author Embarcatech
 *
 * Esta biblioteca fornece funções para configurar e ler o estado de
 * botões conectados aos GPIOs do RP2040. Os botões usam resistores
 * pull-up internos, portanto devem conectar o GPIO ao GND quando pressionados.
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <stdlib.h>
#include "pico/stdlib.h"

// Definição dos pinos GPIO para os botões
#define BTN_A_PIN 5   ///< GPIO para botão A
#define BTN_B_PIN 6   ///< GPIO para botão B
#define BTN_SW_PIN 22 ///< GPIO para botão do joystick (switch)

/**
 * @brief Inicializa um botão específico
 * @param pin Número do GPIO do botão
 *
 * Configura o pino como entrada com pull-up interno.
 * O botão deve conectar o pino ao GND quando pressionado.
 */
void init_btn(uint8_t pin);

/**
 * @brief Inicializa todos os botões definidos
 *
 * Configura BTN_A_PIN, BTN_B_PIN e BTN_SW_PIN como entradas
 * com pull-up interno habilitado.
 */
void init_btns();

/**
 * @brief Verifica se um botão está pressionado
 * @param pin Número do GPIO do botão
 * @return true se o botão está pressionado, false caso contrário
 *
 * Retorna true quando o nível lógico é LOW (botão conecta ao GND).
 * Para debounce, recomenda-se adicionar um delay após detectar pressionamento.
 */
bool btn_is_pressed(uint8_t pin);

#endif // BUTTON_H
