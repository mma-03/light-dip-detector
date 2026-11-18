# Light Dip Detector
Assignment 2 for ENSC 351 (Embedded and Real-Time System Software) at SFU, Fall 2026. A multithreaded C program that runs on a BeagleY-AI board.

The program reads a light sensor through an SPI ADC about 880 times a second, and once a second counts how many times the light "dipped" below its normal level. An LED points at the sensor, and its flash rate is set with a rotary encoder. That makes testing simple: with the LED flashing at 10 Hz, the program should count about 10 dips per second. A PC can also query the program over UDP.

## Hardware

| **Part** | **Connection** |
|---|---|
| MCP3208 12-bit ADC | SPI0 (`/dev/spidev0.0`) |
| Photoresistor + 10 kΩ resistor (voltage divider) | ADC channel 2, more light gives a higher voltage |
| LED + 470 Ω resistor | GPIO12 (header pin 32) |
| Rotary encoder | GPIO16 and GPIO17 (pins 36 and 11), using the board's internal pull-ups |

## How it's organized

| **Folder** | **Modules** |
|---|---|
| `hal/` (talks to hardware) | adc (MCP3208 driver), led, encoder |
| `app/` (never touches hardware) | sampler, dip, flasher, knob, udpServer, main |

Sampling, LED flashing, reading the encoder and the UDP server each run in their own pthread. Data shared between threads is protected with mutexes.

Some design choices:
- **Two buffers:** The sampler fills one buffer while the rest of the program works with a copy of the previous second's samples. Once a second they are swapped inside one locked section, so no samples go missing in between.
- **Dips with hysteresis:** The normal light level is a running average (`avg = 0.999 * avg + 0.001 * sample`). A dip starts when the light drops 0.1 V below the average, and the next one can't start until the light comes back to within 0.07 V. Without that gap, noise around the threshold got counted as extra dips.
- **Software timed LED:** The LED is flashed by a thread that toggles the GPIO, not by hardware PWM. The board's PWM can't go slower than about 2.13 Hz, and the assignment needed a minimum of 1 Hz.
- **Encoder debounce:** The rotary encoder's contacts bounce, so a click only counts once the knob is back at rest, and only if it moved at least 2 steps in one direction.

## Building and running
Cross-compiled on a Debian host with `aarch64-linux-gnu-gcc` and CMake. Needs libgpiod 2.x: libgpiod-dev:arm64 on the host and gpiod on the board.

```sh
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=toolchain-arm.cmake
cmake --build build
```

On the board:

```sh
sudo ./light_sampler
```

From the host, commands are sent with `nc -u 192.168.7.2 12345`:

| **Command** | **Reply** |
|---|---|
| `help` or `?` | List of commands |
| `count` | Total samples taken since start-up |
| `length` | Samples taken in the last second |
| `dips` | Dips counted in the last second |
| `history` | All samples from the last second, in volts |
| `stop` | Shuts the program down |
| Enter | Repeats the last command |

Every second the program prints a stats line like this (LED @ 10Hz):
```
#Smpl/s =  886 Flash @  10Hz avg =  1.781V dips =  10 Smpl ms[ 1.085,  1.277] avg  1.129/ 886
```

## Results
| **Test** | **Result** |
|---|---|
| Sample rate | 874-886 samples/s. 1.08-1.4 ms between samples |
| Dip detection | Kept up with the LED to about 60 Hz (62 Hz gave 61-62 dips per second) |
| Memory | Valgrind: no leaks and 0 errors after shutting down with `stop` |

## noworky
The `noworky/` folder is a debugging exercise from the course: a program that crashed with a segfault. It was debugged remotely, with gdbserver on the board and gdb-multiarch on the host PC. The cause was an `unsigned` loop counter. It wrapped around to 4,294,967,295 instead of going negative, so the loop never stopped and read past the end of the array. The fix (changing it to int) is commented in the file.

## Credits
The assignment and course material are by Dr. Brian Fraser and Dr. Matthew Stewart. `app/periodTimer.c/.h` and the original `noworky.c` were provided by the course.

