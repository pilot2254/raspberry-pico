#include <stdio.h>
#include "pico/stdlib.h"

int main()
{
    stdio_init_all();

    //gpio25 on regular pico
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    int count = 0;
    while (true) {
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(500);
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        sleep_ms(500);

        printf("hello world! blink #%d\n", count++);
    }
}
