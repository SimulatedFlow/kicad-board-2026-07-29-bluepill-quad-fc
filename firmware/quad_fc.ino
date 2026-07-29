/*
 * Bluepill-QuadFC-Carrier — Beispiel-Firmware (STM32duino / Arduino_Core_STM32)
 * Simulated Flow UG · MIT-Lizenz · github.com/SimulatedFlow
 *
 * ZWECK: lauffaehige EDUKATIVE Basis fuer das QuadFC-Traegerboard. Liest die
 * MPU-6050 (I2C, PB6/PB7), 4 RC-Kanaele (PWM-Input) und steuert 4 ESCs (PA0..PA3,
 * TIM2, 50..490 Hz). Einfacher Rate-Mode-Mixer (P auf Gyro). Buzzer PA8, LED PC13.
 *
 * ⚠️ SICHERHEIT: KEIN zertifizierter Flugcontroller. Nur mit ABGENOMMENEN Propellern
 *    testen. Immer erst ARMEN, wenn Gas minimal. Auf eigene Gefahr.
 *
 * Benoetigt: Board "Generic STM32F1 (BluePill F103C8)", Lib "MPU6050" (Electronic Cats)
 *            oder "Adafruit MPU6050". Hier: rohes I2C (Wire) fuer Minimal-Abhaengigkeit.
 */
#include <Wire.h>
#include <Servo.h>

// ---- Pinbelegung (siehe BUILD-PROMPT / Board-Silk) ----
const uint8_t PIN_ESC[4] = {PA0, PA1, PA2, PA3};   // M1 vr, M2 hr, M3 hl, M4 vl
const uint8_t PIN_RX[4]  = {PB0, PB1, PB10, PB11};  // Throttle, Roll, Pitch, Yaw
const uint8_t PIN_BUZZ   = PA8;
const uint8_t PIN_LED    = PC13;
const uint8_t MPU_ADDR   = 0x68;

Servo esc[4];
volatile uint16_t rx_us[4] = {1000, 1500, 1500, 1500};

// ---- MPU-6050 ----
void mpuWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU_ADDR); Wire.write(reg); Wire.write(val); Wire.endTransmission();
}
void mpuInit() {
  Wire.begin();
  mpuWrite(0x6B, 0x00);   // PWR_MGMT_1: wake
  mpuWrite(0x1B, 0x08);   // GYRO_CONFIG: +/-500 deg/s
}
void mpuGyro(float &gx, float &gy, float &gz) {
  Wire.beginTransmission(MPU_ADDR); Wire.write(0x43); Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, (uint8_t)6);
  int16_t x = (Wire.read() << 8) | Wire.read();
  int16_t y = (Wire.read() << 8) | Wire.read();
  int16_t z = (Wire.read() << 8) | Wire.read();
  gx = x / 65.5f; gy = y / 65.5f; gz = z / 65.5f;   // deg/s bei +/-500
}

// ---- RC-Eingang: einfache Pulsmessung (Polling; fuer Produktion Interrupts nutzen) ----
uint16_t readPulse(uint8_t pin) {
  unsigned long t = pulseIn(pin, HIGH, 25000UL);
  return (t >= 900 && t <= 2100) ? (uint16_t)t : 0;
}

int clampUs(int v) { return v < 1000 ? 1000 : (v > 2000 ? 2000 : v); }

void beep(int ms) { digitalWrite(PIN_BUZZ, HIGH); delay(ms); digitalWrite(PIN_BUZZ, LOW); }

void setup() {
  pinMode(PIN_BUZZ, OUTPUT); pinMode(PIN_LED, OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(PIN_RX[i], INPUT);
  for (int i = 0; i < 4; i++) { esc[i].attach(PIN_ESC[i], 1000, 2000); esc[i].writeMicroseconds(1000); }
  mpuInit();
  beep(80); delay(120); beep(80);   // Boot-Signal, ESCs auf min (Arming)
  delay(2000);
}

void loop() {
  // 1) RC lesen
  for (int i = 0; i < 4; i++) { uint16_t p = readPulse(PIN_RX[i]); if (p) rx_us[i] = p; }
  int thr = rx_us[0], roll = rx_us[1] - 1500, pitch = rx_us[2] - 1500, yaw = rx_us[3] - 1500;

  // 2) Gyro lesen
  float gx, gy, gz; mpuGyro(gx, gy, gz);

  // 3) Rate-Mode P-Regler (nur Demo; echte Firmware: PID + Filter + Fail-safe)
  const float Kp = 0.9f;
  int uRoll  = (int)(roll  * 0.5f - Kp * gx);
  int uPitch = (int)(pitch * 0.5f - Kp * gy);
  int uYaw   = (int)(yaw   * 0.5f - Kp * gz);

  // 4) X-Quad-Mixer (M1 vr, M2 hr, M3 hl, M4 vl)
  int m1 = clampUs(thr - uRoll - uPitch - uYaw);
  int m2 = clampUs(thr - uRoll + uPitch + uYaw);
  int m3 = clampUs(thr + uRoll + uPitch - uYaw);
  int m4 = clampUs(thr + uRoll - uPitch + uYaw);

  // 5) Fail-safe: kein/zu kleines Gas -> Motoren aus
  if (thr < 1050) { m1 = m2 = m3 = m4 = 1000; }
  esc[0].writeMicroseconds(m1); esc[1].writeMicroseconds(m2);
  esc[2].writeMicroseconds(m3); esc[3].writeMicroseconds(m4);

  digitalWrite(PIN_LED, (millis() / 250) & 1);   // Heartbeat
  delay(5);   // ~200 Hz Loop
}
