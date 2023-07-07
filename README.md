# USB ArtNET Interface

> **Warning**  
> This is a new hardware iteration and currently under development. The previous hardware iteration is not feature complete, but stable and will no longer be maintained except for bugfixes. This iteration is expected to be stable (but not feature complete) somewhere around 2023Q3 if nothing major happens.</p>

This project is a USB ArtNET interface with four ports, based on the STM32G484. It will register itself as an Ethernet Card using the CDC-NCM protocoll to be automatically recognized by most operating systems (except Windows 10, which needs some manual configuration). It will then bind the ArtNET-implementation to a configurable IP on that virtual ethernet card. The device can be configured either using ArtNET commands for software that supports this, using a web interface on the device and it can optionally be extended to include a display module for configuration directly at the device.

<p float="middle">
    <img src="/Images/device.png" width="400">
    <img src="/Images/device_back.png" width="400">
</p>

## Schematic
The project uses an STM32G484RE, but should work with any STM32G4xxRE variant. The ports are controlled by a MAX3440E each, which will provide some ESD protection in addition to the TVS-Diodes. The device will draw at most 300mA to 400mA, which is easily delivered by USB2.0 ports. Galvanic isolation was omitted because it needs too much space and because it is not recommended for the transmitting ports of DMX-Devices anyway (although it is recommended for receiving ports, but not mandatory).

![](/Images/mainboard.png)

## PCB
The PCB is designed as a four layer board, as with this small size the price was acceptable. The pcb was designed with a `TEKAL 31.29`-case from TEKO in mind, but any case with at least 100x100mm will work.  placing three ports at the rear and one port beside the usb port at the front of the case. The display module is completely optional and is designed to slot into the mainboard and is secured with an additional screw from the front. It is then connected using a 10-pin ribbon cable.

![](/Images/exploded.png)

### BOM
The total cost for this device is currently aproximately 90€. You can save money by printing your own case (25€), finding a cheaper STM32G4xxRE variant at your local dealer (11€), finding cheaper XLR connectors (5€) and skipping on the display module (5€). 

<details>
<summary>Mainboard (~40€)</summary>

| Amount | Type | Value | Footprint |
|-|-|-|-|
| 4 | Capacitor | 1μF | 0603 |
| 7 | Capacitor | 100nF | 0603 |
| 2 | Capacitor | 20pF | 0603 |
| 1 | Capacitor | 10nF | 0603 |
| 1 | Capacitor | 4.7μF | 0603 |
| 8 | TVS Diode | | SMA |
| 1 | USB-C Receptacle | | |
| 1 | FFC Connector | | 1x10, Pitch 1mm |
| 4 | Terminal Block | | 1x3, Pitch 2.54mm |
| 1 | Pin Header | | 1x3, Pitch 2.54mm |
| 1 | Jumper | | 1x2, Pitch 2.54mm |
| 2 | Resistor | 5K1 | 0603 |
| 1 | Resistor | 4K7 | 0603 |
| 4 | Resistor | 130 | 0603 |
| 8 | Resistor | 560 | 0603 |
| 1 | Switch | | Through Hole, Height 6mm |
| 1 | Switch | | Through Hole, Angled, Height 6mm |
| 1 | DIP-Switch | | Through Hole, 2x4, Pitch 2.54mm |
| 1 | ESD Protection | ESDALD05UD4 | SOT23-6 |
| 1 | Linear Regulator | AP2112K-3.3 | SOT23-5 |
| 4 | Transceiver | MAX3440E | SOIC-8 |
| 1 | Crystal | 25MHz | HC49 |
| 1 | μC | STM32G484RE | LQFP64 |
    
</details>

<details>
<summary>Display Board (~5€)</summary>

| Amount | Type | Footprint |
|-|-|-|
| 1 | OLED-Display | 0.96" with I²C |
| 1 | FFC Connector | 1x10, Pitch 1mm |
| 2 | Resistor | 4K7 | 0603 |
| 4 | Switch | Through Hole, Height 10mm |
| 1 | FFC Cable | 1x10, Pitch 1mm, Length ~100mm, Type B (opposing-side contacts) |

</details>

<details>
<summary>Mechanical parts (~40€)</summary>

| Amount | Type | Specs |
|-|-|-|
| 1 | Case | TEKO TEKAL 31.29, or any case that takes Eurocard-sized PCBs (100x100mm) |
| 4 | XLR Connector | An individual mix of Female and Male XLR 3- and 5-Pin connectors, depending on your needs |
| 9 | Screws | M2, Length 15mm, Countersink |
| 9 | Nuts | M2 |
| 1 | Acrylic Window | To be measured |
| 1 | Spacer | M2, Length 6mm |
| 1 | Mainboard PCB | | 
| 1 | Display PCB | |

</details>

Here are two lists on digikey that include all necessary components, except for the mechanical parts and the oled-display. It does contain four XLR-Connectors though.
- [Parts for the Mainboard](https://www.digikey.com/en/mylists/list/8UBRGNUC4X)
- [Parts for the Display-Module](https://www.digikey.com/en/mylists/list/5N5ETFD0BY)

## Firmware
The firmware is completely custom, only based on CMSIS (no HAL) and supports the important parts of the ArtNET protocol. Which port is an input and which an output can be set using the dip switches, but later be overwritten using either ArtNET commands, the WebUI or the display menu. The device also has a reset button to reset the configuration for when things go completely wrong. If the device is completely bricked it can be put into bootloader mode using a jumper on the mainboard.


The device supports mDNS and is always reachable using `artnet.local` in addition to its ip-address. The WebUI allows configuring each individual port with all the settings that ArtNET-Commands also support. You can also change the ip-configuration of the device and its internal minimal DHCP-Server, as well as clearing all settings, rebooting the device and switching the device into DFU/Bootloader mode to flash a new firmware.
![](/Images/webconfig1.png)