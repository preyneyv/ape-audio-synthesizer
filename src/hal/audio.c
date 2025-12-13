#include <stdio.h>

#include <hardware/i2c.h>
#include <hardware/gpio.h>
#include <hardware/dma.h>
#include <hardware/clocks.h>
#include <pico/time.h>

#include "../config.h"

#include "i2s/i2s.h"
#include "audio.h"
#include <synth/synth.h>
#include <tracker/tracker.h>

static __attribute__((aligned(8))) pio_i2s i2s;

static void dma_i2s_in_handler(void)
{
    /* We're double buffering using chained TCBs. By checking which buffer the
     * DMA is currently reading from, we can identify which buffer it has just
     * finished reading (the completion of which has triggered this interrupt).
     */
    if (*(int32_t **)dma_hw->ch[i2s.dma_ch_in_ctrl].read_addr == i2s.input_buffer)
    {
        // It is inputting to the second buffer so we can overwrite the first
        tracker_process_audio(i2s.input_buffer, i2s.output_buffer);
    }
    else
    {
        // It is currently inputting the first buffer, so we write to the second
        tracker_process_audio(&i2s.input_buffer[STEREO_BUFFER_SIZE], &i2s.output_buffer[STEREO_BUFFER_SIZE]);
    }
    dma_hw->ints0 = 1u << i2s.dma_ch_in_data; // clear the IRQ
}

static int read(uint8_t reg, uint8_t *data, size_t len)
{
    sleep_us(200);
    int ret = i2c_write_blocking_until(
        AUDIO_I2C_PORT, AUDIO_I2C_ADDR, &reg, 1, true,
        make_timeout_time_ms(AUDIO_I2C_TIMEOUT_MS));
    if (ret < 0)
        return ret;
    sleep_us(200);
    ret = i2c_read_blocking_until(AUDIO_I2C_PORT, AUDIO_I2C_ADDR, data, len, false,
                                  make_timeout_time_ms(AUDIO_I2C_TIMEOUT_MS));

    return ret;
}

static int write(uint8_t reg, const uint8_t byte)
{
    sleep_us(200);
    int ret = i2c_write_blocking_until(
        AUDIO_I2C_PORT, AUDIO_I2C_ADDR, &reg, 1, true,
        make_timeout_time_ms(AUDIO_I2C_TIMEOUT_MS));
    AUDIO_I2C_PORT->restart_on_next = false;
    printf("write reg %02x ret %d\n", reg, ret);
    if (ret < 0)
        return ret;
    // sleep_us(200);
    ret = i2c_write_blocking_until(AUDIO_I2C_PORT, AUDIO_I2C_ADDR, &byte, 1, false,
                                   make_timeout_time_ms(AUDIO_I2C_TIMEOUT_MS));
    printf("write byte %02x ret %d\n", byte, ret);
    return ret;
}

void audio_i2c_setup()
{

    // configure audio codec via i2c
    write(0x17, 0b00000000); // shutdown

    // set DAC in slave mode
    write(0x05, 0b00100000); // set clockdiv range 10-20MHz
    write(0x06, 0b10000000); // set PLL on
    write(0x07, 0b00000000); // infer clock
    write(0x08, 0b00010000); // slave mode, I2S compat

    // write(0x0b, 0b11000001);
    write(0x0b, 0b00000000);
    // dac level
    write(0x0c, 0b00000110);
    // adc level
    write(0x0d, 0b00110011);

    // line input level
    write(0x0e, 0b00001100);
    write(0x0f, 0b00001100);

    // playback volume
    write(0x10, 0b00001001); // -6db
    write(0x11, 0b00001001); // -6db

    // left mic preamp
    write(0x12, 0b01111111); // +30db

    // adc routing
    write(0x14, 0b01100000); // l = mic, r = line

    // output mode
    write(0x16, 0b00000010); // hpmode stereo capless

    // wake up with all peripherals
    write(0x17, 0b11001111);
}

void audio_init()
{
    i2c_init(AUDIO_I2C_PORT, AUDIO_I2C_SPEED);
    gpio_set_function(AUDIO_I2C_PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(AUDIO_I2C_PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(AUDIO_I2C_PIN_SDA);
    gpio_pull_up(AUDIO_I2C_PIN_SCL);

    // configure dac
    audio_i2c_setup();

    // enable mclk
    clock_gpio_init_int_frac8(
        AUDIO_PIN_MCLK,
        CLOCKS_CLK_GPOUT1_CTRL_AUXSRC_VALUE_CLK_SYS,
        10, 95); // 132 / 12.288 = 10 + 95/128

    // start i2s
    i2s_config cfg = {
        .fs = AUDIO_SAMPLE_RATE,
        .sck_mult = 256,
        .bit_depth = 16,
        .sck_pin = 0,
        .dout_pin = AUDIO_PIN_SDIN,
        .din_pin = AUDIO_PIN_SDOUT,
        .clock_pin_base = AUDIO_PIN_LRCLK,
        .sck_enable = false,
    };

    printf("starting i2s\n");
    i2s_program_start_synched(AUDIO_PIO, &cfg, dma_i2s_in_handler, &i2s);
    printf("i2s started\n");
}
