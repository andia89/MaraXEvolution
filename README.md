# MaraX Evolution - Main Controller

This repository contains the hardware design and firmware for the **MaraX Evolution** main controller board. This project is designed to replace the stock controller of the Lelit Mara X espresso machine, adding advanced features such as PID temperature control, pressure profiling, weighing scale integration, and WiFi connectivity.

## Features

* **Precise Temperature Control:** PID implementation for boiler temperature stability.
* **Pressure Profiling:** Control over the pump pressure (requires compatible hardware).
* **Scale Integration:** Native support for the [MaraX Evolution Scale](https://github.com/andia89/maraxevolution-scale).
* **HMI Support:** Interfacing with the [MaraX Evolution HMI](https://github.com/andia89/maraxevolution-hmi) (Nextion display).
* **WiFi & MQTT:** Remote monitoring and control via MQTT.

## Repository Structure

* `controller_board/`: KiCad v6 hardware design files (schematics, PCB layout, and fabrication files).
* `firmware/`: PlatformIO project source code for the ESP32-based controller.

## Parts list
 * PCB for controller board and HV board (use the .bom and .cpl file if you want to use STM service from JLCPCB for the small resistors/capacitors and Transistors/Diodes). Otherwise use suitable 0850 parts and Transistors and Diodes as described in the kicad files. **R4**, **R12**, **R13** and **R2** I suggest to use lower tolerance (e.g. 0.1%) 0850 resistors since they need to be fairly accurate as they are used for voltage divider. As such they are not included in the BOM and CPL file for JLCPCB.
 * Electronic components (these are the components that are not done by the SMT service of JLCPCB and need to be handsoldered)

## Hardware Assembly

1.  **PCB Fabrication:** Use the Gerber and Drill files provided in `controller_board/jlcpcb/production_files/` to order the PCB.
2.  **Assembly:** Solder the components according to the BOM (`BOM-controller_board.csv`). The board features an ESP32/Arduino-compatible footprint.
3.  **Installation:** Replace the existing Mara X controller with this board, ensuring all connections (sensors, pump, heating element) are mapped correctly.

## Firmware Compilation

The firmware is built using **PlatformIO**.

1.  Install [VSCode](https://code.visualstudio.com/) and the [PlatformIO extension](https://platformio.org/).
2.  Clone this repository.
3.  Open the `firmware` folder in PlatformIO.
4.  Configure `platformio.ini` if necessary (e.g., to select specific build environments or upload ports).
5.  Click the **Build** icon (checkmark) to compile.
6.  Connect your controller board via USB and click **Upload** (arrow).

## Dependencies

This firmware relies on the following libraries (automatically handled by PlatformIO):
* `PID_v1`
* `SimpleKalmanFilter`
* `dimmable_light`
* `NextionX2`

## Firmware setup

## Parts List

### Controller Board

The BOM for the controller board is divided into **SMD components** (suitable for JLCPCB's SMT service) and **Handsolder components** (precision parts, connectors, and the microcontroller to be sourced from Mouser/DigiKey).

#### 1. SMD Components (JLCPCB SMT Service)
These components have LCSC part numbers included in the BOM and can be assembled by the JLCPCB SMT service.

| Comment | Designator | Footprint | LCSC Part | Qty |
| :--- | :--- | :--- | :--- | :--- |
| **0** | R15 | R 0805 | [C17477](https://lcsc.com/product-detail/C17477.html) | 1 |
| **0.05uF** | C6 | C 0805 | [C53134](https://lcsc.com/product-detail/C53134.html) | 1 |
| **0.22u** | C3 | C 0805 | [C5378](https://lcsc.com/product-detail/C5378.html) | 1 |
| **100nF** | C7, C8, C9, C11 | C 0805 | [C28233](https://lcsc.com/product-detail/C28233.html) | 4 |
| **1nF** | C4 | C 0805 | [C46653](https://lcsc.com/product-detail/C46653.html) | 1 |
| **1uF** | C1, C2, C5, C10 | C 0805 | [C28323](https://lcsc.com/product-detail/C28323.html) | 4 |
| **10K** | R1, R3, R5, R7, R9, R10, **R13**, R18, R20, R22, R24, R28, R30, R31, R32, R33, R35, R38 | R 0805 | [C17414](https://lcsc.com/product-detail/C17414.html) | 18 |
| **20uF** | C13 | C 0805 | [C45783](https://lcsc.com/product-detail/C45783.html) | 1 |
| **47uF** | C12 | C 0805 | [C28323](https://lcsc.com/product-detail/C28323.html) | 1 |
| **220** | R23, R25, R26, R34, R36, R39 | R 0805 | [C17557](https://lcsc.com/product-detail/C17557.html) | 6 |
| **300** | R8 | R 0805 | [C17617](https://lcsc.com/product-detail/C17617.html) | 1 |
| **30K** | R17, R19, R21 | R 0805 | [C17621](https://lcsc.com/product-detail/C17621.html) | 3 |
| **470** | R14, R27, R37, R40 | R 0805 | [C17710](https://lcsc.com/product-detail/C17710.html) | 4 |
| **47K** | R6, R29 | R 0805 | [C17713](https://lcsc.com/product-detail/C17713.html) | 2 |
| **BSS138** | Q1, Q2, Q3, Q4, Q5, Q6 | SOT-23 | [C15277](https://lcsc.com/product-detail/C15277.html) | 6 |

#### 2. Handsolder Components (Mouser / DigiKey)
These parts are not included in the SMT assembly and must be purchased separately.

**Precision Resistors**
*0.1% tolerance is recommended for these voltage dividers.*

| Value | Designator | Qty | Link (Mouser) |
| :--- | :--- | :--- | :--- |
| **1K5** | R12 | 1 | [0805 1.5K 0.1%](https://www.mouser.com/c/?q=0805%20resistor%201.5kOhms%200.1%25) |
| **5K6** | R11 | 1 | [0805 5.6K](https://www.mouser.com/c/?q=0805%20resistor%205.6kOhms) |
| **10K** | R2, R4 | 2 | [0805 10K 0.1%](https://www.mouser.com/c/?q=0805%20resistor%2010kOhms) |
| **-** | **R16** | **0** | **DNP (Do Not Populate)** |

**ICs & Modules**

| Part Name | Designator | Description | Qty | Link (Mouser) |
| :--- | :--- | :--- | :--- | :--- |
| **Arduino Nano ESP32**| A1 | Microcontroller | 1 | [Arduino Nano ESP32](https://www.mouser.com/c/?q=Arduino%20Nano%20ESP32) |
| **ADS1115IDGS** | U1 | 16-Bit ADC | 1 | [ADS1115IDGS](https://www.mouser.com/c/?q=ADS1115IDGS) |
| **AQV212** | IC1 | PhotoMOS Relay | 1 | [AQV212](https://www.mouser.com/c/?q=AQV212) |
| **IE092503-1** | U3 | DC-DC Converter | 1 | [IE092503-1](https://www.mouser.com/c/?q=IE092503-1) |
| **LM4050AEM3-3.0** | IC2 | Voltage Reference 3.0V | 1 | [LM4050AEM3-3.0](https://www.mouser.com/c/?q=LM4050AEM3-3.0) |

**Connectors**

| Part Name | Designator | Description | Qty | Link (Mouser) |
| :--- | :--- | :--- | :--- | :--- |
| **280377-2** | CN1, CN2, CN3, CN4, CN5, CN7 | TE Ampmodu II (2 pos) | 6 | [TE 280377-2](https://www.mouser.com/c/?q=280377-2) |
| **280389-2** | CN6 | TE Ampmodu II (2 pos) | 1 | [TE 280389-2](https://www.mouser.com/c/?q=280389-2) |
| **70553-0002** | J4 | Molex SL Header (3 pos)<br>*(Optional: If Pressure Transducer is installed)* | 1 | [Molex 70553-0002](https://www.mouser.com/c/?q=705530002) |
| **70553-0041** | J5 | Molex SL Header (7 pos) | 1 | [Molex 70553-0041](https://www.mouser.com/c/?q=705530041) |
| **Pin Header 1x06** | J2 | 2.00mm Pitch Header | 1 | [Header 1x06 2mm](https://www.mouser.com/c/?q=Pin%20Header%201x06%202.00mm) |

#### 3. Special Components
This component is obsolete or hard to find and typically requires sourcing from eBay.

| Component | Designator | Note | Link |
| :--- | :--- | :--- | :--- |
| **LM1830N** | U2 | Fluid Level Detector | [Search on eBay](https://www.ebay.com/sch/i.html?_nkw=LM1830N) |

## Assembly instructions

## Support me


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
