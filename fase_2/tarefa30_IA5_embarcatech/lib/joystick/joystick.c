#include "joystick.h"

/**
 * @brief Inicializa o joystick
 *
 * Configura o ADC e os pinos GPIO para leitura dos eixos X e Y.
 */
void init_joystick()
{
    // Inicializa o módulo ADC
    adc_init();

    // Configura os pinos para entrada do ADC
    adc_gpio_init(JRY_PIN); // Canal 0 (GPIO 26)
    adc_gpio_init(JRX_PIN); // Canal 1 (GPIO 27)
}

/**
 * @brief Lê o valor bruto do eixo X
 *
 * @return Valor ADC de 12 bits (0-4095)
 *         0 = esquerda
 *         ~2048 = centro
 *         4095 = direita
 */
uint16_t get_joystick_x()
{
    // Seleciona o canal 1 para o eixo X (GPIO 27)
    adc_select_input(1);
    return adc_read();
}

/**
 * @brief Lê o valor bruto do eixo Y
 *
 * @return Valor ADC de 12 bits (0-4095)
 *         0 = para frente
 *         ~2048 = centro
 *         4095 = para trás
 */
uint16_t get_joystick_y()
{
    // Seleciona o canal 0 para o eixo Y (GPIO 26)
    adc_select_input(0);
    return adc_read();
}

/**
 * @brief Interpreta a direção do joystick com base nos valores dos eixos
 *
 * @return Enumeração indicando a direção
 */
joystick_direction_t get_joystick_direction()
{
    uint16_t x = get_joystick_x();
    uint16_t y = get_joystick_y();

    // Calcula as diferenças do centro
    int16_t dx = x - ADC_CENTER;
    int16_t dy = y - ADC_CENTER;

    // Verifica se está dentro da zona morta
    if (abs(dx) < JOYSTICK_DEADZONE && abs(dy) < JOYSTICK_DEADZONE)
    {
        return JOYSTICK_NEUTRAL;
    }

    // Determina a direção predominante
    bool is_left = dx < -JOYSTICK_DEADZONE;
    bool is_right = dx > JOYSTICK_DEADZONE;
    bool is_up = dy < -JOYSTICK_DEADZONE;
    bool is_down = dy > JOYSTICK_DEADZONE;

    // Verifica diagonais primeiro
    if (is_up && is_left)
        return JOYSTICK_UP_LEFT;
    if (is_up && is_right)
        return JOYSTICK_UP_RIGHT;
    if (is_down && is_left)
        return JOYSTICK_DOWN_LEFT;
    if (is_down && is_right)
        return JOYSTICK_DOWN_RIGHT;

    // Verifica direções cardinais
    if (is_up)
        return JOYSTICK_UP;
    if (is_down)
        return JOYSTICK_DOWN;
    if (is_left)
        return JOYSTICK_LEFT;
    if (is_right)
        return JOYSTICK_RIGHT;

    return JOYSTICK_NEUTRAL;
}

/**
 * @brief Verifica se o joystick foi movido fora da zona morta
 *
 * @return true se o movimento é significativo, false caso contrário
 */
bool is_joystick_pressed()
{
    uint16_t x = get_joystick_x();
    uint16_t y = get_joystick_y();

    int16_t dx = x - ADC_CENTER;
    int16_t dy = y - ADC_CENTER;

    return (abs(dx) > JOYSTICK_DEADZONE) || (abs(dy) > JOYSTICK_DEADZONE);
}

/**
 * @brief Calcula a magnitude (intensidade) do movimento do joystick
 *
 * @return Valor de 0.0 (centro) a 1.0 (máxima deflexão)
 */
float get_joystick_magnitude()
{
    uint16_t x = get_joystick_x();
    uint16_t y = get_joystick_y();

    // Normaliza para [-1.0, 1.0]
    float nx = (float)(x - ADC_CENTER) / ADC_CENTER;
    float ny = (float)(y - ADC_CENTER) / ADC_CENTER;

    // Limita ao intervalo [-1.0, 1.0]
    if (nx > 1.0f)
        nx = 1.0f;
    if (nx < -1.0f)
        nx = -1.0f;
    if (ny > 1.0f)
        ny = 1.0f;
    if (ny < -1.0f)
        ny = -1.0f;

    // Calcula a distância Euclidiana
    float magnitude = sqrtf(nx * nx + ny * ny);

    // Normaliza para [0.0, 1.0]
    magnitude /= sqrtf(2.0f); // Máxima distância é sqrt(2) (diagonal)

    // Garante que não exceda 1.0
    if (magnitude > 1.0f)
        magnitude = 1.0f;

    return magnitude;
}

/**
 * @brief Obtém os valores normalizados dos eixos X e Y
 *
 * @param x Ponteiro para armazenar valor X normalizado (-1.0 a 1.0)
 * @param y Ponteiro para armazenar valor Y normalizado (-1.0 a 1.0)
 */
void get_joystick_normalized(float *x, float *y)
{
    if (x == NULL || y == NULL)
        return;

    uint16_t raw_x = get_joystick_x();
    uint16_t raw_y = get_joystick_y();

    // Normaliza para [-1.0, 1.0]
    *x = (float)(raw_x - ADC_CENTER) / ADC_CENTER;
    *y = (float)(raw_y - ADC_CENTER) / ADC_CENTER;

    // Limita ao intervalo [-1.0, 1.0]
    if (*x > 1.0f)
        *x = 1.0f;
    if (*x < -1.0f)
        *x = -1.0f;
    if (*y > 1.0f)
        *y = 1.0f;
    if (*y < -1.0f)
        *y = -1.0f;
}
