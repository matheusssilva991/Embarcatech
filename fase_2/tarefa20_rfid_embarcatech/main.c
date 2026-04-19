#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "lib/ssd1306/ssd1306.h"
#include "lib/ssd1306/display.h"
#include "lib/rfid/mfrc522.h"
#include "lib/led/led.h"
#include "lib/buzzer/buzzer.h"
#include "lib/button/button.h"

void update_display(int status);
void authorize_access();
void deny_access();
void gpio_irq_handler(uint gpio, uint32_t events);

ssd1306_t ssd;

void main()
{
    stdio_init_all();

    init_leds();

    init_display(&ssd);
    init_buzzer(BUZZER_A_PIN, 2.0f);
    init_btn(BTN_A_PIN);
    gpio_set_irq_enabled_with_callback(BTN_A_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_irq_handler);

    // Inicializa MFRC522
    MFRC522Ptr_t mfrc = MFRC522_Init();
    PCD_Init(mfrc, spi0);
    PCD_AntennaOn(mfrc); // Liga antena
    sleep_ms(500);


    char uid_str[24]; // Buffer para UID formatado
    bool color = true;

    while (1)
    {
        // Desliga LEDs antes de ler novo cartão
        gpio_put(RED_LED_PIN, 0);
        gpio_put(GREEN_LED_PIN, 0);

        printf("Waiting for card...\n");
        update_display(0); // Atualiza display para estado inicial

        while (!PICC_IsNewCardPresent(mfrc))
        {
            sleep_ms(500);
        }

        printf("Card detected! Trying to read UID...\n");

        if (PICC_ReadCardSerial(mfrc))
        {
            // Formata UID em string "XX XX XX XX ..."
            int offset = 0;
            for (int i = 0; i < mfrc->uid.size; i++)
            {
                offset += sprintf(&uid_str[offset], "%02X ", mfrc->uid.uidByte[i]);
            }
            printf("UID: %s\n", uid_str);

            // Verifica UID para acionar LEDs
            if (mfrc->uid.size == 4) // Certifica-se que tem 4 bytes no UID
            {
                // Cartão UID 00 FC 95 7C liga LED azul GPIO12
                if (mfrc->uid.uidByte[0] == 0x73 &&
                    mfrc->uid.uidByte[1] == 0x9A &&
                    mfrc->uid.uidByte[2] == 0x3B &&
                    mfrc->uid.uidByte[3] == 0x0E)
                {
                    authorize_access();
                }
                // Cartão UID C0 33 C3 80 liga LED verde GPIO13
                else if (mfrc->uid.uidByte[0] == 0x83 &&
                         mfrc->uid.uidByte[1] == 0xAD &&
                         mfrc->uid.uidByte[2] == 0x12 &&
                         mfrc->uid.uidByte[3] == 0x1A)
                {
                    authorize_access();
                }
                else
                {
                    deny_access();
                }
            }
        }
        else
        {
            printf("Failed to read UID\n");
            ssd1306_draw_string(&ssd, "Falha na", 8, 41);  // Desenha uma string
            ssd1306_draw_string(&ssd, " Leitura", 8, 52);   // Desenha uma string
            ssd1306_send_data(&ssd);
        }
        sleep_ms(2000);
    }
}


void update_display(int status)
{
    bool color = true;
    ssd1306_fill(&ssd, !color);                  // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 60, color, !color);      // Desenha um retângulo
    draw_centered_text(&ssd, "Access Control", 5);
    draw_centered_text(&ssd, "RFID RC522", 16);
    ssd1306_line(&ssd, 3, 25, 123, 25, color);           // Desenha uma linha

    if (status == 0)
    {
        draw_centered_text(&ssd, "Aproxime o", 32);
        draw_centered_text(&ssd, "CARTAO", 42);
    }
    else if (status == 1)
    {
        draw_centered_text(&ssd, "Acesso", 32);
        draw_centered_text(&ssd, "Autorizado.", 42);
    }
    else if (status == 2)
    {
        draw_centered_text(&ssd, "Acesso", 32);
        draw_centered_text(&ssd, "Negado.", 42);
    } else if (status == 3)
    {
        draw_centered_text(&ssd, "Lendo...", 37);
    }

    ssd1306_send_data(&ssd);
}

void authorize_access()
{
    gpio_put(GREEN_LED_PIN, 1);
    update_display(1); // Atualiza display para acesso autorizado
    play_tone_with_duration(BUZZER_A_PIN, 1000, 100); // Bip agudo curto
    sleep_ms(50);
    play_tone_with_duration(BUZZER_A_PIN, 800, 150);  // Bip médio
    sleep_ms(1500);
    gpio_put(GREEN_LED_PIN, 0);
}

void deny_access()
{
    gpio_put(RED_LED_PIN, 1);
    update_display(2); // Atualiza display para acesso negado
    for (int i = 0; i < 3; i++) {
        play_tone_with_duration(BUZZER_A_PIN, 400, 100); // Bip grave curto
        sleep_ms(50);
    }
    sleep_ms(1000);
    gpio_put(RED_LED_PIN, 0);
}

void gpio_irq_handler(uint gpio, uint32_t events)
{
    if (gpio == BTN_A_PIN)
    {
        reset_usb_boot(0, 0);
    }
}
