#include <Arduino.h>
#include "target.h"

DakeBoard board;
// Arduino defaults to HSPI, but ESP32-C3 only exposes FSPI.
static SPIClass spi(FSPI);
RADIO_CLASS radio = new Module(P_LORA_NSS, P_LORA_DIO_1, P_LORA_RESET, P_LORA_BUSY, spi);
WRAPPER_CLASS radio_driver(radio, board);
ESP32RTCClock rtc_clock;
EnvironmentSensorManager sensors;

bool radio_init() {
  // No external RTC or I2C bus on this receiver.
  rtc_clock.begin();
  if (!radio.std_init(&spi)) return false;
  // ExpressLRS default LR1121 switch bytes, with a checked command result.
  int16_t status = radio.setDioAsRfSwitch(15, 0, 4, 8, 8, 2, 0, 1);
  if (status != RADIOLIB_ERR_NONE) {
    Serial.printf("ERROR: LR1121 RF switch failed: %d\n", status);
    radio.reportDevice();
    return false;
  }
  return true;
}

mesh::LocalIdentity radio_new_identity() {
  RadioNoiseListener rng(radio);
  return mesh::LocalIdentity(&rng);
}
