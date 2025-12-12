#include <stdint.h>
#include <stdbool.h>

#include <hardware/pio.h>
#include <hardware/gpio.h>

#include "../config.h"

#include "encoders.pio.h"
#include "encoders.h"

encoder_t g_encoders[ENC_COUNT];

void encoders_init()
{
    int prog = pio_add_program(ENC_PIO, &encoder_program);

    for (uint i = 0; i < ENC_COUNT; i++)
    {
        encoder_t *enc = &g_encoders[i];
        uint pin_ab = (i == 0) ? ENC0_PIN_AB : ENC1_PIN_AB;
        uint pin_press = (i == 0) ? ENC0_PIN_PRESS : ENC1_PIN_PRESS;

        // set up encoder pio for decoding gray code
        uint sm = pio_claim_unused_sm(ENC_PIO, true);
        encoder_program_init(ENC_PIO, sm, pin_ab, 0);

        enc->_sm = sm;
        enc->_last_value = 0;
        enc->delta = 0;

        // set up gpio
        gpio_init(pin_press);
        gpio_set_dir(pin_press, GPIO_IN);
        gpio_pull_up(pin_press);
        enc->_pin_press = pin_press;
    }
}

void encoders_tick()
{
    for (uint i = 0; i < ENC_COUNT; i++)
    {
        encoder_t *enc = &g_encoders[i];
        int32_t value = encoder_get_count(ENC_PIO, enc->_sm) / 2;
        enc->delta = (enc->_last_value - value);
        enc->_last_value = value;

        bool pressed = !gpio_get(enc->_pin_press);
        enc->edge = (pressed != enc->pressed);
        enc->pressed = pressed;
    }
}
