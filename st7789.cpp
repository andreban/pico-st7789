//
// Created by andre on 07/03/2021.
//

#include "pico/stdlib.h"
#include "st7789.h"

St7789::St7789(
        uint8_t width,
        uint8_t height,
        uint16_t *frame_buffer,
        spi_inst *spi,
        int8_t pin_dc,
        int8_t pin_reset,
        int8_t pin_sck,
        int8_t pin_mosi,
        int8_t pin_cs,
        int8_t pin_miso) {
    this->spi = spi;
    this->width = width;
    this->height = height;
    this->pin_reset = pin_reset;
    this->pin_dc = pin_dc;
    this->frame_buffer = frame_buffer;
    this->pin_sck = pin_sck;
    this->pin_mosi = pin_mosi;
    this->pin_cs = pin_cs;
    this->pin_miso = pin_miso;
}

void St7789::sleep_mode(bool value) {
    if (value) {
        send_command(ST7789_SLPIN);
    } else {
        send_command(ST7789_SLPOUT);
    }
}

void St7789::set_invert_mode(bool invert) {
    if (invert) {
        send_command(ST7789_INVON);
    } else {
        send_command(ST7789_INVOFF);
    }
}

void St7789::set_color_mode(uint8_t mode) {
    send_command(ST7789_COLMOD, mode);
}

void St7789::hard_reset() {
    gpio_put(pin_reset, 1);
    sleep_ms(50);
    gpio_put(pin_reset, 0);
    sleep_ms(50);
    gpio_put(pin_reset, 1);
    sleep_ms(150);
}

void St7789::soft_reset() {
    send_command(ST7789_SWRESET);
    sleep_ms(150);
}

void St7789::update() {
    send_command(ST7789_RAMWR, width * height * sizeof(uint16_t), (uint8_t *) frame_buffer);
}

void St7789::init_pins() {
    gpio_set_function(pin_dc, GPIO_FUNC_SIO); // Same as "gpio_init(PIN_DC);
    gpio_set_dir(pin_dc, GPIO_OUT);

    if (pin_reset != -1) {
        gpio_set_function(pin_reset, GPIO_FUNC_SIO); // gpio_init(PIN_RESET);
        gpio_set_dir(pin_reset, GPIO_OUT);
    }

    if (pin_cs != -1) {
        gpio_set_function(pin_cs, GPIO_FUNC_SIO);
        gpio_set_dir(pin_cs, GPIO_OUT);
    }

    spi_init(spi, spi_baudrate);
    gpio_set_function(pin_sck, GPIO_FUNC_SPI);
    gpio_set_function(pin_mosi, GPIO_FUNC_SPI);

    if(pin_miso != -1) {
        gpio_set_function(pin_miso, GPIO_FUNC_SPI);
    }

    // Pimoroni's pico use CS, with the default config. The cheap screen doesn't have CS and needs to require setting
    // polarity to 1.
    if (pin_cs == -1) {
        spi_set_format(spi, 8, SPI_CPOL_1, SPI_CPHA_0, SPI_MSB_FIRST);
    }
}

void St7789::init() {
    init_pins();

    if (pin_reset != -1) {
        hard_reset();
    }
    soft_reset();
    sleep_mode(false);
    sleep_ms(50);

    if(width == 240 && height == 240) {
        send_command(ST7789_MADCTL,0x04);  // row/column addressing order - rgb pixel order
        send_command(ST7789_TEON, 0x00);  // enable frame sync signal if used
    }

    if(width == 240 && height == 135) {
        send_command(ST7789_MADCTL,0x70);
    }

    set_color_mode(COLOR_MODE_16BIT);

    set_invert_mode(true);
    sleep_ms(10);

    send_command(ST7789_NORON);
    sleep_ms(10);

    send_command(ST7789_DISPON);
    // setup correct addressing window
    if(width == 240 && height == 240) {
        send_command(ST7789_CASET, 4, new uint8_t[4]{0x00, 0x00, 0x00, 0xef});  // 0 .. 239 columns
        send_command(ST7789_RASET, 4, new uint8_t[4]{0x00, 0x00, 0x00, 0xef});  // 0 .. 239 rows
    }

    if(width == 240 && height == 135) {
        send_command(ST7789_RASET, 4, new uint8_t[4]{0x00, 0x35, 0x00, 0xbb}); // 53 .. 187 (135 rows)
        send_command(ST7789_CASET, 4, new uint8_t[4]{0x00, 0x28, 0x01, 0x17}); // 40 .. 279 (240 columns)
    }
}

void St7789::send_command(uint8_t command, uint len, uint8_t *data) {
    if (pin_cs != -1) {
        gpio_put(pin_cs, 0);
    }

    gpio_put(pin_dc, 0);
    spi_write_blocking(spi, &command, 1);

    if (data) {
        gpio_put(pin_dc, 1);
        spi_write_blocking(spi, data, len);
    }

    if (pin_cs != -1) {
        gpio_put(pin_cs, 1);
    }
}

void St7789::send_command(uint8_t command, uint8_t data) {
    send_command(command, 1, &data);
}

void St7789::send_command(uint8_t command) {
    send_command(command, 0, nullptr);
}

