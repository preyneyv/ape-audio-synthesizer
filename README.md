## Touch Sensor

```py
import time

import board
import busio
from adafruit_bus_device.i2c_device import I2CDevice

i = busio.I2C(board.GP15, board.GP14)
d = I2CDevice(i, 0x0D)


def read(addr):
    buf = bytearray(1)
    with d:
        d.write(bytes([addr]))

    with d:
        d.readinto(buf)
    return buf[0]


def write(addr, *vals):
    with d:
        d.write(bytes([addr] + list(vals)))


def init():
    # wait for device to be alive
    while read(0) != 0x11:
        pass

    # lp_mode
    # write(12, 1)

    # gpio direction
    write(73, 0b00011100)

    # set burst repetition
    write(13, 2)

    # disable drift comp (probably bad!!)
    # write(15, 0, 0)

    # disable negative recal delay
    write(18, 0)

    # set dht/awake to max
    write(19, 255)

    # set burst lengths
    bursts = [8] * 16
    bursts[9] = 0  # disable x1 y1
    write(54, *bursts)

    # set neg threshold
    write(38, *([70] * 16))

    # calibrate
    write(10, 1)


def read_inputs():
    k1 = read(3)
    k2 = read(4)
    print("--")
    print(f"{k1:08b}\n{k2:08b}")


def run():
    init()
    while True:
        read_inputs()
        time.sleep(0.2)
```

## Audio

```py
import array
import math
import time

import adafruit_pioasm
import audiobusio
import audiocore
import board
import busio
import countio
import digitalio
import rp2pio
import supervisor
from adafruit_bus_device.i2c_device import I2CDevice

# A tiny PIO program that drives a pin high/low forever
program_source = """
.program clock_out
    set pins, 1
    set pins, 0
"""

# Assemble to machine code
program = adafruit_pioasm.assemble(program_source)

# Create & start the StateMachine
sm = rp2pio.StateMachine(
    program,
    frequency=24_000_000,  # PIO clock frequency, NOT pin frequency directly
    first_set_pin=board.GP23,
    set_pin_count=1,
    initial_set_pin_direction=1,
)

i = busio.I2C(board.GP25, board.GP24, frequency=100_000)
d = I2CDevice(i, 0x18)

adc_sdout = digitalio.DigitalInOut(board.GP26)
adc_sdout.direction = digitalio.Direction.INPUT
# adc_sdin = digitalio.DigitalInOut(board.GP27)
# adc_sdin.direction = digitalio.Direction.OUTPUT
# adc_sdin.value = False

# adc_lrclk = digitalio.DigitalInOut(board.GP28)
# adc_lrclk.direction = digitalio.Direction.INPUT
# adc_bclk = digitalio.DigitalInOut(board.A3)
# adc_bclk.direction = digitalio.Direction.INPUT


def read(addr):
    buf = bytearray(1)
    with d:
        d.write_then_readinto(bytes([addr]), buf)
    return buf[0]


def write(addr, *vals):
    with d:
        d.write(bytes([addr] + list(vals)))


def init():
    write(0x17, 0b00000000)

    # for testing, using master mode
    # set system clock range to 10-20 MHz
    # write(0x05, 0b00_01_0000)
    # write(0x06, 0b0_0100000)
    # write(0x07, 0b11000101)
    # write(0x08, 0b1000_0000)
    write(0x05, 0b00_10_0000)
    write(0x06, 0b1_0000000)
    write(0x07, 0)
    write(0x08, 0b0001_0000)

    # enable filters
    # write(0x0A, 0)
    write(0x0A, 0b0_101_0_101)

    # enable sidetone
    # write(0x0B, 0b0)
    write(0x0B, 0b11_0_00001)

    # set dac level
    write(0x0C, 0b0_0_00_0110)

    # set adc level
    write(0x0D, 0b1111_1111)

    # disable line input to headphone amp
    write(0x0E, 0b0100_1100)
    write(0x0F, 0b0100_1100)

    # set playback volume
    write(0x10, 0b0000_1001)  # -6db
    write(0x11, 0b0000_1001)  # -6db

    # enable left mic preamp
    write(0x12, 0b0_11_11111)  # +30db

    # setup adc input
    write(0x14, 0b11_10_0000)  # l = mic, r = none, jacksns

    # setup mode
    write(0x16, 0b0000_0_010)  # enable jsns, hpmode stereo capless

    # wake from shutdown, enable both line inputs, dacs, adcs.
    write(0x17, 0b1_11_0_11_11)
    # write(0x17, 0b1_11_0_00_00)


init()

sample_rate = 16000
frequency = 400  # 1kHz test tone
# One period of sine at this sample rate
length = sample_rate // frequency

# Build 16-bit signed sine table
sine_samples = array.array(
    "h",
    [
        int(0 * 32767 * math.sin(2 * math.pi * frequency * i / sample_rate))
        for i in range(length)
    ],
)

sine_wave = audiocore.RawSample(sine_samples, sample_rate=sample_rate)

# I2S pins:
# BCLK: GP29
# LRCLK: GP28
# SDOUT from RP2040 -> SDIN on MAX9867: GP27
i2s = audiobusio.I2SOut(bit_clock=board.A3, word_select=board.GP28, data=board.GP27)
print("playing sine")
i2s.play(sine_wave, loop=True)
while True:
    time.sleep(1)

i2s.stop()
# print(read(0))
```

high hiss when dac enabled, unsure if that's the fault of my earphones or smth else.
