# Assembly Instructions

> [!DANGER]
> **SAFETY WARNING:** Disconnect your coffee machine from mains voltage before attempting any modifications! Working with high voltage is dangerous.

## Phase 1: PCB Assembly

### 1. Receive and Inspect Boards
[<img src="https://github.com/user-attachments/assets/b2bef99c-e74c-4d39-bb34-1229ab425327" width="450" alt="PCBs as arrived from factory">](https://github.com/user-attachments/assets/b2bef99c-e74c-4d39-bb34-1229ab425327)

If you ordered SMT assembly (e.g., from JLCPCB), your boards should arrive in the condition shown above. Note that several small SMD parts will still require manual soldering.

### 2. Controller Board Assembly
[<img src="https://github.com/user-attachments/assets/12f04637-929b-4e32-97e9-930ab41f4924" width="450" alt="Assembled controller board">](https://github.com/user-attachments/assets/12f04637-929b-4e32-97e9-930ab41f4924)

Above is the fully assembled controller board. 

**Key Assembly Notes:**
* **Socketing:** The **LM1830** chip and the **AQV212** chip can be socketed as shown in the picture.
* **Arduino Nano:** In the picture the Arduino Nano is also socketed (too many have been destroyed during testing), but be mindful about vertical space constraints. Standard headers are too tall. Use **Mil-max (low profile) sockets** to ensure the board fits inside the housing.
* *(Please ignore the "dodgy" soldering in the reference photo—a hot air station is recommended for cleaner results!)*

### 3. HV-Board Assembly
[<img src="https://github.com/user-attachments/assets/1e37c6e1-696d-41fc-987f-07d94724000b" width="450" alt="Assembled HV board">](https://github.com/user-attachments/assets/1e37c6e1-696d-41fc-987f-07d94724000b)

Above is the fully assembled High Voltage (HV) board.

**Key Assembly Notes:**
* **Heatspreader:** You may need to trim/cut down the heatspreader on the **TRIAC** to ensure there is enough vertical clearance within the enclosure.
* **Order of Operations:** It is highly suggested to solder all missing SMD parts (such as the Fuse) **before** soldering the bulky components like the Relays and the Power Supply.

---

## Phase 2: Housing & Preparation

### 4. Gicar Box Retrofit
[<img src="https://github.com/user-attachments/assets/f11d798c-b8f2-4f87-a943-a14a00daa7bc" width="450" alt="Gicar box bottom view 1">](https://github.com/user-attachments/assets/f11d798c-b8f2-4f87-a943-a14a00daa7bc)
[<img src="https://github.com/user-attachments/assets/999779a8-40dc-4ba7-b476-8f0ee90f952a" width="450" alt="Gicar box bottom view 2">](https://github.com/user-attachments/assets/999779a8-40dc-4ba7-b476-8f0ee90f952a)

Repurpose the existing Gicar control box. As shown in the bottom views above, all connectors should fit neatly through the existing holes without modification to the plastic.

---

## Phase 3: Pressure Transducer (Optional)

*Note: The following three steps are only relevant if you are installing the optional pressure transducer.*

### 5. Tubing Installation
[<img src="https://github.com/user-attachments/assets/a6955f76-337c-45ba-98c9-09f81a4d1a6b" width="450" alt="Transducer molex connector">](https://github.com/user-attachments/assets/a6955f76-337c-45ba-98c9-09f81a4d1a6b)

Replace the existing transparent silicone tube with a **5mm polyurethane tube (food grade)**. Tie your pressure transducer into the circuit as shown in the image.

### 6. Pump Wiring Modification
[<img src="https://github.com/user-attachments/assets/bf58da85-ba2a-4dcf-aa4a-16d0d0918a21" width="450" alt="Pressure transducer tubing">](https://github.com/user-attachments/assets/bf58da85-ba2a-4dcf-aa4a-16d0d0918a21)
[<img src="https://github.com/user-attachments/assets/be84fa4f-fe3a-4688-a5c8-f4e108196a9e" width="450" alt="Pump wiring modification">](https://github.com/user-attachments/assets/be84fa4f-fe3a-4688-a5c8-f4e108196a9e)

1.  Unplug the **Brown** (Live) wire from the pump.
2.  Connect your own custom wire to the pump's Live terminal instead.
3.  This new wire will connect to the modified Gicar box (detailed in Phase 4).

### 7. Transducer Connection
[<img src="https://github.com/user-attachments/assets/9462ca87-df7b-42ad-8284-b255f486efc5" width="450" alt="Transducer molex connector5">](https://github.com/user-attachments/assets/9462ca87-df7b-42ad-8284-b255f486efc5)

Terminate the pressure transducer wires with a Molex connector. This will mate with the corresponding header on the controller board. Make sure the order is exactly as shown in the picture with red being 12V, black being GND and yellow the signal cable.

---

## Phase 4: Final Wiring

### 8. High Voltage (Faston) Connections
[<img src="https://github.com/user-attachments/assets/db68a03c-02f9-4e8a-b33e-5b859a19b8ac" width="450" alt="Bottom controller connections">](https://github.com/user-attachments/assets/db68a03c-02f9-4e8a-b33e-5b859a19b8ac)

Connect the Faston tabs to the Gicar box.

> [!WARNING]
> **Important:** The original schematic printed on the Gicar box plastic is **NO LONGER CORRECT**. You must follow the new pinout order below.

**Pinout Order (Left to Right / as shown):**
1.  **230V Live Wire**
2.  **230V Neutral Wire**
3.  **Pump Live Wire**
4.  **Custom Pump Dimmer Wire** (The cable you created in Step 6)
5.  **Coffee Relay Live Wire**

### 9. Top Controller Connections
[<img src="https://github.com/user-attachments/assets/f80dea4b-8075-4388-870e-5d993e090e9b" width="450" alt="Gicar box Faston connections">](https://github.com/user-attachments/assets/f80dea4b-8075-4388-870e-5d993e090e9b)

On the top side of the controller board, connect the existing machine connectors in the following order:

* **Boiler Temperature Sensor**
* **Boiler Fill Sensor**
* **Heater SSR Relay**
* **Brew Lever Sensor**

### 10. Bottom Controller Connections (Low Voltage)
[<img src="https://github.com/user-attachments/assets/c3b830b4-7888-44e8-86a4-bd78d6e86eda" width="450" alt="Final assembly view">](https://github.com/user-attachments/assets/c3b830b4-7888-44e8-86a4-bd78d6e86eda)

Connect the low voltage peripherals to the bottom of the board in the following order:

* **Water Level Sensor (Tank)**
* **Heat Exchanger (HX) Temperature Sensor**
* **LEDs and Switch Sensors**
* **Custom Pressure Transducer**
* **Custom Scale Cable** *(Refer to the separate Tray Scale repository for scale installation instructions)*
