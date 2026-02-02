
## Parts List

 * PCBs for both the controller board and the HV board. I had good experience with JLCPCB but PCBWay is also fine. The gerber files can be found in production_files in the jlcpcb folder. 

* Electronic parts for assembly. They are listed at the end of this section in [Controller board](#conroller-board) and [HV board](#hv-board)

* If you want to use a pressure transducer (this is necessary to do profiling based on pressure, and to electronically monitor the pressure). 
    * The board is designed for [this one](https://aliexpress.com/item/4000756631924.html?spm=a2g0o.order_list.order_list_main.5.ee9e1802cAHCQO); specifically 0-1.6MPa at 0.5V-4.5V and a supply voltage of 12V
    * [T-Piece for pipe](https://www.landefeld.de/artikel/de/t-steckanschluss-5mm-5mm-iqs-msv-standard-/IQST%2050%20MSV). It is important to note that the MaraX uses 5mm pipes which are quite non-standard
    * [Pipe push fitting](https://www.landefeld.de/artikel/de/steckanschluss-m-innengew-m-5-4mm-iqs-msv-standard-/IQSF%20M54%20MSV)
    * [5mm polyurethane pipe](https://www.landefeld.de/artikel/de/polyurethan-schlauch-5-x-3-mm-blau-meterware-von-50-mtr-rolle-/PU%205X3%20BLAU); get 1m to have a bit left-over
    * [Molex connector](https://www.mouser.at/ProductDetail/Molex/50-57-9403?qs=u6Gr9%2FNt%252B%2F9Ok3bHhq8UPA%3D%3D) that has its mate on the controller board, with at least [3 Receptables that need to be crimped on the wire](https://www.mouser.at/ProductDetail/Molex/16-02-0102?qs=UAyrm%2FnZ%252BCifWkt7bqgfPw%3D%3D)
* If you want to use AC dimmer for flow/pressure profiling (and you either have the Pressure transducer and/or the scale installed) one additional (ideally heat-resistant cable) is necessary. Two [Connectors](https://www.mouser.at/ProductDetail/571-606501) like this one have to be crimped on a sufficiently long cable. 

### Controller Board

The BOM for the controller board is divided into **SMD components** (suitable for JLCPCB's SMT service) and **Handsolder components** (precision parts, connectors, and the microcontroller to be sourced from Mouser/DigiKey).

#### 1. SMD Components (JLCPCB SMT Service)
These components have LCSC part numbers included in the BOM and can be assembled by the JLCPCB SMT service. Alternatively they can be of course hand soldered. These Resistors and Capacitors are mainly used for Pull-up/down, current limiting and filtering, as such their exact values are not that important. The transistors are mainly used for switching LEDs. In both controller_board and hv_board folder there is a jlcpcb folder that contains all the necessary files for fabrication as well as SMT assembly.

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
*0.1% tolerance is recommended for these voltage dividers. If this is really necessary I am not sure, but I used 0.1% tolerance ones and it works great*

| Value | Designator | Qty | Link (Mouser) |
| :--- | :--- | :--- | :--- |
| **1K5** | R12 | 1 | [0805 1.5K 0.1%](https://www.mouser.com/c/?q=0805%20resistor%201.5kOhms%200.1%25) |
| **5K6** | R11 | 1 | [0805 5.6K 0.1%](https://www.mouser.com/c/?q=0805%20resistor%205.6kOhms%200.1%25) |
| **10K** | R2, R4 | 2 | [0805 10K 0.1%](https://www.mouser.com/c/?q=0805%20resistor%2010kOhms%200.1%25) |
| **-** | **R16** | **0** | **DNP (Do Not Populate)** |

**ICs & Modules**

| Part Name | Designator | Description | Qty | Link (Mouser) | Comment | 
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Arduino Nano ESP32**| A1 | Microcontroller | 1 | [Arduino Nano ESP32](https://www.mouser.com/c/?q=Arduino%20Nano%20ESP32) | with headers if directly soldered to the board. Otherwise [Mill-Max](https://www.mill-max.com/products/new/precision-machined-pin-headers) headers are a way to socket it (there is not enough vertical space for standard machine headers)
| **ADS1115IDGS** | U1 | 16-Bit ADC | 1 | [ADS1115IDGS](https://www.mouser.com/c/?q=ADS1115IDGS) | Any ADS1115 will work fine
| **AQV212** | IC1 | PhotoMOS Relay | 1 | [AQV212](https://www.mouser.com/c/?q=AQV212%20DIP-6) | can be socketed using standard DIP-6 socket
| **IE092503-1** | U3 | Buzzer | 1 | [IE092503-1](https://www.mouser.com/c/?q=IE092503-1) | If you don't want a buzzer, can be omitted
| **LM4050AEM3-3.0** | IC2 | Voltage Reference 3.0V | 1 | [LM4050AEM3-3.0](https://www.mouser.com/c/?q=LM4050AEM3-3.0) | A precise voltage reference for temperature measurements

**Connectors**

| Part Name | Designator | Description | Qty | Link (Mouser) | Comment | 
| :--- | :--- | :--- | :--- | :--- | :--- |
| **280377-2** | CN1, CN2, CN3, CN4, CN5, CN7 | TE Ampmodu II (2 pos) | 6 | [TE 280377-2](https://www.mouser.com/c/?q=280377-2) |
| **280389-2** | CN6 | TE Ampmodu II (8 pos) | 1 | [TE 280389-2](https://www.mouser.com/c/?q=280389-2) |
| **70553-0002** | J4 | Molex SL Header (3 pos)| 1 | [Molex 70553-0002](https://www.mouser.com/c/?q=705530002) | Can be omitted if no pressure transducer is to be installed
| **70553-0041** | J5 | Molex SL Header (7 pos) | 1 | [Molex 70553-0041](https://www.mouser.com/c/?q=705530041) | Can be omitted if no scale is to be installed
| **MTMM-106-14-T-S-520** | J2 | 2.00mm Pitch Header | 1 | [Header 1x06 2mm](https://www.mouser.com/c/?q=MTMM-106-14-T-S-520) | This mates with the correspnding ESQT connector on the HV board. The exact part numbers are not that important, but the two connectors need to bridge a gap of 27mm (distance between the topsides of both PCBs)

#### 3. Special Components
This component is obsolete and typically requires sourcing from eBay.

| Component | Designator | Note | Link | Comment |
| :--- | :--- | :--- | :--- | :--- |
| **LM1830N** | U2 | Fluid Level Detector | [Search on eBay](https://www.ebay.com/sch/i.html?_nkw=LM1830N) | Can be socketed by using socket for N14A package

### HV Board

These components have LCSC part numbers included in the BOM and can be assembled by the JLCPCB SMT service. Alternatively they can be of course hand soldered. These Resistors and Capacitors are mainly used for Pull-up/down, current limiting and filtering, as such their exact values are not that important. Transistors are used to switch the Relays and Diodes as flyback. Both can be cheaply done with SMT service from JLCPCB

#### 1. SMD Components (JLCPCB SMT Service)
These components have LCSC part numbers and can be assembled by the JLCPCB SMT service.

| Comment | Designator | Footprint | LCSC Part | Qty |
| :--- | :--- | :--- | :--- | :--- |
| **100** | R8 | R 0805 | [C17408](https://lcsc.com/product-detail/C17408.html) | 1 |
| **10K** | R1, R28 | R 0805 | [C17414](https://lcsc.com/product-detail/C17414.html) | 2 |
| **1k** | R7 | R 0805 | [C17513](https://lcsc.com/product-detail/C17513.html) | 1 |
| **BAT46WJ** <br>*(I would have liked the BAT46WJ but only the BAT46WS is available for SMT service. It is also fine)* | U3, U4 | SC-90 / SOD323F | [C7502692](https://lcsc.com/product-detail/C7502692.html) | 2 |
| **BSS138** | Q1, Q2 | SOT-23 | [C7420339](https://lcsc.com/product-detail/C7420339.html) | 2 |

#### 2. Handsolder Components (Mouser / DigiKey)
These parts involve mains voltage or high power.

**High Power Resistors (2010 Package)**
*Note: These require the larger 2010 [5025 metric] footprint for power handling. Go for Power rating of >1W for those*

| Value | Designator | Qty | Link (Mouser) | Comments |
| :--- | :--- | :--- | :--- | :--- |
| **47k** | R5, R6 | 2 | [2010 Resistor 47k](https://www.mouser.com/c/?q=2010%20resistor%2047k) | Use at least 1W rating. Can be omitted if AC dimmer is not used
| **330** | R9 | 1 | [2010 Resistor 330](https://www.mouser.com/c/?q=2010%20resistor%20330) | Use at least 1W rating. Can be omitted if AC dimmer is not used
| **39** | R11 | 1 | [2010 Resistor 39](https://www.mouser.com/c/?q=2010%20resistor%2039) | Use at least 1W rating. Can be omitted if AC dimmer is not used

**Relays, Triacs & Power Modules**

| Part Name | Designator | Description | Qty | Link (Mouser) | Comment |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **ECE10US12** | U1 | AC-DC Converter 12V | 1 | [ECE10US12](https://www.mouser.com/c/?q=ECE10US12) |
| **ALDP112** | U2 | Panasonic Relay 12V | 1 | [ALDP112](https://www.mouser.com/c/?q=ALDP112) |
| **RT314A12** | K1 | TE Schrack Relay 12V | 1 | [RT314A12](https://www.mouser.com/c/?q=RT314A12) |
| **BTA16-600BWRG**| Q3 | Triac 600V 16A | 1 | [BTA16-600BWRG](https://www.mouser.com/c/?q=BTA16-600BWRG) | Can be omitted if AC dimmer is not used
| **H11L1** | U5 | Optocoupler (Logic) | 1 | [H11L1](https://www.mouser.com/c/?q=H11L1) | Can be omitted if AC dimmer is not used. Can be socketed with DIP-6 socket
| **MOC3021M** | U7 | Optocoupler (Triac) | 1 | [MOC3021M](https://www.mouser.com/c/?q=MOC3021M) | Can be omitted if AC dimmer is not used. Can be socketed with DIP-6 socket
| **W04G** | D1 | Bridge Rectifier | 1 | [W04G](https://www.mouser.com/c/?q=W04G) | Can be omitted if AC dimmer is not used
| **7178DG** | H1 | Heatsink for Triac | 1 | [7178DG](https://www.mouser.com/c/?q=7178DG) | Can be omitted if AC dimmer is not used

**Capacitors & Fuses**

| Part Name | Designator | Description | Qty | Link (Mouser) | Comment |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **VJ2220Y103K...**| C1 | 2220 Safety Cap X2 | 1 | [VJ2220Y103KXUSTX2](https://www.mouser.com/c/?q=VJ2220Y103KXUSTX2) | Can be omitted if AC dimmer is not used
| **100uF** | C3 | Radial (D6.3mm P2.5mm) | 1 | [100uF Radial 6.3mm 2.5mm](https://www.mouser.com/c/?q=100uF%20radial%206.3mm%202.5mm%20pitch) |
| **0034.6808** | F1 | Schurter MSS Fuse | 1 | [0034.6808](https://www.mouser.com/c/?q=0034.6808) |
| **0697H9100-02** | F2 | Bel Fuse Radial | 1 | [0697H9100-02](https://www.mouser.com/c/?q=0697H9100-02) |
| **3413.0215.22** | F4 | Schurter SMD Fuse | 1 | [3413.0215.22](https://www.mouser.com/c/?q=3413.0215.22) |

**Connectors**

| Name | Designator | Description | Qty | Link |
| :--- | :--- | :--- | :--- | :--- |
| **571-160650-2** | FA1, FA4, FA7, FA10, FA11 | Faston Tabs 6.35mm | 5 | [571-160650-2](https://www.mouser.at/ProductDetail/571-160650-2) |
| **ESQT-106-02-L-S-530** | J6 | 2.00mm Pitch Female Header Receptable | 1 | [Socket 1x06 2mm](https://www.mouser.com/c/?q=ESQT-106-02-L-S-530) | This mates with the corresponding MTMM connector on the controller board. The exact part numbers are not that important, but the two connectors need to bridge a gap of 27mm (distance between the topsides of both PCBs)
