/**
 * @file ws2812b.h
 * @brief Biblioteca para controle de matriz de LEDs WS2812B (NeoPixel)
 * @author Embarcatech
 *
 * Esta biblioteca utiliza o hardware PIO do RP2040 para controlar
 * LEDs endereçáveis WS2812B com timing preciso. Suporta controle
 * individual de cor RGB para cada LED da matriz 5x5.
 *
 * Os LEDs WS2812B requerem sinais com timing muito preciso (800 kHz),
 * que são gerados pela máquina de estado PIO.
 */

#ifndef WS2812B_H
#define WS2812B_H

#include <stdlib.h>
#include <stdio.h>
#include "hardware/pio.h"
#include "pico/stdlib.h"

// Configuração da matriz de LEDs
#define LED_MATRIX_ROW 5                                  ///< Número de linhas da matriz
#define LED_MATRIX_COL 5                                  ///< Número de colunas da matriz
#define LED_MATRIX_PIN 7                                  ///< GPIO conectado ao pino de dados dos LEDs
#define LED_MATRIX_SIZE (LED_MATRIX_ROW * LED_MATRIX_COL) ///< Total de LEDs na matriz (25)

/**
 * @brief Estrutura representando um pixel RGB
 *
 * Cada pixel é composto por três componentes de cor de 8 bits.
 * ATENÇÃO: A ordem é GRB, não RGB (específico do WS2812B)
 */
struct pixel_t
{
    uint8_t G; ///< Componente verde (0-255)
    uint8_t R; ///< Componente vermelho (0-255)
    uint8_t B; ///< Componente azul (0-255)
};
typedef struct pixel_t pixel_t;
typedef pixel_t ws2812b_LED_t; ///< Alias para melhor legibilidade

// Variáveis globais (definidas em ws2812b.c)
extern ws2812b_LED_t led_matrix[LED_MATRIX_SIZE]; ///< Buffer de pixels da matriz
extern PIO led_matrix_pio;                        ///< Ponteiro para a máquina PIO
extern uint sm;                                   ///< Número da state machine PIO

/**
 * @brief Inicializa a matriz de LEDs WS2812B
 *
 * Configura o hardware PIO e a state machine para gerar
 * o sinal de controle dos LEDs. Limpa todos os LEDs após inicialização.
 */
void ws2812b_init();

/**
 * @brief Define a cor de um LED específico
 * @param index Índice do LED (0 a 24 para matriz 5x5)
 * @param r Componente vermelho (0-255)
 * @param g Componente verde (0-255)
 * @param b Componente azul (0-255)
 *
 * Define a cor no buffer. Use ws2812b_write() para aplicar as mudanças.
 * O índice começa em 0 no canto superior esquerdo e aumenta da
 * esquerda para direita, linha por linha.
 */
void ws2812b_set_led(const uint index, const uint8_t r, const uint8_t g, const uint8_t b);

/**
 * @brief Limpa todos os LEDs da matriz
 *
 * Define a cor de todos os LEDs como preto (RGB = 0,0,0).
 * Use ws2812b_write() para aplicar as mudanças.
 */
void ws2812b_clear();

/**
 * @brief Atualiza os LEDs com as cores definidas no buffer
 *
 * Envia os dados do buffer para os LEDs físicos via PIO.
 * Deve ser chamado após modificar cores com ws2812b_set_led() ou ws2812b_clear().
 */
void ws2812b_write();

/**
 * @brief Desenha um ponto (LED) com cor específica
 * @param number_index Índice do LED (0-24)
 * @param color Array RGB com 3 elementos [R, G, B]
 *
 * Função auxiliar para definir cor usando array ao invés de parâmetros separados.
 */
void ws2812b_draw_point(uint8_t number_index, const int color[3]);

/**
 * @brief Preenche uma coluna inteira com uma cor
 * @param column Número da coluna (0-4)
 * @param color Array RGB com 3 elementos [R, G, B]
 *
 * Define todos os 5 LEDs da coluna especificada com a mesma cor.
 */
void ws2812b_fill_column(uint8_t column, const int color[3]);

/**
 * @brief Desenha um número (0-9) na matriz de LEDs
 * @param number_index Número a ser desenhado (0-9)
 *
 * Desenha o número especificado na matriz 5x5 usando padrões pré-definidos.
 * Cada número tem sua própria cor associada.
 * A matriz é limpa antes de desenhar o número.
 */
void ws2812b_draw_number(uint8_t number_index);

#endif // WS2812B_H
