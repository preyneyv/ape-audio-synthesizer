#include <stdio.h>

#include <hardware/i2c.h>
#include <hardware/gpio.h>
#include <pico/time.h>

#include <utils/bitflags.h>

#include "../config.h"
#include "keys.h"

key_t g_keys[KEY_COUNT];

uint8_t last_reg;
uint8_t last_op_write;

// mapping from touch controller key indices to logical order
const uint8_t key_map[16] = {
    2, 4, 5, 7, 9, 11, KEY_OCT_DN, 0,   // y0
    3, 255, 6, 8, 10, 12, KEY_OCT_UP, 1 // y1
};

static int read(uint8_t reg, uint8_t *data, size_t len)
{
    last_reg = reg;
    last_op_write = 0;
    sleep_us(200);
    int ret = i2c_write_blocking_until(
        TOUCH_I2C_PORT, TOUCH_I2C_ADDR, &reg, 1, true,
        make_timeout_time_ms(TOUCH_I2C_TIMEOUT_MS));
    if (ret < 0)
        return ret;
    last_op_write = 1;
    sleep_us(200);
    ret = i2c_read_blocking_until(TOUCH_I2C_PORT, TOUCH_I2C_ADDR, data, len, false,
                                  make_timeout_time_ms(TOUCH_I2C_TIMEOUT_MS));
    return ret;
}

static int write(uint8_t reg, const uint8_t *data, size_t len)
{
    last_reg = reg;
    last_op_write = 0;
    sleep_us(200);
    int ret = i2c_write_blocking_until(
        TOUCH_I2C_PORT, TOUCH_I2C_ADDR, &reg, 1, false,
        make_timeout_time_ms(TOUCH_I2C_TIMEOUT_MS));
    if (ret < 0)
        return ret;
    last_op_write = 1;

    for (size_t i = 0; i < len; i++)
    {
        sleep_us(200);
        ret = i2c_write_blocking_until(TOUCH_I2C_PORT, TOUCH_I2C_ADDR, &data[i], 1, false,
                                       make_timeout_time_ms(TOUCH_I2C_TIMEOUT_MS));
        if (ret < 0)
            return ret;
    }
    return 0;
}

int keys_init_controller()
{
    uint8_t data;
    int ret;

    // send a reset (if the touch controller is already alive)
    data = 0x01;
    write(11, &data, 1);

    // Wait for device to be alive
    absolute_time_t start_timeout = make_timeout_time_ms(15000);
    while (true)
    {
        if (time_reached(start_timeout))
        {
            return PICO_ERROR_TIMEOUT;
        }
        ret = read(0, &data, 1);
        if (ret < 0)
        {
            sleep_ms(100);
            continue;
        }
        if (data == 0x11)
            break;
    }

    // GPIO direction
    data = 0b00011100;
    if ((ret = write(73, &data, 1)) < 0)
        return ret;

    // Set burst repetition
    data = 1;
    if ((ret = write(13, &data, 1)) < 0)
        return ret;

    // Set dht/awake to max
    data = 255;
    if ((ret = write(19, &data, 1)) < 0)
        return ret;

    // Set burst lengths
    uint8_t bursts[16];
    for (int i = 0; i < 16; i++)
        bursts[i] = 4;
    bursts[9] = 0; // Disable x1 y1
    if ((ret = write(54, bursts, 16)) < 0)
        return ret;

    // Set neg threshold
    uint8_t thresholds[16];
    for (int i = 0; i < 16; i++)
        thresholds[i] = 7;
    if ((ret = write(38, thresholds, 16)) < 0)
        return ret;

    // Calibrate
    data = 1;
    if ((ret = write(10, &data, 1)) < 0)
        return ret;

    return 0;
}

int keys_recalibrate()
{
    uint8_t data = 1;
    return write(10, &data, 1);
}

int keys_init()
{
    for (int i = 0; i < KEY_COUNT; i++)
    {
        key_t *key = &g_keys[i];
        key->idx = i;
        key->pressed = false;
        key->edge = false;
    }

    i2c_init(TOUCH_I2C_PORT, 50000);
    gpio_set_function(TOUCH_I2C_PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(TOUCH_I2C_PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(TOUCH_I2C_PIN_SDA);
    gpio_pull_up(TOUCH_I2C_PIN_SCL);
    gpio_set_slew_rate(TOUCH_I2C_PIN_SDA, GPIO_SLEW_RATE_FAST);
    gpio_set_slew_rate(TOUCH_I2C_PIN_SCL, GPIO_SLEW_RATE_FAST);
    gpio_set_drive_strength(TOUCH_I2C_PIN_SDA, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(TOUCH_I2C_PIN_SCL, GPIO_DRIVE_STRENGTH_12MA);

    return keys_init_controller();
}

void keys_tick()
{
    uint8_t status;
    for (int i = 0; i < KEY_COUNT; i++)
    {
        key_t *key = &g_keys[i];
        key->edge = false;
    }
    if (read(2, &status, 1) < 0)
        return;
    if (bf_has(status, 7))
    {
        // chip was reset, reinit
        keys_init_controller();
        return;
    }

    uint16_t keys = 0;
    if (read(3, (uint8_t *)&keys, 2) < 0)
        return;
    for (int i = 0; i < 16; i++)
    {
        uint8_t key_idx = key_map[i];
        if (key_idx == 255)
            continue;
        key_t *key = &g_keys[key_idx];

        bool pressed = (keys >> i) & 1;
        key->edge = (pressed != key->pressed);
        key->pressed = pressed;
    }
}
