#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "../pimoroni-pico/libraries/pico_graphics/pico_graphics.hpp"
#include "st7789.h"

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

#ifdef PIMORONI_DISPLAY
const uint WIDTH = 240;
const uint HEIGHT = 135;
const int8_t PIN_SPI1_SCK_SCL = 18;
const int8_t PIN_SPI1_MOSI_SDA = 19;
const int8_t PIN_SPI1_MISO_SDA = -1;
const int8_t PIN_RESET = -1;
const int8_t PIN_DC = 16;
const int8_t PIN_CS = 17;
spi_inst *SPI = spi0;
#else
const uint WIDTH = 240;
const uint HEIGHT = 240;
const int8_t PIN_SPI1_SCK_SCL = 10;
const int8_t PIN_SPI1_MOSI_SDA = 11;
const int8_t PIN_SPI1_MISO_SDA = -1;
const int8_t PIN_RESET = 12;
const int8_t PIN_DC = 13;
const int8_t PIN_CS = -1;
spi_inst *SPI = spi1;
#endif

const uint8_t PIN_ONBOARD_LED = PICO_DEFAULT_LED_PIN;
uint16_t *buffer = new uint16_t[WIDTH * HEIGHT];

int posAt(int x, int y) {
    return y * WIDTH + x;
}

static unsigned int g_seed;

// Used to seed the generator.
inline void fast_srand(int seed) {
    g_seed = seed;
}

// Compute a pseudorandom integer.
// Output value in range [0, 32767]
inline int fast_rand(void) {
    g_seed = (214013*g_seed+2531011);
    return (g_seed>>16)&0x7FFF;
}

int main() {
    using namespace pimoroni;
    gpio_set_function(PIN_ONBOARD_LED, GPIO_FUNC_SIO); //    gpio_init(PIN_ONBOARD_LED);
    gpio_set_dir(PIN_ONBOARD_LED, GPIO_OUT);
    gpio_put(PIN_ONBOARD_LED, 0);

    St7789 display = St7789(
            WIDTH,
            HEIGHT,
            buffer,
            SPI,
            PIN_DC,
            PIN_RESET,
            PIN_SPI1_SCK_SCL,
            PIN_SPI1_MOSI_SDA,
            PIN_CS,
            PIN_SPI1_MISO_SDA);
    display.init();

    PicoGraphics graphics = PicoGraphics(WIDTH, HEIGHT, buffer);
    graphics.set_pen(0, 0, 0);
    graphics.clear();

    uint16_t pallete[36] = { // 36 colours
            graphics.create_pen(0x07, 0x07, 0x07), graphics.create_pen(0x1f, 0x07, 0x07),
            graphics.create_pen(0x2f, 0x0f, 0x07), graphics.create_pen(0x47, 0x0f, 0x07),
            graphics.create_pen(0x57, 0x17, 0x07), graphics.create_pen(0x67, 0x1f, 0x07),
            graphics.create_pen(0x77, 0x1f, 0x07), graphics.create_pen(0x8f, 0x27, 0x07),
            graphics.create_pen(0x9f, 0x2f, 0x07), graphics.create_pen(0xaf, 0x3f, 0x07),
            graphics.create_pen(0xbf, 0x47, 0x07), graphics.create_pen(0xc7, 0x47, 0x07),
            graphics.create_pen(0xDF, 0x4F, 0x07), graphics.create_pen(0xDF, 0x57, 0x07),
            graphics.create_pen(0xDF, 0x57, 0x07), graphics.create_pen(0xD7, 0x5F, 0x07),
            graphics.create_pen(0xD7, 0x67, 0x0F), graphics.create_pen(0xcf, 0x6f, 0x0f),
            graphics.create_pen(0xcf, 0x77, 0x0f), graphics.create_pen(0xcf, 0x7f, 0x0f),
            graphics.create_pen(0xCF, 0x87, 0x17), graphics.create_pen(0xC7, 0x87, 0x17),
            graphics.create_pen(0xC7, 0x8F, 0x17), graphics.create_pen(0xC7, 0x97, 0x1F),
            graphics.create_pen(0xBF, 0x9F, 0x1F), graphics.create_pen(0xBF, 0x9F, 0x1F),
            graphics.create_pen(0xBF, 0xA7, 0x27), graphics.create_pen(0xBF, 0xA7, 0x27),
            graphics.create_pen(0xBF, 0xAF, 0x2F), graphics.create_pen(0xB7, 0xAF, 0x2F),
            graphics.create_pen(0xB7, 0xB7, 0x2F), graphics.create_pen(0xB7, 0xB7, 0x37),
            graphics.create_pen(0xCF, 0xCF, 0x6F), graphics.create_pen(0xDF, 0xDF, 0x9F),
            graphics.create_pen(0xEF, 0xEF, 0xC7), graphics.create_pen(0xFF, 0xFF, 0xFF)
    };

    uint8_t fire[WIDTH * HEIGHT];

    // Initialises the screen
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            uint32_t pos = posAt(x, y);
            fire[pos] = 0;
        }
    }

    // Initialises the bottom line
    for (int i = 0; i < WIDTH; i++) {
        uint32_t pos = posAt(i, HEIGHT - 1);
        fire[pos] = 35;
    }

    display.update();
    gpio_put(PIN_ONBOARD_LED, 1);
    while(true) {
        for (int y = 0; y < HEIGHT; y++) {
            // We precompute the rows for a small performance gain.
            int row = y * WIDTH;

            // For each pixel in each row, we calculate the colours that will be
            // rendered on the previous row, on the next call to update.
            int next_row = y == 0 ? 0 : (y - 1) * WIDTH;

            for (int x = 0; x < WIDTH; x++) {
                uint8_t color = fire[row + x];
                uint16_t pen = pallete[color];
                *graphics.ptr(x, y) = pen;

                if (y > 0) {
                    int new_x = x;
                    int rand = fast_rand() % 3;
                    new_x = (new_x + rand - 1);
                    if (new_x >= WIDTH) {
                        new_x = new_x - WIDTH;
                    } else if (new_x < 0) {
                        new_x = new_x + WIDTH;
                    }
                    color = color > 0 ? color - (rand & 1) : 0;
                    fire[next_row + new_x] = color;
                }
            }
        }
        display.update();
    }
    return 0;
}

#pragma clang diagnostic pop