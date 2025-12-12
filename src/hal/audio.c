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

static __attribute__((aligned(8))) pio_i2s i2s;

static void decimate_48_to_16khz(const int32_t *src, int32_t *dst, size_t num_dst_frames)
{
    // simple decimation by averaging every 3 samples
    for (size_t i = 0; i < num_dst_frames; i++)
    {
        dst[i] = (src[i * 3] + src[i * 3 + 1] + src[i * 3 + 2]) / 3;
    }
}

static void inflate_16_to_48khz(const int32_t *src, int32_t *dst, size_t num_src_frames)
{
    // simple inflation by repeating each sample 3 times
    for (size_t i = 0; i < num_src_frames; i++)
    {
        dst[i * 3] = src[i];
        dst[i * 3 + 1] = src[i];
        dst[i * 3 + 2] = src[i];
    }
}

static void process_audio(const int32_t *input, int32_t *output, size_t num_frames)
{
    static int32_t mono_buffer[AUDIO_BUFFER_FRAMES];
    static int32_t decimated_buffer[AUDIO_BUFFER_FRAMES / 3];

    audio_synth_fill_buffer(&g_synth, mono_buffer, num_frames);
    decimate_48_to_16khz(mono_buffer, decimated_buffer, num_frames / 3);
    inflate_16_to_48khz(decimated_buffer, mono_buffer, num_frames / 3);

    for (size_t i = 0; i < num_frames; i++)
    {
        // stereo output from mono buffer
        output[i * 2] = mono_buffer[i] << 16;
        output[i * 2 + 1] = output[i * 2];
    }

    // int32_t decimated_l;
    // int32_t decimated_r;
    // // Just copy the input to the output
    // for (size_t i = 0; i < num_frames * 2; i += 6)
    // {
    //     // output[i] = input[i];
    //     // // output[i] = 0;
    //     decimated_l = (input[i] + input[i + 2] + input[i + 4]) / 3;
    //     decimated_r = (input[i + 1] + input[i + 3] + input[i + 5]) / 3;

    //     output[i] = output[i + 2] = output[i + 4] = decimated_l;
    //     output[i + 1] = output[i + 3] = output[i + 5] = decimated_r;
    // }
}

static void dma_i2s_in_handler(void)
{
    /* We're double buffering using chained TCBs. By checking which buffer the
     * DMA is currently reading from, we can identify which buffer it has just
     * finished reading (the completion of which has triggered this interrupt).
     */
    if (*(int32_t **)dma_hw->ch[i2s.dma_ch_in_ctrl].read_addr == i2s.input_buffer)
    {
        // It is inputting to the second buffer so we can overwrite the first
        process_audio(i2s.input_buffer, i2s.output_buffer, AUDIO_BUFFER_FRAMES);
    }
    else
    {
        // It is currently inputting the first buffer, so we write to the second
        process_audio(&i2s.input_buffer[STEREO_BUFFER_SIZE], &i2s.output_buffer[STEREO_BUFFER_SIZE], AUDIO_BUFFER_FRAMES);
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

    // write(0x05, 0b00010000);
    // write(0x06, 0b00100000);
    // write(0x07, 0b11000101);
    // write(0x08, 0b10000000);

    // write(0x0b, 0b11000001);
    write(0x0b, 0b00000000);
    // dac level
    write(0x0c, 0b00000110);
    // adc level
    write(0x0d, 0b00110011);

    // line input level
    write(0x0e, 0b01001100);
    write(0x0f, 0b01001100);

    // playback volume
    write(0x10, 0b00001001); // -6db
    write(0x11, 0b00001001); // -6db

    // left mic preamp
    write(0x12, 0b01111111); // +30db

    // adc routing
    write(0x14, 0b10100000); // l = mic + line, r = line

    // output mode
    write(0x16, 0b00000010); // hpmode stereo capless

    // wake up with all peripherals
    write(0x17, 0b11101111);
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
