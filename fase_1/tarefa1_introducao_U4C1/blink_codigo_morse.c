#include <stdio.h>
#include "pico/stdlib.h"

const uint8_t LED_PIN = 13;
const uint8_t GAP = 125;
const uint8_t LETTER_GAP = 250;
const uint32_t WORD_GAP = 3000;

void blink_morse(char *word);
void dot();
void dash();

int main()
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    stdio_init_all();

    while (true) {
        blink_morse("SOS");
        printf("SOS!\n");
    }
}

void blink_morse(char *word) {
    for (int i = 0; word[i] != '\0'; i++) {
        switch (word[i]) {
            case 'S':
            case 's':
                for (int j = 0; j < 3; j++) {
                    dot();
                    sleep_ms(GAP);
                }
                sleep_ms(LETTER_GAP);
                break;
            case 'O':
            case 'o':
                for (int j = 0; j < 3; j++){
                    dash();
                    sleep_ms(GAP);
                }
                sleep_ms(LETTER_GAP);
                break;
            default:
                printf("Invalid character: %c\n", word[i]);
        }
    }
    sleep_ms(WORD_GAP);
}

void dot() {
    gpio_put(LED_PIN, 1);
    sleep_ms(200);
    gpio_put(LED_PIN, 0);
}

void dash() {
    gpio_put(LED_PIN, 1);
    sleep_ms(800);
    gpio_put(LED_PIN, 0);
}
