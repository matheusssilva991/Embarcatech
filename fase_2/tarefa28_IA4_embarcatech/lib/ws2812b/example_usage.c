/**
 * @file example_usage.c
 * @brief Exemplo de uso da biblioteca ws2812b com números
 *
 * Este arquivo demonstra como usar a biblioteca ws2812b para:
 * - Inicializar a matriz de LEDs
 * - Desenhar números de 0 a 9
 * - Controlar LEDs individuais
 * - Preencher colunas
 */

#include "pico/stdlib.h"
#include "ws2812b.h"
#include <stdio.h>

/**
 * @brief Exemplo 1: Desenhar todos os números de 0 a 9 em sequência
 */
void example_draw_all_numbers()
{
    printf("=== Exemplo: Desenhar números de 0 a 9 ===\n");

    ws2812b_init();

    for (uint8_t i = 0; i < 10; i++)
    {
        printf("Mostrando número: %d\n", i);
        ws2812b_draw_number(i);
        sleep_ms(1000); // Aguarda 1 segundo entre números
    }

    // Limpa a matriz ao final
    ws2812b_clear();
    ws2812b_write();
}

/**
 * @brief Exemplo 2: Contador regressivo de 9 a 0
 */
void example_countdown()
{
    printf("=== Exemplo: Contador regressivo ===\n");

    ws2812b_init();

    for (int i = 9; i >= 0; i--)
    {
        printf("Contagem: %d\n", i);
        ws2812b_draw_number((uint8_t)i);
        sleep_ms(1000);
    }

    // Limpa a matriz ao final
    ws2812b_clear();
    ws2812b_write();
}

/**
 * @brief Exemplo 3: Mostrar um número específico
 */
void example_show_specific_number(uint8_t number)
{
    printf("=== Exemplo: Mostrar número específico ===\n");

    ws2812b_init();

    if (number <= 9)
    {
        printf("Mostrando número: %d\n", number);
        ws2812b_draw_number(number);
        sleep_ms(5000); // Mantém por 5 segundos
    }
    else
    {
        printf("ERRO: Número deve ser entre 0-9\n");
    }

    ws2812b_clear();
    ws2812b_write();
}

/**
 * @brief Exemplo 4: Animação de carregamento com colunas
 */
void example_loading_animation()
{
    printf("=== Exemplo: Animação de carregamento ===\n");

    ws2812b_init();

    const int green[] = {0, 15, 0};

    // Preenche coluna por coluna
    for (uint8_t col = 0; col < LED_MATRIX_COL; col++)
    {
        ws2812b_fill_column(col, green);
        ws2812b_write();
        sleep_ms(300);
    }

    sleep_ms(500);

    // Limpa
    ws2812b_clear();
    ws2812b_write();
}

/**
 * @brief Exemplo 5: Padrão personalizado de LEDs
 */
void example_custom_pattern()
{
    printf("=== Exemplo: Padrão personalizado ===\n");

    ws2812b_init();

    // Desenha um "X" vermelho na matriz
    const uint8_t red = 15;

    // Diagonal principal
    for (int i = 0; i < 5; i++)
    {
        int led_index = i * 5 + i; // (0,0), (1,1), (2,2), (3,3), (4,4)
        ws2812b_set_led(led_index, red, 0, 0);
    }

    // Diagonal secundária
    for (int i = 0; i < 5; i++)
    {
        int led_index = i * 5 + (4 - i); // (0,4), (1,3), (2,2), (3,1), (4,0)
        ws2812b_set_led(led_index, red, 0, 0);
    }

    ws2812b_write();
    sleep_ms(3000);

    ws2812b_clear();
    ws2812b_write();
}

/**
 * @brief Exemplo completo: modo demo
 *
 * Executa todos os exemplos em sequência
 */
void demo_mode()
{
    printf("\n");
    printf("======================================\n");
    printf("  WS2812B + LED Matrix Numbers Demo  \n");
    printf("======================================\n\n");

    // Exemplo 1: Todos os números
    example_draw_all_numbers();
    sleep_ms(1000);

    // Exemplo 2: Contador regressivo
    example_countdown();
    sleep_ms(1000);

    // Exemplo 3: Número específico (7)
    example_show_specific_number(7);
    sleep_ms(1000);

    // Exemplo 4: Animação de carregamento
    example_loading_animation();
    sleep_ms(1000);

    // Exemplo 5: Padrão personalizado
    example_custom_pattern();

    printf("\n=== Demo concluído ===\n");
}

// Para usar no seu main.c, descomente uma das opções abaixo:
/*
int main() {
    stdio_init_all();
    sleep_ms(2000);

    // Opção 1: Executar demo completo
    demo_mode();

    // Opção 2: Executar apenas um exemplo específico
    // example_draw_all_numbers();
    // example_countdown();
    // example_show_specific_number(5);
    // example_loading_animation();
    // example_custom_pattern();

    // Loop infinito (se necessário)
    while (true) {
        tight_loop_contents();
    }

    return 0;
}
*/
