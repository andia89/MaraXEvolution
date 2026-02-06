# MaraX Evolution - Main Controller
![License](https://img.shields.io/badge/license-PolyForm%20Noncommercial-blue)
![Platform](https://img.shields.io/badge/platform-ESP32-green)
![Status](https://img.shields.io/badge/status-active-success)

### Screenshot
[<img src="https://github.com/user-attachments/assets/aa3f5071-2165-40e5-ae51-54051c40426c" width="450" alt="">](https://github.com/user-attachments/assets/aa3f5071-2165-40e5-ae51-54051c40426c)

### Videos
[<img src="https://github.com/user-attachments/assets/e681b708-b621-4cd1-969d-cbfb5e469f87" height="400">](https://github.com/user-attachments/assets/e88c7372-fec7-41ba-ae10-4c3a202e55e9)
[<img src="https://github.com/user-attachments/assets/5358e4d0-2b67-4217-8e4f-33426ae74307" height="400">](https://github.com/user-attachments/assets/11aead3c-08d9-41a0-b3eb-2458ce605d03)





This repository contains the hardware design and firmware for the **MaraX Evolution** main controller board. This project is designed to replace the stock controller of the Lelit Mara X espresso machine, adding advanced features such as PID temperature control, pressure profiling, weighing scale integration, and WiFi connectivity.

## Features

### Core Control
* **Hybrid Thermal Control:** Utilizes a custom **Feed-Forward + PID algorithm** (`feedForwardHeater`) that estimates heat loss based on ambient vs. boiler temperature, offering better stability than standard PIDs.
* **Dual-PID Architecture:** Dedicated PID loops for Temperature (Boiler/HX) and Pump (Pressure/Flow).
* **Smart State Machine:** Manages states including `COOLING_FLUSH`, `STEAM_BOOST`, and `STANDBY`.
* Performs all tasks the original Lelit MaraX machine does as well (boiler refill, water tank presence detection, relay activations etc...)

### Profiling & Extraction
* **Pressure Profiling:** Real-time pump dimmer control via Zero-Cross detection.
* **Flow Profiling:** (Requires [MaraX Evolution Scale](https://github.com/andia89/MaraXEvolution-Scale/)) Target specific flow rates (g/s) using Kalman-filtered weight data.
* **Gravimetric stopping:** (Requires [MaraX Evolution Scale](https://github.com/andia89/MaraXEvolution-Scale/)) Stop the shot after a set amount of time or when a certain weight is reached
* **Advanced Profiles:** Supports manual control, flat 9-bar shots, or complex JSON-based profiles (Ramped or Stepped).

### Integration & Connectivity
* **WiFi & MQTT:** Full smart home integration. Reports sensors (`boiler_temp`, `pressure`, `weight`) and accepts commands remotely.
* **Telnet CLI:** Built-in command-line interface for debugging and calibration over WiFi.
* **ESP-NOW HMI:** Low-latency connection to the optional [MaraX Evolution HMI](https://github.com/andia89/maraxevolution-hmi).
* **OTA Updates:** Update firmware wirelessly without opening the machine.

### Maintenance
* **Automated Cleaning:** Built-in backflush program (`CLEANING_START` -> `CLEANING_PUMPING` -> `PAUSE`).
* **Scale Calibration:** Integrated wizard for taring and calibrating the load cell.

## Repository Structure

* `controller_board/`: KiCad v6 hardware design files (schematics, PCB layout, and fabrication files).
* `firmware/`: PlatformIO project source code for the ESP32-based controller.


## Hardware Assembly

1.  **PCB Fabrication:** Use the Gerber and Drill files provided in `controller_board/jlcpcb/production_files/` to order the PCB.
2.  **Assembly:** Solder the components according to the BOM (`BOM-controller_board.csv`). The board features an Arduino-Nano ESP32 compatible footprint. See [Assembly](ASSEMBLY.md) for details
3.  **Installation:** Replace the existing Mara X controller with this board, ensuring all connections (sensors, pump, heating element) are mapped correctly.

## Firmware Compilation

The firmware is modular. You can enable/disable hardware features via `platformio.ini` build flags:

```cpp
build_flags = 
    -D HAS_PRESSURE_GAUGE
    -D HAS_SCALE
    -D HAS_SCREEN
```

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

I had way too much fun building the MaraX Evolution—it turned into quite the obsession! I am definitely planning to tackle another coffee machine and give it the same "Evolution" treatment.

**However, espresso machines are expensive! :D**

If you enjoy this project or use it daily, please consider supporting me.

**Your Support = Your Vote**

When you support me on Ko-fi or Patreon, **please leave a comment telling me which coffee machine you want me to do next.**

Think of it as a voting system: As soon as I have enough funds to cover the machine with the most votes (and the parts to mod it), I will buy it, reverse-engineer it, and build an open-source controller for it. If you have the MaraX and still want to support me, you can also tell me if there is a specific feature you want to have!

I will keep track of the money earned and will update occasionally the votes on the different coffee machines that are requested

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
