```json
{
  "board_name": "Bluepill-QuadFC-Carrier",
  "one_liner": "STM32-Bluepill Quadcopter-Flugcontroller-Traegerboard mit IMU-Modulsockel, 4x ESC-PWM-Ausgaengen, RC-Empfaenger-Header, Buzzer und Status-LED.",
  "confidence": "high",
  "price_eur": 17
}
```

## BUILD-PROMPT

### 1. Board-Spezifikation & DFM
* **Abmessungen:** 80,0 mm × 80,0 mm, 4-lagig (F.Cu/B.Cu Signal, In1.Cu GND-Plane, In2.Cu +5V-Plane), FR4 1,6 mm.
* **Nur THT-Bauteile** (maximale Handloet-Tauglichkeit). Alle Sensor-/IMU-Teile als aufsteckbares Modul (Buchsenleiste), keine SMD-ICs.
* **Spurbreiten:** Signal (PWM/GPIO/I2C) ≥ 0,4 mm; Versorgung (+5V/GND) ≥ 1,0 mm.
* **Clearance:** ≥ 0,3 mm. Bauteile weiträumig platzieren, damit Freerouting sicher konvergiert.

### 2. Mechanik
* Nullpunkt (0,0) unten links. Board 80×80 mm.
* **4× M3-Bohrung** (Ø 3,2 mm) bei (5,5), (75,5), (75,75), (5,75); 3,0 mm Keepout.

### 3. Bauteilliste (nur Standard-KiCad-Bibliotheken)
1. **J_BP1, J_BP2 (Bluepill-Sockel):** 2× Buchsenleiste 1x20, Reihenabstand exakt 0.9″ (22,86 mm).
   * `Connector_PinSocket_2.54mm:PinSocket_1x20_P2.54mm_Vertical`
2. **J_IMU (IMU-Modulsockel, GY-521/MPU-6050):** 1× Buchsenleiste 1x08 (VCC, GND, SCL, SDA, XDA, XCL, AD0, INT).
   * `Connector_PinSocket_2.54mm:PinSocket_1x08_P2.54mm_Vertical`
3. **J_M1..J_M4 (ESC-Ausgaenge):** 4× Stiftleiste 1x03 (Signal, +5V, GND) fuer die 4 ESCs.
   * `Connector_PinHeader_2.54mm:PinHeader_1x03_P2.54mm_Vertical`
4. **J_RX (RC-Empfaenger):** 1× Stiftleiste 1x06 (RX_CH1..CH4, +5V, GND).
   * `Connector_PinHeader_2.54mm:PinHeader_1x06_P2.54mm_Vertical`
5. **J_PWR (BEC/5V-Eingang):** 1× Schraubklemme 2-polig (+5V, GND).
   * `TerminalBlock_Phoenix:TerminalBlock_Phoenix_MKDS-1,5-2-5.08_1x02_P5.08mm_Horizontal`
6. **D_PWR:** 1× Schottky 1N5819 (Verpolschutz in Serie zu +5V).
   * `Diode_THT:D_DO-41_SOD81_P10.16mm_Horizontal`
7. **C_BULK:** 1× Elko 470 µF / 16 V. `Capacitor_THT:CP_Radial_D8.0mm_P3.50mm`
8. **C_DEC:** 1× Keramik 100 nF. `Capacitor_THT:C_Disc_D3.0mm_W1.6mm_P2.50mm`
9. **BZ (Buzzer-Header):** 1× Stiftleiste 1x02 (aktiver Piezo-Buzzer). `Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical`
10. **LED1 (Status):** 1× LED 5 mm. `LED_THT:LED_D5.0mm`
11. **R_LED:** 1× 330 Ω. `Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal`
12. **R_SDA, R_SCL:** 2× 4,7 kΩ I2C-Pullups nach +3V3. `Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P7.62mm_Horizontal`

### 4. Netze (Schaltplan)
* **Versorgung:** J_PWR.1(+5V_IN) → Anode D_PWR; Kathode D_PWR → Netz **+5V**. +5V → C_BULK(+), C_DEC(+), Bluepill 5V-Pin, J_IMU.VCC, J_M1..4 Pin2, J_RX.5. Alle GND (Bluepill GND, C_BULK−, C_DEC−, J_IMU.GND, J_M1..4 Pin3, J_RX.6, BZ.2, J_PWR.2) → **GND**.
* **3V3-Rail:** Bluepill 3V3 → Netz **+3V3** → R_SDA, R_SCL (Pullup-Versorgung).
* **I2C (IMU):** Bluepill **PB6=SCL** → J_IMU.SCL + R_SCL; **PB7=SDA** → J_IMU.SDA + R_SDA. J_IMU.INT → Bluepill **PB5**. J_IMU.AD0 → GND.
* **ESC-PWM (TIM2):** Bluepill **PA0→J_M1.1, PA1→J_M2.1, PA2→J_M3.1, PA3→J_M4.1**.
* **RC-Empfaenger:** Bluepill **PB0→J_RX.1, PB1→J_RX.2, PB10→J_RX.3, PB11→J_RX.4** (4 PWM-Eingaenge / oder 1 davon PPM).
* **Buzzer:** Bluepill **PA8 → BZ.1** (BZ.2 → GND).
* **Status-LED:** Bluepill **PC13 → R_LED → LED1 Anode**, LED1 Kathode → GND.

### 5. Layout
* Bluepill mittig (Reihenabstand 22,86 mm). IMU-Sockel **so zentral wie moeglich** (Vibration/Schwerpunkt), nahe Boardmitte.
* Die 4 ESC-Header (J_M1..4) in die 4 Ecken (Motorpositionen) legen, beschriftet **M1 (v.r.), M2 (h.r.), M3 (h.l.), M4 (v.l.)** auf F.SilkS.
* J_PWR und J_RX an gegenueberliegende Raender. Silk beschriften: +5V, GND, RX1-4, SCL, SDA, Buzzer.

### 6. Workflow (MCP)
1. Projekt init, Schaltplan mit klaren Net-Labels + Footprints. 2. PCB, Edge.Cuts 80×80, 4× M3. 3. 4 Lagen: In1.Cu GND-Zone + In2.Cu +5V-Zone ueber die ganze Flaeche ZUERST. 4. Bauteile weiträumig platzieren. 5. Freerouting (Signal 0,4 / Power 1,0 mm) NUR F.Cu/B.Cu. 6. DRC 0 Fehler. 7. Gerber+Drill nach ./gerbers. 8. Ehrliche Zusammenfassung.
