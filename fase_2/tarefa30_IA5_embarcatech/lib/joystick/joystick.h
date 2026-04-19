/**
 * @file joystick.h
 * @brief Biblioteca para controle do joystick analógico
 * @author Embarcatech
 *
 * Esta biblioteca fornece funções para ler e interpretar entradas
 * do joystick analógico com dois eixos (X e Y) conectado ao RP2040.
 *
 * O joystick retorna valores de 0 a 4095 (resolução de 12 bits do ADC):
 * - Eixo X: 0 (esquerda) -> 2048 (centro) -> 4095 (direita)
 * - Eixo Y: 0 (para frente) -> 2048 (centro) -> 4095 (para trás)
 *
 * @note Os valores retornados variam de acordo com a calibração do joystick
 * @note Use as constantes de DEADZONE para evitar leituras incorretas próximas ao centro
 */

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

/// GPIO para o eixo Y do joystick (ADC chan 0)
#define JRY_PIN 26
/// GPIO para o eixo X do joystick (ADC chan 1)
#define JRX_PIN 27

/// Valor máximo retornado pelo ADC de 12 bits
#define ADC_MAX_VALUE 4095
/// Valor central esperado do ADC (50% de 4095)
#define ADC_CENTER 2048
/// Largura da zona morta do joystick (threshold para considerar movimento real)
#define JOYSTICK_DEADZONE 200

/// Estados das direções do joystick
typedef enum
{
    JOYSTICK_NEUTRAL = 0,   ///< Centro (dentro da deadzone)
    JOYSTICK_UP = 1,        ///< Para frente (Y baixo)
    JOYSTICK_DOWN = 2,      ///< Para trás (Y alto)
    JOYSTICK_LEFT = 3,      ///< Esquerda (X baixo)
    JOYSTICK_RIGHT = 4,     ///< Direita (X alto)
    JOYSTICK_UP_LEFT = 5,   ///< Diagonal para frente-esquerda
    JOYSTICK_UP_RIGHT = 6,  ///< Diagonal para frente-direita
    JOYSTICK_DOWN_LEFT = 7, ///< Diagonal para trás-esquerda
    JOYSTICK_DOWN_RIGHT = 8 ///< Diagonal para trás-direita
} joystick_direction_t;

/**
 * @brief Inicializa o joystick
 *
 * Configura o ADC para ler os dois eixos do joystick nos pinos GPIO 26 e 27.
 * Deve ser chamado uma vez antes de usar as outras funções.
 */
void init_joystick();

/**
 * @brief Lê o valor bruto do eixo X do joystick
 * @return Valor entre 0 (esquerda) e 4095 (direita), área central ~2048
 */
uint16_t get_joystick_x();

/**
 * @brief Lê o valor bruto do eixo Y do joystick
 * @return Valor entre 0 (para frente) e 4095 (para trás), área central ~2048
 */
uint16_t get_joystick_y();

/**
 * @brief Interpreta a direção do joystick
 * @return Enumeração com a direção (JOYSTICK_UP, JOYSTICK_DOWN, etc.)
 *
 * Utiliza JOYSTICK_DEADZONE para evitar leituras falsas perto do centro.
 * Retorna a direção mais próxima: cardinal (4 direções) ou diagonal (8 direções).
 */
joystick_direction_t get_joystick_direction();

/**
 * @brief Verifica se o joystick está fora da zona morta
 * @return true se o joystick foi movido significativamente, false caso contrário
 *
 * Útil para detectar movimentos intents do usuário.
 */
bool is_joystick_pressed();

/**
 * @brief Calcule a magnitude (distância do centro) do joystick
 * @return Valor normalizado de 0.0 (centro) a 1.0 (máxima deflexão)
 *
 * Usa cálculo de distância Euclidiana para determinar o quão longe
 * o joystick foi movido do ponto central.
 */
float get_joystick_magnitude();

/**
 * @brief Obtém os valores normalizados de X e Y de forma segura
 * @param x Ponteiro para receber valor X normalizado (-1.0 a 1.0)
 * @param y Ponteiro para receber valor Y normalizado (-1.0 a 1.0)
 *
 * Normaliza os valores lidos do ADC para o intervalo [-1.0, 1.0],
 * onde 0 representa o centro do joystick.
 */
void get_joystick_normalized(float *x, float *y);

#endif // JOYSTICK_H
