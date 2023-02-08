# USB ArtNET Interface
This project is a usb ArtNET interface with four ports, based on the STM32G441. It will register itself as an Ethernet Card as to be recognized by most software. It will then bind the ArtNET-implementation to a configurable IP on that virtual ethernet card.

![](https://github.com/CShark/usb_dmx/raw/master/Images/project.jpg)

## Schematic
The project uses an STM32G441RB clocked at ~140MHz. The ports are controlled by a MAX3440E each, which will provide some ESD protection in addition to the TVS-Diodes. The device will draw at most 300mA, galvanic isolation was omitted because it needs too much space :D, and because it is not recommended for the transmitting ports of DMX-Devices anyway (although it is recommended for receiving ports, but not mandatory).

![](https://github.com/CShark/usb_dmx/raw/master/Images/schematic.png)

## PCB
The PCB is designed as a four layer board, as with this small size the price was acceptable. The pcb was designed with a `TEKAL 31.29`-case from TEKO in mind, placing three ports at the rear and one port beside the usb port at the front of the case.
![](https://github.com/CShark/usb_dmx/raw/master/Images/render.jpg)
![](https://github.com/CShark/usb_dmx/raw/master/Images/render_2.jpg)

## Firmware
The firmware is completely custom, only based on CMSIS (no HAL) and supports the important parts of the ArtNET protocol. The initial port configuration can be set using resistors, but later be overwritten using the Command parameter of the ArtAddress-packet. The device also registers an emergency serial port, for when you misconfigure the ethernet addresses and loose connection to the device. It allows you to change the address and config on the fly and also to switch into the bootloader. The configuration can be done by the included tool that mainly uses regular ArtNET-packets for configuration but also supports the emergency serial commands if it finds a serial port for the device.

By default the device will spin up a DHCP server that supports two clients in the 10.0.0.0 network.

To adjust the hardcoded name of the ethernet card in the device manager, look at `USB_GetString` in the `usb_config.c`.