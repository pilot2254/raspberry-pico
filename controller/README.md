# pico joystick mouse

## how to wire up this bullshit

| r1 stick pin | pico pin                                       |
|--------------|------------------------------------------------|
| `+5V`        | `3V3(OUT)` (dont connect this to `VBUS` or 5v) |
| `GND`        | `GND`                                          |
| `VRX`        | `GP26` / `ADC0`                                |
| `VRY`        | `GP27` / `ADC1`                                |
| `SW`         | `GP14`                                         |

the stick says `+5V`, but the pico's pins use 3.3v. powering the stick from `3V3(OUT)` keeps its signal pins in the safe range for the pico

## what the code does

- `adc_read()` measures the voltage from `VRX` and `VRY` as numbers from 0 to 4095
- `axis_speed()` treats about 2048 as the stick's center, ignores a small area around it, and turns the rest into cursor movement
- `SW` is the stick's push button. it reads low when pressed, which becomes a left mouse click
- tinyusb sends those readings to windows as ordinary usb mouse reports. no windows program is needed

## building it

build and flash the `controller` project with the raspberry pi pico c/c++ extension or your pico sdk setup. plug the pico's usb socket into the pc. windows should recognize it as a mouse

tilt the stick to move the cursor, and press the stick to click. if the cursor creeps while the stick is centered, increase `180` in `axis_speed()` in `controller.cpp`. if an axis moves the wrong way, the `invert` option in that function can flip it

any problems? just take a look at the docs