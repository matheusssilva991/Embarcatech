#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"

// Protótipos
void init_led_pins();
void init_matrix_pins();
void init_buzzer();
void play_buzzer(uint freq, uint duration_ms);
void put_led_rgb(int red, int green, int blue);
char get_pressed_key();
void debounce_delay();
void police_siren_effect();

// Constantes
#define LED_RED_PIN 13
#define LED_GREEN_PIN 11
#define LED_BLUE_PIN 12
#define BUZZER_FREQUENCY 100
#define BUZZER_PIN 21
#define CLK_DIV 4.0f

// Definir variáveis do teclado matricial
int rows[4] = {8, 7, 6, 5};
int columns[4] = {4, 3, 2, 28};

char keypad[4][4] = {{'1', '2', '3', 'A'},
                     {'4', '5', '6', 'B'},
                     {'7', '8', '9', 'C'},
                     {'*', '0', '#', 'D'}};

int main()
{
    char key;

    init_led_pins(); // Inicializar GPIOs do LED RGB
    init_matrix_pins(); // chama a função do GPIO do teclado matricial
    init_buzzer(); // Inicializa o Buzzer

    stdio_init_all();

    while (true)
    {
        // Ler tecla pressionada
        key = get_pressed_key();

        if (key != '\0')
        {
            switch (key)
            {

            case '0':
                printf("Tecla pressionada: %c\n", key);
                put_led_rgb(0, 0, 0);
                sleep_ms(200);
                break;

            case '1':
                printf("Tecla pressionada: %c\n", key);
                put_led_rgb(1, 0, 0);
                sleep_ms(200);
                break;

            case '2':
                printf("Tecla pressionada: %c\n", key);
                put_led_rgb(0, 1, 0);
                sleep_ms(200);
                break;

            case '3':
                printf("Tecla pressionada: %c\n", key);
                put_led_rgb(0, 0, 1);
                sleep_ms(200);
                break;

            case '4':
                printf("Tecla pressionada: %c\n", key);
                put_led_rgb(1, 1, 0);
                sleep_ms(200);
                break;

            case '5':
                printf("Tecla pressionada: %c\n", key);
                put_led_rgb(1, 0, 1);
                sleep_ms(200);
                break;

            case '6':
                printf("Tecla pressionada: %c\n", key);
                put_led_rgb(0, 1, 1);
                sleep_ms(200);
                break;
            case '7':
                printf("Tecla pressionada: %c\n", key);
                put_led_rgb(1, 1, 1);
                sleep_ms(200);
                break;

            case '8':
                printf("Tecla pressionada: %c\n", key);
                sleep_ms(200);
                break;

            case '9':
                printf("Tecla pressionada: %c\n", key);
                sleep_ms(200);
                break;

            case 'A':
                police_siren_effect(); // Aciona efeito de sirene de polícia
                break;

            case 'B':
                // Implementar função correspondente
                break;

            case 'C':
                // Implementar função correspondente
                break;

            case 'D':
                // Implementar função correspondente
                break;

            case '#':
                play_buzzer(BUZZER_FREQUENCY, 500); // Bipe do buzzer de 500ms
                break;

            case '*':
                // Implementar função correspondente
                break;

            default:
                printf("Tecla não reconhecida ou erro na operação!\n");
                break;
            }
        }
    }
}

void debounce_delay() {
    sleep_ms(50); // Delay de 50ms
}

// Essa função inicializa os pinos GPIO do led RGB
void init_led_pins() {
    gpio_init(LED_RED_PIN);
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_init(LED_GREEN_PIN);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_init(LED_BLUE_PIN);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);
}

/*Essa função inicializa os pinos GPIO do teclado matricial e
Ativa resistores pull-down nos pinos de entrada para evitar leituras flutuantes */
void init_matrix_pins()
{
    for (int i = 0; i < 4; i++)
    {
        gpio_init(rows[i]);
        gpio_set_dir(rows[i], GPIO_OUT);
        gpio_put(rows[i], 0);
    }

    for (int j = 0; j < 4; j++)
    {
        gpio_init(columns[j]);
        gpio_set_dir(columns[j], GPIO_IN);
        gpio_pull_down(columns[j]);
    }
}

// Função para inicializar o Buzzer
void init_buzzer() {
    // Inicializar GPIOs do Buzzer
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM); // Configura o pino como PWM

    // Slice PWM do pino
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);

    // Configuração do PWM
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0f); // Definindo o divisor de clock
    pwm_init(slice_num, &config, true);

    // Define o PWM inicialmente no nível baixo
    pwm_set_gpio_level(BUZZER_PIN, 0);
}

// Função para tocar o Buzzer
void play_buzzer(uint freq, uint duration_ms)
{
    if (freq == 0) {
        printf("Frequência inválida: %u Hz\n", freq);
        return; // Evita divisão por zero ou comportamento inesperado
    }

    // Obtém o slice PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(BUZZER_PIN);

    // Frequência do PWM pela BUZZER_FREQUENCY
    uint clock_freq = clock_get_hz(clk_sys); // Frequência do clock                   // Divisor do clock
    uint top = clock_freq / (freq * CLK_DIV); // Cálculo do TOP

    pwm_set_wrap(slice_num, top);
    pwm_set_gpio_level(BUZZER_PIN, top / 2);

    // Duração do som emitido
    sleep_ms(duration_ms);

    // Desliga o Buzzer
    pwm_set_gpio_level(BUZZER_PIN, 0);

    sleep_ms(100); // Entrepausa de 100ms
}

// Efeito de sirene de polícia
void police_siren_effect() {
    for (int i = 0; i < 5; i++) {
        put_led_rgb(1, 0, 0); // LED vermelho
        play_buzzer(440, 200); // Som agudo
        put_led_rgb(0, 0, 1); // LED azul
        play_buzzer(220, 200); // Som grave
    }
    put_led_rgb(0, 0, 0); // Desliga LEDs
}

// Função para ligar/desligar o LED RGB
void put_led_rgb(int red, int green, int blue)
{
    gpio_put(LED_RED_PIN, red);
    gpio_put(LED_GREEN_PIN, green);
    gpio_put(LED_BLUE_PIN, blue);
}

// Essa função verifica o teclado matricial para detectar qual tecla foi pressionada.
char get_pressed_key() {
    for (int i = 0; i < 4; i++) {
        gpio_put(rows[i], 1);
        debounce_delay();
        for (int j = 0; j < 4; j++) {
            if (gpio_get(columns[j])) {
                gpio_put(rows[i], 0);
                return keypad[i][j];
            }
        }
        gpio_put(rows[i], 0);
    }
    return '\0';
}

