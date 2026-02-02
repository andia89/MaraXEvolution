## Assembly instructions

Disconnect your coffee machine from mains voltage before doing any modifications!

<img width="2374" height="1780" alt="image" src="https://github.com/user-attachments/assets/b2bef99c-e74c-4d39-bb34-1229ab425327" />
After SMT service from JLCPCB the PCBs should arrive in the following condition. Several of the small SMD parts still needs manual soldering.
<img width="1336" height="1780" alt="image" src="https://github.com/user-attachments/assets/12f04637-929b-4e32-97e9-930ab41f4924" />
Finished assembled controller board. Please ignore the dodgy soldering :) (I don't have hot air station yet). The LM1830 chip uand the AQV212 chip is socketed. For this specific board I also socketed the Arduino Nano (because during testing I destroyed too many), but beware of limited vertical space. Mil-max (low profile) sockets work.
<img width="1336" height="1780" alt="image" src="https://github.com/user-attachments/assets/1e37c6e1-696d-41fc-987f-07d94724000b" />
Finished assembled HV-board. In order to have enough vertical space it might be necessary to cut down the heatspreader on the TRIAC a bit. I suggest to first solder all the missing SMD parts (Fuse) before soldering the Relais and the power supply
<img width="1336" height="1780" alt="image" src="https://github.com/user-attachments/assets/f11d798c-b8f2-4f87-a943-a14a00daa7bc" />
Bottom view of repurposed Gicar box. All the connectors should fit neatly through the existing holes as is shown here. 
<img width="1336" height="1780" alt="image" src="https://github.com/user-attachments/assets/999779a8-40dc-4ba7-b476-8f0ee90f952a" />
Bottom view of repurposed Gicar box. All the connectors should fit neatly through the existing holes as is shown here. 

(the following three pictures are only relevant if pressure transducer is to be installed)

<img width="2374" height="1780" alt="image" src="https://github.com/user-attachments/assets/bf58da85-ba2a-4dcf-aa4a-16d0d0918a21" />
Replace the existing transparent silicone tube with your 5mm polyurethane tube (food grade). Tie your pressure transducer into the circuit as shown in the picture.
<img width="2374" height="1780" alt="image" src="https://github.com/user-attachments/assets/be84fa4f-fe3a-4688-a5c8-f4e108196a9e" />
Unplug the brown (i.e. the live) wire from the pump and instead connect your own wire to it. This wire will then connect to the modified Gicar box (see following pictures).  

<img width="1336" height="1780" alt="image" src="https://github.com/user-attachments/assets/a6955f76-337c-45ba-98c9-09f81a4d1a6b" />
The pressure transducer gets a molex connector that has its mate at the controller board. 
<img width="1336" height="1780" alt="image" src="https://github.com/user-attachments/assets/f80dea4b-8075-4388-870e-5d993e090e9b" />
Connect the Faston tabs in the following way. Note that the printed schematic on the Gicar box is not correct anymore and the order is slightly different! The order is 230V Live wire - 230V Neutral wire, pump live wire, custom pump dimmer wire (the cable you made yourself), coffee relai live wire.
<img width="1336" height="1780" alt="image" src="https://github.com/user-attachments/assets/bd423a37-a7bf-4c6d-bf0a-2b3288f16874" />
on the top of the controller board, connect the existing connectors as shown in the picture. The order is: Boiler temperature sensor, Boiler fill sensor, Heater SSR relay, Brew lever sensor


<img width="1336" height="1780" alt="image" src="https://github.com/user-attachments/assets/db68a03c-02f9-4e8a-b33e-5b859a19b8ac" />
Bottom low voltage connectors. Order is Water level sensor tank - Heatexchanger temperature sensor, LEDs and switch sensors, custom pressure transducer, and custom scale cable (install is explained in tray scale repo)
<img width="1336" height="1780" alt="image" src="https://github.com/user-attachments/assets/c3b830b4-7888-44e8-86a4-bd78d6e86eda" />


