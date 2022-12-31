# USB DMX Interface
This project is a usb dmx interface with four ports, based on the STM32G441. It will register itself as an FX5-Clone as to be recognized by most software. It will register four HID-Devices, one for each port.

![](https://github.com/CShark/usb_dmx/raw/master/Images/project.jpg)

## Schematic
The project uses an STM32G441RB clocked at ~140MHz. The ports are controlled by a MAX3440E each, which will provide some ESD protection in addition to the TVS-Diodes. The device will draw at most 300mA, galvanic isolation was omitted because it needs too much space :D, and because it is not recommended for the transmitting ports of DMX-Devices anyway (although it is recommended for receiving ports, but not mandatory).

![](https://github.com/CShark/usb_dmx/raw/master/Images/schematic.png)

## PCB
The PCB is designed as a four layer board, as with this small size the price was acceptable. The pcb was designed with a `TEKAL 31.29`-case from TEKO in mind, placing three ports at the rear and one port beside the usb port at the front of the case.
![](https://github.com/CShark/usb_dmx/raw/master/Images/render.jpg)
![](https://github.com/CShark/usb_dmx/raw/master/Images/render_2.jpg)

## Firmware
The firmware is completely custom, only based on CMSIS (no HAL) and aims to be compatible with the FX5-Device, which is no longer in production. In theory, the ports can be configured using the resistors at the backside of the pcb but I did not program the logic for it, so currently the first three are Outputs and the fourth is an Input. The configuration can easily be changed at the beginning of main.

To adjust the name of the interface, look at `USB_GetString` in the `usb_config.c`.