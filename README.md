# MaraX Evolution - Main Controller

[<img src="https://github.com/user-attachments/assets/aa3f5071-2165-40e5-ae51-54051c40426c" width="450" alt="">](https://github.com/user-attachments/assets/aa3f5071-2165-40e5-ae51-54051c40426c)

This repository contains the hardware design and firmware for the **MaraX Evolution** main controller board. This project is designed to replace the stock controller of the Lelit Mara X espresso machine, adding advanced features such as PID temperature control, pressure profiling, weighing scale integration, and WiFi connectivity.

## Features

* **Precise Temperature Control:** PID implementation for brew temperature stability.
* **Pressure Profiling:** Control over the pump pressure (requires compatible hardware). Up to 32 custom profiles can be defined (requires the HMI Screen). Profiling via flowrate (requires the scale to be installed) or pressure based on weight or time.
* **Scale Integration:** [MaraX Evolution Scale](https://github.com/andia89/maraxevolution-scale) as an optional upgrade.
* **HMI Support:** Interfacing with the [MaraX Evolution HMI](https://github.com/andia89/maraxevolution-hmi) (also optional) allows to control the machine with a sophisticated HMI device (based on a Nextion display).
* **WiFi & MQTT:** Remote monitoring and control via MQTT. Integration of the coffee machine in your smarthome
* **Hardware:** Reusing as much as possible from the original machine, only the computer, one tube and one cable has to be replaced, otherwise the excellent hardware of the original machine is used (including its temperature calibration)

## Repository Structure

* `controller_board/`: KiCad v6 hardware design files (schematics, PCB layout, and fabrication files).
* `firmware/`: PlatformIO project source code for the ESP32-based controller.


## Hardware Assembly

1.  **PCB Fabrication:** Use the Gerber and Drill files provided in `controller_board/jlcpcb/production_files/` to order the PCB.
2.  **Assembly:** Solder the components according to the BOM (`BOM-controller_board.csv`). The board features an Arduino-Nano ESP32 compatible footprint. See [Assembly](ASSEMBLY.md) for details
3.  **Installation:** Replace the existing Mara X controller with this board, ensuring all connections (sensors, pump, heating element) are mapped correctly.

## Firmware Compilation

The firmware is built using **PlatformIO**.

1.  Install [VSCode](https://code.visualstudio.com/) and the [PlatformIO extension](https://platformio.org/).
2.  Clone this repository.
3.  Open the `firmware` folder in PlatformIO.
4.  Configure `platformio.ini` if necessary (e.g., to select specific build environments or upload ports)
5.  Click the **Build** icon (checkmark) to compile.
6.  Connect your Arduino Nano ESP32 board via USB and click **Upload** (select `env:firmware` build environment).
7.  For subsequent uploads OTA should be enabled (you might have to adjust IP-address in `platformio.ini`) and selecting the `env:firmware-ota` environment

## [User Guide](USERGUIDE.md)

## [Parts list](PARTS.md)

## [Assembly instructions](ASSEMBLY.md)

## Support me

## Limitations
* The MaraX is a very good heatexchanger machine, using quite sophisticated hardware to achieve superior temperature stability than a lot of other HX machines out of the box. It is, however, not a dual boiler machine and as such perfect temperature control is almost impossible. I played around a lot with PID values and was able to come up with some good values (and some pretty sophisticated feed forward control), but still perfect stability is almost impossible (or only if one tunes the PID super conservative, but then it takes hours to heat the machine)
* This is not a cheap project. Most parts can be sourced pretty cheaply, but I used good ADC chips (the ADS1115) and one unfortunately obsolete LM1830 chip for boiler water level detection as well as a pretty expensive power supply for the 12V power. Most of these things could certainly be done cheaper (and maybe someday will). All in all expect to spend a bit of money if you go for the full set (which is recommended).
* In the end the softweare got waaaaay more complicated than I originally anticipated. Some software was written using AI but I went through all of it and made sure it made sense to me. There certainly will be bugs as there is an infinite amount of edge cases I certainly have not considered. Please report any bugs you may find

## Disclaimer & Safety Warning

**PLEASE READ CAREFULLY BEFORE PROCEEDING.**

**1. I am a Hobbyist, Not a Professional**
I am **not** a professional mechanical, electrical, or software engineer. This project was created purely as a personal hobby undertaking for educational purposes. The code and hardware designs provided here are **not** certified, tested to industrial standards, or guaranteed to be bug-free.

**2. Warranty Voiding**
Modifying your espresso machine, uploading custom firmware, or opening the chassis **will almost certainly void your manufacturer’s warranty**. By proceeding with any instructions or files from this repository, you accept full responsibility for the loss of any existing warranty or support from the original manufacturer.

**3. No Liability**
This software and hardware documentation is provided **"AS IS", WITHOUT WARRANTY OF ANY KIND**, express or implied.
* **I am not responsible** for any damage to your machine (bricking the device, burning out components, leaks, etc.).
* **I am not responsible** for any personal injury or property damage (including fire or electrical shock) resulting from the use of this project.
* **You assume all risk** associated with modifying your hardware and running this software.

**4. High Voltage & Pressure Warning**
Espresso machines involve **Mains Electricity (110V/220V)**, **Water**, and **High Pressure**. This is a dangerous combination.
* Never work on the machine while it is plugged in.
* Ensure you have a proper understanding of electrical safety before attempting any hardware modifications.
* Improper handling of the boiler or pressure lines can result in explosion or severe burns.

**Use this project entirely at your own risk.**

If you find anything that you think could be improved (be it software/electrical or mechanical side) please let me know, and feel free to open a Pull request

## Licensing
This project is dual-licensed to protect the work while allowing for personal study and modification.

* **Firmware:** The main source code located in the `/firmware` directory is licensed under the **PolyForm Noncommercial License 1.0.0**. You may modify and use it for personal projects, but you cannot use it for commercial products.
    * *Third-Party Libraries:* This firmware includes modified versions of the following libraries, which remain under their original licenses:
        * [dimmable-light](https://github.com/fabianoriccardi/dimmable-light): Licensed under **LGPL-2.1**.
        * [PID_v1](https://github.com/br3ttb/Arduino-PID-Library/tree/master): Licensed under **MIT License**.
        * [Simple Kalman Filter](https://github.com/denyssene/SimpleKalmanFilter): Licensed under **MIT License**.

* **Firmware:** The hardware designs, schematics, and 3D models located in the /controller_board and /hv_board directories are licensed under the Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0) license.

Please see the LICENSE file in each respective subdirectory for the full legal text.

## Similar projects
Idea for controlling the pump via an AC dimmer has been done in https://github.com/larszi/marax-pressure-mod. A complete overhaul of control software is currently proposed by the great project in https://github.com/variegated-coffee (they try to make a one-fit all solution though)
