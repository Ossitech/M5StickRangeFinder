# M5StickRangeFinder
This is a small project that aims to drive a Benewake TF02 range finder module with the M5StickC to create a small and prortable range finder.

## Hardware setup
You basically only need:
* Benewake TF02-Pro range finder
* M5 Stick-C

> This should also work with the newer M5 Stick Variants like the plus or the plus v2 but I did not test
these.

This project uses the range finder in UART-Mode but the module can also communicate
over I²C. Both communication methods have their benefits.
Look into the [user manual](/TF02-Pro%20Manual.pdf) if you want to change the configuration
of your range finder module.

### Connecting the range finder with your M5 Stick-C
The range finder has 4 connections.
I got the range finder without the standard cable connector.
Female gpio ports where soldered to the individual wires,
so see wire colors where visible.
In my case the wires had the following colors:
* 5V (red)
* Ground (black)
* Rx (white)
* Tx (green)

I connected them to the M5 Stick-C like this:
| TF02-Pro | M5 Stick-C |
|----------|------------|
| 5V | 5V |
| Ground | Ground |
| Rx | Tx (G0) |
| Tx | Rx (G26) |

These are all hardware connections needed for this project to work.

## Things I learned with this project
### Serial setup
The M5 Stick-C has 3 serial ports (Serial, Serial1 and Serial2).
The first one is used inside the USB-Connection and for flashing and
the other two can be used freely.
You can also use the first port if you don't need
the serial monitor to work over the usb connection.
I choose the 3rd port for communicating with the range finder module.
The ESP32 on which the M5 Stick-C is based on allows
setting different rx and tx pins for the serial ports.
Not every pin can be used, that's why I didn't
use pin G36. The board would crash when I tried
to initialize the serial port using this pin.
But with G26 and G0 it works fine.
These pins are part of the upper pin header of
the M5 Stick-C and are right next to
the 5V and Ground pins.
To set this up I used:
```
Serial2.begin(115200, (uint32_t)SERIAL_8N1, G26, G0);
```
This uses the default baudrate and parity settings of the TF02-Pro.

### Power consumption
The range finder module can easily be powered by the M5-Stick,
because it has the 5V output pin.
Even though the builtin battery is not very big,
it lasts long enough to take many measurements.
And because you typically won't keep
the device switched on for long periods of time,
the small capacity doesn't really matter.
