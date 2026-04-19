#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/irq.h"
#include "math.h"

// #include "lib/ws2812b/ws2812b.h"
#include "lib/button/button.h"
#include "pico/multicore.h"
#include "lib/ssd1306/display.h"
#include "lib/ssd1306/ssd1306.h"
#include "lib/aht20/aht20.h"
#include "lib/bmp280/bmp280.h"

#define I2C0_PORT i2c0              // i2c0 pinos 0 e 1
#define I2C0_SDA 0                  // 0
#define I2C0_SCL 1                  // 1
#define SEA_LEVEL_PRESSURE 101325.0 // 101325.0 // Pressão ao nível do mar em Pa

void gpio_irq_callback(uint gpio, uint32_t events);
void core1_entry();
void core1_interrupt_handler();
double calculate_altitude(double pressure);

typedef struct weather_data
{
    float temperature;
    float humidity;
    float pressure;
    float altitude;
} weather_data_t;

ssd1306_t ssd;
static weather_data_t weather_data = {0, 0, 0, 0}; // Dados do tempo

int main()
{
    stdio_init_all();
    init_display(&ssd);
    // ws2812b_init();
    init_btns();

    // Inicializa o I2C para o SMP280
    i2c_init(I2C0_PORT, 400 * 1000);
    gpio_set_function(I2C0_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA);
    gpio_pull_up(I2C0_SCL);

    // Inicializa o I2C para o AHT20
    i2c_init(I2C0_PORT, 400 * 1000);
    gpio_set_function(I2C0_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA);
    gpio_pull_up(I2C0_SCL);

    // Inicializa o BMP280
    bmp280_init(I2C0_PORT);
    struct bmp280_calib_param params;
    bmp280_get_calib_params(I2C0_PORT, &params);

    // Inicializa o AHT20
    aht20_reset(I2C0_PORT);
    aht20_init(I2C0_PORT);

    gpio_set_irq_enabled_with_callback(
        BTN_A_PIN, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_callback);

    multicore_launch_core1(core1_entry);

    AHT20_Data data;
    int32_t raw_temp_bmp;
    int32_t raw_pressure;

    while (true)
    {
        // Leitura do BMP280
        bmp280_read_raw(I2C0_PORT, &raw_temp_bmp, &raw_pressure);

        weather_data.temperature = bmp280_convert_temp(raw_temp_bmp, &params);
        weather_data.temperature /= 100.0; // Converte para Celsius

        weather_data.pressure = bmp280_convert_pressure(raw_pressure, raw_temp_bmp, &params);
        weather_data.pressure /= 100.0;                                            // Converte para hPa
        weather_data.altitude = calculate_altitude(weather_data.pressure * 100.0); // Converte hPa para Pa

        if (aht20_read(I2C0_PORT, &data))
            weather_data.humidity = data.humidity;
        else
        {
            printf("Erro na leitura do AHT20!\n");
            weather_data.humidity = 0.0; // Valor padrão em caso de erro
        }

        // Envia os dados para o Core 1
        multicore_fifo_push_blocking(*(uint32_t *)&weather_data.temperature);
        multicore_fifo_push_blocking(*(uint32_t *)&weather_data.humidity);
        multicore_fifo_push_blocking(*(uint32_t *)&weather_data.pressure);
        multicore_fifo_push_blocking(*(uint32_t *)&weather_data.altitude);

        sleep_ms(2000); // Aguarda 2 segundos entre as leituras
    }
}

void gpio_irq_callback(uint gpio, uint32_t events)
{
    if (gpio == BTN_A_PIN && (events & GPIO_IRQ_EDGE_FALL))
    {
        reset_usb_boot(0, 0); // Reset the Pico to bootloader mode
    }
}

void core1_interrupt_handler()
{
    bool color = true;

    while (multicore_fifo_rvalid())
    {
        weather_data_t received_weather_data;
        union { uint32_t u; float f; } conv;

        conv.u = multicore_fifo_pop_blocking();
        received_weather_data.temperature = conv.f;

        conv.u = multicore_fifo_pop_blocking();
        received_weather_data.humidity = conv.f;

        conv.u = multicore_fifo_pop_blocking();
        received_weather_data.pressure = conv.f;

        conv.u = multicore_fifo_pop_blocking();
        received_weather_data.altitude = conv.f;

        printf("Core 1: Temp=%.2f°C, Hum=%.2f%%, Press=%.2f hPa, Alt=%.2f m\n",
               received_weather_data.temperature,
               received_weather_data.humidity,
               received_weather_data.pressure,
               received_weather_data.altitude);

        ssd1306_fill(&ssd, !color);                             // Limpa o display
        ssd1306_rect(&ssd, 3, 3, 122, 60, color, !color);       // Retângulo
        draw_centered_text(&ssd, "Weather Data", 6);       // Título
        ssd1306_line(&ssd, 3, 15, 124, 15, color);              // Linha

        char buf[32];
        snprintf(buf, sizeof(buf), "Temp: %.1f C", received_weather_data.temperature);
        ssd1306_draw_string(&ssd, buf, 8, 20);

        snprintf(buf, sizeof(buf), "Hum: %.1f %%", received_weather_data.humidity);
        ssd1306_draw_string(&ssd, buf, 8, 30);

        snprintf(buf, sizeof(buf), "Pre:%.1f hPa", received_weather_data.pressure);
        ssd1306_draw_string(&ssd, buf, 8, 40);

        snprintf(buf, sizeof(buf), "Alt: %.1f m", received_weather_data.altitude);
        ssd1306_draw_string(&ssd, buf, 8, 50);

        ssd1306_send_data(&ssd);
    }

    multicore_fifo_clear_irq();
}

void core1_entry()
{
    multicore_fifo_clear_irq();
    irq_set_exclusive_handler(SIO_IRQ_PROC1, core1_interrupt_handler);
    irq_set_enabled(SIO_IRQ_PROC1, true);
    while (true)
    {
        tight_loop_contents();
    }
}

// Função para calcular a altitude a partir da pressão atmosférica
double calculate_altitude(double pressure)
{
    return 44330.0 * (1.0 - pow(pressure / SEA_LEVEL_PRESSURE, 0.1903));
}
