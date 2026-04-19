#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/watchdog.h"         // Biblioteca para uso de WatchDog
#include "hardware/structs/watchdog.h"
#include "pico/bootrom.h"
#include "pico/multicore.h" // Biblioteca para uso dos dois núcleos
#include "pico/sync.h"      // Biblioteca para sincronização (Mutex)
#include <math.h>

#include "pico/cyw43_arch.h" // Biblioteca para arquitetura Wi-Fi da Pico com CYW43
#include "lwip/tcp.h"

#include "lib/led/led.h"
#include "lib/button/button.h"
#include "lib/ws2812b/ws2812b.h"
#include "lib/buzzer/buzzer.h"
#include "lib/aht20/aht20.h"
#include "lib/bmp280/bmp280.h"

#include "config/wifi_config.h"
#include "public/html_data.h"
#include "tflm_wrapper.h"

// Definições de Checkpoints para diagnóstico
#define CHECKPOINT_INIT      0x11  // Código para falha na inicialização
#define CHECKPOINT_WIFI      0x22  // Código para falha em funções de rede
#define CHECKPOINT_SENSOR    0x33  // Código para falha na leitura I2C
#define CHECKPOINT_AI        0x44  // Código para falha no cálculo da IA
#define ERRO_FALHA_CRITICA   0xDEAD // Código para falha crítica simulada


#define I2C0_PORT i2c0              // i2c0 pinos 0 e 1
#define I2C0_SDA 0                  // 0
#define I2C0_SCL 1                  // 1
#define I2C1_PORT i2c1              // i2c1 pinos 2 e 3
#define I2C1_SDA 2                  // 2
#define I2C1_SCL 3                  // 3
#define SEA_LEVEL_PRESSURE 101325.0 // Pressão ao nível do mar em Pa

// Constantes de limites climáticos
#define TEMP_HOT_THRESHOLD 30.0f
#define TEMP_VERY_HOT_THRESHOLD 50.0f
#define TEMP_COLD_THRESHOLD 15.0f
#define TEMP_VERY_COLD_THRESHOLD 5.0f
#define HUMIDITY_HIGH_THRESHOLD 80.0f
#define HUMIDITY_LOW_THRESHOLD 20.0f

// Constantes de temporização
#define BUTTON_DEBOUNCE_MS 300
#define SENSOR_READ_INTERVAL_MS 1000
#define WIFI_CHECK_INTERVAL_MS 30000
#define WIFI_RETRY_DELAY_MS 5000
#define ALERT_BUZZER_DURATION_MS 250
#define RAIN_LOG_INTERVAL_MS 5000 

// Estrutura para controle da simulação
typedef struct {
    bool active;       
    float temperature;
    float humidity;
    float pressure;
} sim_data_t;

// Variáveis globais de simulação e controle de concorrência
volatile sim_data_t global_sim_data = {false, 0.0f, 0.0f, 0.0f};
mutex_t sim_mutex; // Protege leitura/escrita dos dados de simulação
mutex_t ai_mutex;  // Protege o acesso ao interpretador TensorFlow

// Tipos de dados
struct http_state
{
    char response[4096]; // Buffer apenas para cabecalhos e respostas pequenas
    size_t len;
    size_t header_len;
    size_t sent;
    const char *stream_ptr;
    size_t stream_len;
    size_t stream_sent;
};

static struct http_state http_state_singleton;
static volatile bool http_state_in_use = false;

typedef struct weather_data
{
    float temperature;
    float humidity;
    float pressure;
    float altitude;
    float rain_probability;
    int minTemperature;
    int maxTemperature;
    float offsetTemperature;
} weather_data_t;

// Prototipos
double calculate_altitude(double pressure);
void check_alerts();
void check_climate_conditions();
static void update_temperature_leds(float temp);
static void update_humidity_leds(float humidity);
static err_t http_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err);
static void start_http_server(void);
void gpio_irq_handler(uint gpio, uint32_t events);
bool try_wifi_connect(void);
static bool wait_for_wifi_connection(uint32_t max_attempts);
void core1_entry(); // Função principal do Core 1
void clear_uart_buffer();

// Variáveis globais
static weather_data_t weather_data = {0.0f, 0.0f, 0.0f, 0.0f, 10, 70, 0.0f}; // Dados climáticos e limites iniciais
static volatile int64_t last_button_a_press_time = 0;                        // Tempo do último pressionamento de botão A
static volatile int64_t last_button_b_press_time = 0;                        // Tempo do último pressionamento de botão B
static volatile int64_t last_button_sw_press_time = 0;                       // Tempo do último pressionamento do botão SW
static volatile bool is_alert_active = true;                                 // Flag para indicar se o alerta está ativo
static volatile bool wifi_connected = false;
static volatile bool server_started = false;
static uint64_t last_wifi_check = 0;
static uint64_t last_sensor_read = 0; // Tempo da última leitura de sensores
static volatile bool btn_a_event = false;
static volatile bool btn_b_event = false;

// Configurações da IA
#define RAIN_WINDOW_SIZE 24
#define RAIN_FEATURES 6
#define RAIN_HISTORY_SIZE 600
#define RAIN_DELTA_STEPS 600

static volatile int8_t *rain_input_ptr = NULL;
static volatile int8_t *rain_output_ptr = NULL;
static volatile int rain_input_bytes = 0;
static volatile int rain_output_bytes = 0;
static volatile float rain_in_scale = 1.0f;
static volatile int rain_in_zp = 0;
static volatile float rain_out_scale = 1.0f;
static volatile int rain_out_zp = 0;
static volatile bool rain_model_ready = false;
static const char * volatile rain_status = "not_ready";

static float feature_window[RAIN_WINDOW_SIZE][RAIN_FEATURES];
static int feature_index = 0;
static int feature_count = 0;
static float history_temperature[RAIN_HISTORY_SIZE];
static float history_humidity[RAIN_HISTORY_SIZE];
static float history_pressure[RAIN_HISTORY_SIZE];
static int history_index = 0;
static int history_count = 0;

static const float feature_mean[RAIN_FEATURES] = {
    2.37319765e+01f, 8.33860739e+01f, 1.00486506e+03f,
    -5.29792270e-05f, 3.22167255e-05f, -8.59112680e-05f};
static const float feature_scale[RAIN_FEATURES] = {
    3.36597984f, 14.15288448f, 5.94865175f,
    1.20307008f, 2.08927441f, 9.28103259f};

static float standardize_feature(int idx, float value)
{
    return (value - feature_mean[idx]) / feature_scale[idx];
}

static int8_t quantize_f32_to_i8(float x, float scale, int zp)
{
    long q = lroundf(x / scale) + zp;
    if (q < -128) q = -128;
    if (q > 127) q = 127;
    return (int8_t)q;
}

static void update_rain_feature_window(float temperature, float humidity, float pressure)
{
    float delta_pressure = 0.0f;
    float delta_temperature = 0.0f;
    float delta_humidity = 0.0f;

    if (history_count > RAIN_DELTA_STEPS)
    {
        int past_index = history_index - RAIN_DELTA_STEPS;
        if (past_index < 0)
            past_index += RAIN_HISTORY_SIZE;

        delta_pressure = pressure - history_pressure[past_index];
        delta_temperature = temperature - history_temperature[past_index];
        delta_humidity = humidity - history_humidity[past_index];
    }

    history_pressure[history_index] = pressure;
    history_temperature[history_index] = temperature;
    history_humidity[history_index] = humidity;
    history_index = (history_index + 1) % RAIN_HISTORY_SIZE;
    if (history_count < RAIN_HISTORY_SIZE)
        history_count++;

    feature_window[feature_index][0] = standardize_feature(0, temperature);
    feature_window[feature_index][1] = standardize_feature(1, humidity);
    feature_window[feature_index][2] = standardize_feature(2, pressure);
    feature_window[feature_index][3] = standardize_feature(3, delta_pressure);
    feature_window[feature_index][4] = standardize_feature(4, delta_temperature);
    feature_window[feature_index][5] = standardize_feature(5, delta_humidity);

    feature_index = (feature_index + 1) % RAIN_WINDOW_SIZE;
    if (feature_count < RAIN_WINDOW_SIZE)
        feature_count++;
}

static void run_rain_inference(void)
{
    const uint64_t now_ms = to_ms_since_boot(get_absolute_time());
    static uint64_t last_rain_log_ms = 0;

    // Checagem 1: O Core 0 já inicializou a IA?
    if (!rain_model_ready)
    {
        rain_status = "not_ready";
        if (now_ms - last_rain_log_ms >= RAIN_LOG_INTERVAL_MS)
        {
            printf("[IA] Modelo ainda nao inicializado pelo Core 0.\n");
            last_rain_log_ms = now_ms;
        }
        return;
    }

    // Checagem 2: Temos histórico suficiente (24 leituras)?
    if (feature_count < RAIN_WINDOW_SIZE)
    {
        rain_status = "warming_up";
        if (now_ms - last_rain_log_ms >= RAIN_LOG_INTERVAL_MS)
        {
            printf("[IA] Coletando dados: %d/%d (Aguarde...)\n", feature_count, RAIN_WINDOW_SIZE);
            last_rain_log_ms = now_ms;
        }
        return;
    }

    // Checagem 3: Os buffers estão integros?
    int expected_bytes = RAIN_WINDOW_SIZE * RAIN_FEATURES;
    // Nota: rain_input_bytes é volatile, acesso direto ok
    if (rain_input_bytes < expected_bytes || rain_output_bytes < 1)
    {
        rain_status = "invalid_io";
        if (now_ms - last_rain_log_ms >= RAIN_LOG_INTERVAL_MS)
        {
            printf("[IA] Erro de Buffer: In=%d, Out=%d\n", rain_input_bytes, rain_output_bytes);
            last_rain_log_ms = now_ms;
        }
        return;
    }

    // Preenchimento do Buffer (Lógica original mantida)
    int start = feature_index;
    int offset = 0;
    for (int i = 0; i < RAIN_WINDOW_SIZE; i++)
    {
        int idx = (start + i) % RAIN_WINDOW_SIZE;
        for (int f = 0; f < RAIN_FEATURES; f++)
        {
            float value = feature_window[idx][f];
            rain_input_ptr[offset++] = quantize_f32_to_i8(value, rain_in_scale, rain_in_zp);
        }
    }

    // Execução e Resultado
    if (tflm_invoke() == 0)
    {
        int8_t q = rain_output_ptr[0];
        float prob = (float)(q - rain_out_zp) * rain_out_scale;
        if (prob < 0.0f) prob = 0.0f;
        if (prob > 1.0f) prob = 1.0f;
        
        weather_data.rain_probability = prob;
        rain_status = "ok"; // Status oficial: Tudo funcionando
    }
    else
    {
        rain_status = "invoke_error";
        if (now_ms - last_rain_log_ms >= RAIN_LOG_INTERVAL_MS)
        {
            printf("[IA] Falha crítica ao rodar tflm_invoke()\n");
            last_rain_log_ms = now_ms;
        }
    }
}

// Executa uma inferência isolada para teste, sem alterar o histórico global
static void run_manual_inference_test(float temp, float hum, float press) {
    if (!rain_model_ready) {
        printf("ERRO: Modelo IA nao inicializado.\n");
        return;
    }

    // Bloqueia o uso da IA pelo Core 0 para não dar conflito no buffer
    mutex_enter_blocking(&ai_mutex);

    // Prepara features temporárias (Delta = 0 para teste pontual)
    float features[RAIN_FEATURES];
    features[0] = standardize_feature(0, temp);
    features[1] = standardize_feature(1, hum);
    features[2] = standardize_feature(2, press);
    features[3] = standardize_feature(3, 0.0f); 
    features[4] = standardize_feature(4, 0.0f); 
    features[5] = standardize_feature(5, 0.0f); 

    // Preenche buffer de entrada
    int offset = 0;
    for (int i = 0; i < RAIN_WINDOW_SIZE; i++) {
        for (int f = 0; f < RAIN_FEATURES; f++) {
            rain_input_ptr[offset++] = quantize_f32_to_i8(features[f], rain_in_scale, rain_in_zp);
        }
    }

    // Roda a inferência
    if (tflm_invoke() == 0) {
        int8_t q = rain_output_ptr[0];
        float prob = (float)(q - rain_out_zp) * rain_out_scale;
        if (prob < 0.0f) prob = 0.0f; if (prob > 1.0f) prob = 1.0f;
        
        printf("\n>>> RESULTADO DO TESTE DE IA <<<\n");
        printf("Entrada: T=%.2f C, H=%.2f %%, P=%.2f hPa\n", temp, hum, press);
        printf("Previsao de Chuva: %.1f %% \n", prob * 100.0f);
        printf("------------------------------\n");
    } else {
        printf("Erro ao invocar o modelo TensorFlow.\n");
    }

    mutex_exit(&ai_mutex);
}

// Limpa buffer do serial para evitar leituras incorretas
void clear_uart_buffer() {
    int c;
    while ((c = getchar_timeout_us(0)) != PICO_ERROR_TIMEOUT);
}

// --------------------------------------------------------------------------
// CORE 1: Terminal de Comandos (Entrada de Dados)
// --------------------------------------------------------------------------
void core1_entry() {
    char cmd;
    float in_t, in_h, in_p;

    while (true) {
        // Bloqueia esperando input do usuário
        if (scanf(" %c", &cmd) == 1) {
            
            if (cmd == 'S' || cmd == 's') { // Simulação Fixa
                if (scanf("%f %f %f", &in_t, &in_h, &in_p) == 3) {
                    mutex_enter_blocking(&sim_mutex);
                    global_sim_data.active = true;
                    global_sim_data.temperature = in_t;
                    global_sim_data.humidity = in_h;
                    global_sim_data.pressure = in_p;
                    mutex_exit(&sim_mutex);
                    printf(">>> SIMULACAO ATIVADA: T=%.2f H=%.2f P=%.2f <<<\n", in_t, in_h, in_p);
                } else {
                    printf("Erro formato. Use: S [temp] [hum] [press]\n");
                    clear_uart_buffer();
                }
            } 
            else if (cmd == 'R' || cmd == 'r') { // Modo Real
                mutex_enter_blocking(&sim_mutex);
                global_sim_data.active = false;
                mutex_exit(&sim_mutex);
                printf(">>> MODO SENSORES REAIS <<<\n");
            } 
            else if (cmd == 'P' || cmd == 'p') { // Previsão Pontual (Teste)
                if (scanf("%f %f %f", &in_t, &in_h, &in_p) == 3) {
                    printf("Calculando previsao...\n");
                    run_manual_inference_test(in_t, in_h, in_p);
                } else {
                    printf("Erro formato. Use: P [temp] [hum] [press]\n");
                    clear_uart_buffer();
                }
            }
            else if (cmd == 'K' || cmd == 'k') { 
                printf("Simulando travamento (KILL)...\n");
                watchdog_hw->scratch[4] = ERRO_FALHA_CRITICA; // Grava erro manual
                mutex_enter_blocking(&ai_mutex); // Trava o Core 0
                while(true); // Trava o Core 1
            }

            else {
                printf("Comando invalido: %c\n", cmd);
                clear_uart_buffer();
            }
        }
    }
}

// --------------------------------------------------------------------------
// CORE 0: Principal (Sensores, Wi-Fi, Controle)
// --------------------------------------------------------------------------
int main()
{
    stdio_init_all();

    // --- BLOCO DE DIAGNÓSTICO DO WATCHDOG ---
    if (watchdog_caused_reboot()) {
        printf("\n!!! SISTEMA RECUPERADO DE TRAVAMENTO (WATCHDOG) !!!\n");
        
        // Lê o registrador Scratch 4 para saber onde estava antes de morrer
        uint32_t motivo = watchdog_hw->scratch[4];
        
        printf("Diagnostico: ");
        switch(motivo) {
            case CHECKPOINT_WIFI:
                printf("O sistema travou durante operacoes de WI-FI/REDE.\n");
                break;
            case CHECKPOINT_SENSOR:
                printf("O sistema travou durante a leitura dos SENSORES (Possivel falha I2C).\n");
                break;
            case CHECKPOINT_AI:
                printf("O sistema travou durante a inferencia da INTELIGENCIA ARTIFICIAL.\n");
                break;
            case CHECKPOINT_INIT:
                printf("O sistema travou durante a INICIALIZACAO (Possivel falha de hardware ou configuracao).\n");
                break;
            case ERRO_FALHA_CRITICA:
                printf("O sistema travou devido a uma FALHA CRITICA SIMULADA.\n");
                break;
            default:
                 printf("Causa desconhecida (Watchdog padrao).\n");
        }
        
        // Limpa o registrador para não confundir no próximo reset
        watchdog_hw->scratch[4] = 0; 
        sleep_ms(3000); // Pausa para leitura do terminal
    }


    // Inicialização dos Mutexes
    mutex_init(&sim_mutex);
    mutex_init(&ai_mutex);

    init_btns();
    init_leds_pwm();
    ws2812b_init();

    init_buzzer(BUZZER_A_PIN, 4.0f); // Inicializa o buzzer A
    init_buzzer(BUZZER_B_PIN, 4.0f); // Inicializa o buzzer B

    gpio_set_irq_enabled_with_callback(BTN_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled(BTN_B_PIN, GPIO_IRQ_EDGE_FALL, true);

    // Inicializa o I2C para o SMP280
    printf("Inicializando I2C0 (BMP280)...\n");
    i2c_init(I2C0_PORT, 400 * 1000);
    gpio_set_function(I2C0_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA);
    gpio_pull_up(I2C0_SCL);
    printf("I2C0 inicializado\n");

    // Inicializa o I2C para o AHT20
    printf("Inicializando I2C1 (AHT20)...\n");
    i2c_init(I2C1_PORT, 400 * 1000);
    gpio_set_function(I2C1_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C1_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C1_SDA);
    gpio_pull_up(I2C1_SCL);
    printf("I2C1 inicializado\n");

    // Inicializa o BMP280
    printf("Inicializando BMP280...\n");
    bmp280_init(I2C0_PORT);
    struct bmp280_calib_param params;
    bmp280_get_calib_params(I2C0_PORT, &params);
    printf("BMP280 inicializado com parametros de calibracao\n");

    // Inicializa o AHT20
    printf("Inicializando AHT20...\n");
    aht20_reset(I2C1_PORT);
    aht20_init(I2C1_PORT);
    printf("AHT20 inicializado\n");

    int tflm_rc = tflm_init();
    if (tflm_rc == 0)
    {
        rain_input_ptr = (volatile int8_t*)tflm_input_ptr((int*)&rain_input_bytes);
        rain_output_ptr = (volatile int8_t*)tflm_output_ptr((int*)&rain_output_bytes);
        rain_in_scale = tflm_input_scale();
        rain_in_zp = tflm_input_zero_point();
        rain_out_scale = tflm_output_scale();
        rain_out_zp = tflm_output_zero_point();
        if (rain_input_ptr && rain_output_ptr)
            rain_model_ready = true;
    }

    if (cyw43_arch_init())
    {
        printf("Falha ao inicializar a arquitetura CYW43\n ");
        set_led_red_pwm(); // LED vermelho para falha de inicialização
        sleep_ms(2000);
        return 1;
    }

    cyw43_arch_enable_sta_mode();

    // Mensagens de inicialização do Terminal
    printf("\n--- SISTEMA METEOROLOGICO ---\n");
    printf(" 'S [T] [H] [P]' -> Simular (Fixa valores)\n");
    printf(" 'P [T] [H] [P]' -> Prever (Teste rapido IA)\n");
    printf(" 'R'             -> Voltar ao Real\n");

    // Lança o Core 1 para processar entradas do terminal em paralelo
    multicore_launch_core1(core1_entry);

    // Loop até conseguir conectar ao Wi-Fi pela primeira vez
    wifi_connected = wait_for_wifi_connection(0); // 0 = ilimitado

    sleep_ms(10000);

    // Exibe o IP do servidor
    uint8_t *ip = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
    printf("\n========================================\n");
    printf("SERVIDOR HTTP INICIADO\n");
    printf("IP: http://%d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
    printf("Porta: 80\n");
    printf("========================================\n\n");

    // Só inicia o servidor HTTP após conectar ao Wi-Fi
    start_http_server();
    server_started = true;

    // --- ATIVAÇÃO DO WATCHDOG ---
    // O sistema tem 8000ms para chamar watchdog_update() ou será reiniciado
    watchdog_enable(8000, 1); 
    printf("Monitoramento Watchdog Ativo (Timeout: 8s)\n");

    // Estruturas para leitura de sensores
    AHT20_Data data;
    int32_t raw_temp_bmp;
    int32_t raw_pressure;
    double altitude;
    uint64_t current_time;

    // Loop principal
    while (true)
    {
        // Alimenta o Watchdog (Reset do timer para 8s)
        watchdog_update();

        // Marca Checkpoint: Entrando em operações de rede
        watchdog_hw->scratch[4] = CHECKPOINT_WIFI;

        current_time = to_ms_since_boot(get_absolute_time());
        cyw43_arch_poll(); // Mantém o Wi-Fi funcionando

        if (btn_a_event)
        {
            btn_a_event = false;
            printf("Entrando em modo BOOTSEL...\n");
            sleep_ms(100);
            reset_usb_boot(0, 0);
        }
        if (btn_b_event)
        {
            btn_b_event = false;
            weather_data.minTemperature = 10;
            weather_data.maxTemperature = 70;
            weather_data.offsetTemperature = 0.0f;
            printf("Limites resetados: Min=10C, Max=70C, Offset=0.0\n");
        }

        // Verifica periodicamente o status da conexão Wi-Fi (a cada 30 segundos)
        if (current_time - last_wifi_check >= WIFI_CHECK_INTERVAL_MS)
        {
            last_wifi_check = current_time;

            // Verifica o status do link Wi-Fi
            if (cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA) != CYW43_LINK_UP)
            {
                printf("Conexão Wi-Fi perdida! Tentando reconectar...\n");
                wifi_connected = false;

                // Tenta reconectar
                wifi_connected = wait_for_wifi_connection(5); // Máximo 5 tentativas

                // Reinicia o servidor HTTP se necessário
                if (wifi_connected && !server_started)
                {
                    start_http_server();
                    server_started = true;
                }
            }
        }

        // Leitura de sensores com intervalo não-bloqueante
        if (current_time - last_sensor_read >= SENSOR_READ_INTERVAL_MS)
        {
            last_sensor_read = current_time;

            bool use_sim = false;
            sim_data_t current_sim;
            
            // Verifica se deve usar simulação protegendo leitura com Mutex
            mutex_enter_blocking(&sim_mutex);
            use_sim = global_sim_data.active;
            current_sim = (sim_data_t)global_sim_data;
            mutex_exit(&sim_mutex);

            if (use_sim) {
                // Modo Simulação
                weather_data.temperature = current_sim.temperature;
                weather_data.humidity = current_sim.humidity;
                weather_data.pressure = current_sim.pressure;
                // Adiciona ruído leve
                weather_data.pressure += ((rand() % 10) - 5) / 100.0f;
            } 
            else {

                // Antes de começar a usar o barramento I2C (bmp280 e aht20), atualizamos o código de erro
                // Se um fio soltar, o código trava aqui.
                watchdog_hw->scratch[4] = CHECKPOINT_SENSOR;

                // Modo Real: Leitura do BMP280
                bmp280_read_raw(I2C0_PORT, &raw_temp_bmp, &raw_pressure);

                weather_data.temperature = bmp280_convert_temp(raw_temp_bmp, &params);
                weather_data.temperature /= 100.0; // Converte para Celsius

                weather_data.pressure = bmp280_convert_pressure(raw_pressure, raw_temp_bmp, &params);
                weather_data.pressure /= 100.0; // Converte para hPa
                
                // Leitura do AHT20
                if (aht20_read(I2C1_PORT, &data))
                {
                    weather_data.humidity = data.humidity;
                }
                else
                {
                    printf("Erro na leitura do AHT20!\n");
                    weather_data.humidity = 0.0; // Valor padrão em caso de erro
                }
            }

            weather_data.altitude = calculate_altitude(weather_data.pressure * 100.0);

            // Antes de bloquear o Mutex da IA e rodar a inferência, atualiza o código de erro para diagnóstico
            watchdog_hw->scratch[4] = CHECKPOINT_AI;

            // Protege o uso da IA com Mutex para evitar conflito com o comando 'P'
            mutex_enter_blocking(&ai_mutex);
            update_rain_feature_window(weather_data.temperature, weather_data.humidity, weather_data.pressure);
            run_rain_inference();
            mutex_exit(&ai_mutex);

            printf("[%s] Previsao chuva: %.3f (T=%.2fC, U=%.2f%%, P=%.2fhPa)\n",
                   use_sim ? "SIM" : "REAL",
                   weather_data.rain_probability,
                   weather_data.temperature,
                   weather_data.humidity,
                   weather_data.pressure);

            // Verifica os alertas
            check_alerts();

            // Verifica as condições climáticas
            check_climate_conditions();
        }

        sleep_ms(10); // Pequeno delay para não sobrecarregar a CPU

        // Se chegou aqui, o loop rodou com sucesso. Zeramos o erro.
        watchdog_hw->scratch[4] = 0;
    }
    cyw43_arch_deinit(); // Esperamos que nunca chegue aqui
}

// Função para calcular a altitude a partir da pressão atmosférica
double calculate_altitude(double pressure)
{
    return 44330.0 * (1.0 - pow(pressure / SEA_LEVEL_PRESSURE, 0.1903));
}

// Função para verificar os alertas de temperatura
void check_alerts()
{
    if (is_alert_active)
    {
        float adjusted_temp = weather_data.temperature + weather_data.offsetTemperature;

        if (adjusted_temp > weather_data.maxTemperature)
        {
            printf("Alerta: Temperatura acima do limite! (%.2f°C > %d°C)\n",
                   adjusted_temp, weather_data.maxTemperature);
            play_tone(BUZZER_A_PIN, 700); // Toca o buzzer A
            sleep_ms(ALERT_BUZZER_DURATION_MS);
            stop_tone(BUZZER_A_PIN); // Para o buzzer A
        }
        else if (adjusted_temp < weather_data.minTemperature)
        {
            printf("Alerta: Temperatura abaixo do limite! (%.2f°C < %d°C)\n",
                   adjusted_temp, weather_data.minTemperature);
            play_tone(BUZZER_B_PIN, 400); // Toca o buzzer B
            sleep_ms(ALERT_BUZZER_DURATION_MS);
            stop_tone(BUZZER_B_PIN); // Para o buzzer B
        }
    }
}

// Atualiza os LEDs da matriz WS2812B baseado na temperatura
static void update_temperature_leds(float temp)
{
    bool is_hot = temp > TEMP_HOT_THRESHOLD;
    bool is_very_hot = temp > TEMP_VERY_HOT_THRESHOLD;
    bool is_cold = temp < TEMP_COLD_THRESHOLD;
    bool is_very_cold = temp < TEMP_VERY_COLD_THRESHOLD;

    ws2812b_clear(); // Limpa os LEDs

    if (is_hot)
    {
        ws2812b_fill_row(0, (int[]){0, 0, 32}); // Azul forte
        ws2812b_fill_row(1, (int[]){0, 0, 16}); // Azul fraco
        ws2812b_fill_row(2, (int[]){0, 32, 0}); // Verde
        ws2812b_fill_row(3, (int[]){32, 0, 0}); // Vermelho fraco

        if (is_very_hot)
        {
            ws2812b_fill_row(4, (int[]){32, 0, 0}); // Vermelho forte
        }
    }
    else if (is_cold)
    {
        if (is_very_cold)
        {
            ws2812b_fill_row(0, (int[]){0, 0, 32}); // Azul forte
        }
        else
        {
            ws2812b_fill_row(0, (int[]){0, 0, 32}); // Azul forte
            ws2812b_fill_row(1, (int[]){0, 0, 16}); // Azul fraco
        }
    }
    else // Temperatura normal
    {
        ws2812b_fill_row(0, (int[]){0, 0, 32}); // Azul forte
        ws2812b_fill_row(1, (int[]){0, 0, 16}); // Azul fraco
        ws2812b_fill_row(2, (int[]){0, 32, 0}); // Verde
    }

    ws2812b_write(); // Atualiza a matriz de LEDs
}

// Atualiza o LED RGB PWM baseado na umidade
static void update_humidity_leds(float humidity)
{
    if (humidity > HUMIDITY_HIGH_THRESHOLD)
    {
        set_led_blue_pwm(); // LED azul para clima úmido
    }
    else if (humidity < HUMIDITY_LOW_THRESHOLD)
    {
        set_led_red_pwm(); // LED vermelho para clima seco
    }
    else
    {
        set_led_green_pwm(); // LED verde para clima normal
    }
}

// Função para verificar as condições climáticas
void check_climate_conditions()
{
    update_temperature_leds(weather_data.temperature);
    update_humidity_leds(weather_data.humidity);
}

// Função de callback para enviar dados HTTP
static err_t http_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    struct http_state *hs = (struct http_state *)arg;
    hs->sent += len;
    if (hs->stream_ptr && hs->stream_sent < hs->stream_len)
    {
        size_t remaining = hs->stream_len - hs->stream_sent;
        size_t max_chunk = tcp_sndbuf(tpcb);
        if (max_chunk > 512)
            max_chunk = 512;
        if (max_chunk == 0)
            return ERR_OK;
        size_t chunk = remaining > max_chunk ? max_chunk : remaining;
        err_t err = tcp_write(tpcb, hs->stream_ptr + hs->stream_sent, chunk, TCP_WRITE_FLAG_COPY);
        if (err == ERR_OK)
        {
            hs->stream_sent += chunk;
            tcp_output(tpcb);
        }
        else if (err == ERR_MEM)
        {
            return ERR_OK;
        }
        else
        {
            printf("ERRO: tcp_write falhou ao enviar HTML (err=%d)\n", err);
            tcp_close(tpcb);
            http_state_in_use = false;
        }
        return ERR_OK;
    }
    if (hs->sent >= hs->len)
    {
        tcp_close(tpcb);
        http_state_in_use = false;
    }
    return ERR_OK;
}

// Função de recebimento HTTP
static err_t http_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    if (!p)
    {
        tcp_close(tpcb);
        return ERR_OK;
    }

    char *req = (char *)p->payload;
    if (http_state_in_use)
    {
        pbuf_free(p);
        tcp_close(tpcb);
        return ERR_MEM;
    }
    http_state_in_use = true;
    struct http_state *hs = &http_state_singleton;
    hs->sent = 0;
    hs->len = 0;
    hs->header_len = 0;
    hs->stream_ptr = NULL;
    hs->stream_len = 0;
    hs->stream_sent = 0;

    if (strstr(req, "POST /api/limits"))
    {
        char *body = strstr(req, "\r\n\r\n");
        if (body)
        {
            body += 4;
            int max_val, min_val;
            float offset_val = 0.0f;

            if (sscanf(body, "{\"min\":%d,\"max\":%d,\"offset\":%f", &min_val, &max_val, &offset_val) == 3)
            {
                // Validação dos valores recebidos
                bool valid = true;
                const char *error_msg = "OK";

                if (max_val < 0 || max_val > 100)
                {
                    valid = false;
                    error_msg = "Temperatura máxima inválida (0-100)";
                }
                else if (min_val < -50 || min_val > 50)
                {
                    valid = false;
                    error_msg = "Temperatura mínima inválida (-50 a 50)";
                }
                else if (min_val >= max_val)
                {
                    valid = false;
                    error_msg = "Temperatura mínima deve ser menor que máxima";
                }
                else if (offset_val < -50.0f || offset_val > 50.0f)
                {
                    valid = false;
                    error_msg = "Offset inválido (-50.0 a 50.0)";
                }

                if (valid)
                {
                    weather_data.maxTemperature = max_val;
                    weather_data.minTemperature = min_val;
                    weather_data.offsetTemperature = offset_val;
                    printf("Limites atualizados: Max=%d, Min=%d, Offset=%.2f\n",
                           max_val, min_val, offset_val);
                }
                else
                {
                    printf("Erro na validação: %s\n", error_msg);
                }

                const char *txt = valid ? "Limites atualizados" : error_msg;
                hs->len = snprintf(hs->response, sizeof(hs->response),
                                   "HTTP/1.1 %s\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Access-Control-Allow-Origin: *\r\n"
                                   "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                                   "Access-Control-Allow-Headers: Content-Type\r\n"
                                   "Content-Length: %d\r\n"
                                   "\r\n"
                                   "%s",
                                   valid ? "200 OK" : "400 Bad Request",
                                   (int)strlen(txt), txt);
            }
            else
            {
                const char *txt = "Formato JSON inválido";
                hs->len = snprintf(hs->response, sizeof(hs->response),
                                   "HTTP/1.1 400 Bad Request\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Content-Length: %d\r\n"
                                   "\r\n"
                                   "%s",
                                   (int)strlen(txt), txt);
            }
        }
    }
    else if (strstr(req, "GET /api/weather"))
    {
        // Calcula alerta simples (>50%) para o frontend
        const bool rain_alert = (weather_data.rain_probability >= 0.5f);
        
        char json_data[512]; 
        
        // Monta o JSON com os novos campos (rainAlert e rainStatus)
        int json_len = snprintf(json_data, sizeof(json_data),
                 "{\"temperature\":%.2f,\"humidity\":%.2f,\"pressure\":%.2f,\"altitude\":%.2f,"
                 "\"rainProbability\":%.3f,\"rainAlert\":%s,\"rainStatus\":\"%s\","
                 "\"minTemperature\":%d,\"maxTemperature\":%d,\"tempOffset\":%.2f}",
                 weather_data.temperature, weather_data.humidity,
                 weather_data.pressure, weather_data.altitude, weather_data.rain_probability,
                 rain_alert ? "true" : "false",
                 rain_status, // Envia "warming_up", "ok", etc.
                 weather_data.minTemperature, weather_data.maxTemperature, weather_data.offsetTemperature);

        if (json_len < 0 || json_len >= (int)sizeof(json_data) - 1)
        {
            printf("ERRO: Buffer JSON truncado!\n");
            snprintf(json_data, sizeof(json_data), "{\"error\":\"Buffer truncado\"}");
        }

        int response_len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: application/json\r\n"
                           "Access-Control-Allow-Origin: *\r\n"
                           "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                           "Access-Control-Allow-Headers: Content-Type\r\n"
                           "Content-Length: %zu\r\n"
                           "\r\n"
                           "%s",
                           (int)strlen(json_data), json_data);

        // Tratamento de erro de buffer HTTP (mantido igual)
        if (response_len < 0 || response_len >= (int)sizeof(hs->response) - 1) {
            hs->len = snprintf(hs->response, sizeof(hs->response),
                       "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 21\r\n\r\nErro: Buffer truncado");
            hs->header_len = hs->len;
        } else {
            hs->len = response_len;
            hs->header_len = hs->len;
        }
        
        //printf("JSON enviado: %s\n", json_data);
    }
    else
    {
        // HTML principal
        size_t html_size = strlen(html_data);
        int header_len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html; charset=utf-8\r\n"
                           "Content-Length: %zu\r\n"
                           "Connection: close\r\n"
                           "Cache-Control: no-cache\r\n"
                           "\r\n"
                           "%s",
                           html_size, "");

        if (header_len < 0 || header_len >= (int)sizeof(hs->response) - 1)
        {
            printf("ERRO: Buffer HTTP truncado ao enviar HTML!\n");
            printf("   Tamanho HTML: %zu bytes\n", html_size);
            printf("   Tamanho buffer: %zu bytes\n", sizeof(hs->response));
            printf("   snprintf retornou: %d\n", header_len);
            
            hs->len = snprintf(hs->response, sizeof(hs->response),
                           "HTTP/1.1 413 Payload Too Large\r\n"
                           "Content-Type: text/plain\r\n"
                           "Content-Length: 47\r\n"
                           "\r\n"
                           "Erro: Página HTML muito grande para buffer");
            hs->header_len = hs->len;
        }
        else
        {
            hs->header_len = header_len;
            hs->stream_ptr = html_data;
            hs->stream_len = html_size;
            hs->stream_sent = 0;
            hs->len = hs->header_len + hs->stream_len;
            printf("HTML enviado em streaming (%zu bytes)\n", hs->stream_len);
        }
    }

    tcp_arg(tpcb, hs);
    tcp_sent(tpcb, http_sent);
    if (hs->stream_ptr)
    {
        err_t err = tcp_write(tpcb, hs->response, hs->header_len, TCP_WRITE_FLAG_COPY);
        if (err == ERR_OK)
        {
            size_t remaining = hs->stream_len - hs->stream_sent;
            size_t max_chunk = tcp_sndbuf(tpcb);
            if (max_chunk > 512)
                max_chunk = 512;
            if (max_chunk == 0)
            {
                tcp_output(tpcb);
                pbuf_free(p);
                return ERR_OK;
            }
            size_t chunk = remaining > max_chunk ? max_chunk : remaining;
            err = tcp_write(tpcb, hs->stream_ptr + hs->stream_sent, chunk, TCP_WRITE_FLAG_COPY);
            if (err == ERR_OK)
            {
                hs->stream_sent += chunk;
            }
            else if (err == ERR_MEM)
            {
                tcp_output(tpcb);
                pbuf_free(p);
                return ERR_OK;
            }
            else
            {
                printf("ERRO: tcp_write falhou ao iniciar envio de HTML (err=%d)\n", err);
            }
        }
        else
        {
            printf("ERRO: tcp_write falhou ao enviar cabecalho (err=%d)\n", err);
        }
        tcp_output(tpcb);
    }
    else
    {
        tcp_write(tpcb, hs->response, hs->len, TCP_WRITE_FLAG_COPY);
        tcp_output(tpcb);
    }
    pbuf_free(p);
    return ERR_OK;
}

// Função de callback para aceitar conexões TCP
static err_t connection_callback(void *arg, struct tcp_pcb *newpcb, err_t err)
{
    tcp_recv(newpcb, http_recv);
    return ERR_OK;
}

// Função para iniciar o servidor HTTP
static void start_http_server(void)
{
    struct tcp_pcb *pcb = tcp_new();
    if (!pcb)
    {
        printf("Erro ao criar PCB TCP\n");
        return;
    }
    if (tcp_bind(pcb, IP_ADDR_ANY, 80) != ERR_OK)
    {
        printf("Erro ao ligar o servidor na porta 80\n");
        return;
    }
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, connection_callback);
    printf("Servidor HTTP rodando na porta 80...\n");
}

// Função de interrupção para os botões
// Alterna estados: simulação (SW), alertas (A), reset limites (B)
void gpio_irq_handler(uint gpio, uint32_t events)
{
    int current_time = to_ms_since_boot(get_absolute_time());

    if (gpio == BTN_A_PIN && (current_time - last_button_a_press_time > BUTTON_DEBOUNCE_MS))
    {
        last_button_a_press_time = current_time;
        btn_a_event = true;
    }
    else if (gpio == BTN_B_PIN && (current_time - last_button_b_press_time > BUTTON_DEBOUNCE_MS))
    {
        last_button_b_press_time = current_time;
        btn_b_event = true;
    }
}

// Função para tentar conectar ao Wi-Fi
// Retorna true se a conexão for bem-sucedida, false caso contrário
bool try_wifi_connect()
{
    printf("Tentando conectar ao Wi-Fi '%s'...\n", WIFI_SSID);

    set_led_blue_pwm(); // LED azul para indicar tentativa de conexão

    // Tenta conectar com timeout de 10 segundos
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 10000))
    {
        printf("Falha ao conectar ao Wi-Fi\n");
        set_led_red_pwm(); // LED vermelho para falha
        return false;
    }

    // Conexão bem-sucedida
    uint8_t *ip = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
    printf("Wi-Fi conectado! IP: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
    set_led_green_pwm(); // LED verde para conexão bem-sucedida
    return true;
}

// Função para aguardar conexão Wi-Fi com múltiplas tentativas
// Retorna true se conectar com sucesso, false se exceder tentativas
// max_attempts = 0 significa tentativas ilimitadas
static bool wait_for_wifi_connection(uint32_t max_attempts)
{
    uint32_t attempts = 0;

    while (max_attempts == 0 || attempts < max_attempts)
    {
        attempts++;

        if (max_attempts > 0)
        {
            printf("Tentativa %u de %u...\n", attempts, max_attempts);
        }

        if (try_wifi_connect())
        {
            return true;
        }

        printf("Nova tentativa em %d segundos...\n", WIFI_RETRY_DELAY_MS / 1000);
        sleep_ms(WIFI_RETRY_DELAY_MS);
    }

    printf("Falha após %u tentativas de conexão\n", attempts);
    return false;
}