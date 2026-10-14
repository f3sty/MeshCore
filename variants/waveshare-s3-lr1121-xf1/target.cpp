#include <Arduino.h>
#include "target.h"

ESP32Board board;

static SPIClass spi;
RADIO_CLASS radio = new Module(P_LORA_NSS, P_LORA_DIO_1, P_LORA_RESET, P_LORA_BUSY, spi);
WRAPPER_CLASS radio_driver(radio, board);

ESP32RTCClock fallback_clock;
AutoDiscoverRTCClock rtc_clock(fallback_clock);
SensorManager sensors;

bool radio_init() {
  fallback_clock.begin();
  rtc_clock.begin(Wire);
  if (!radio.std_init(&spi)) return false;

  // Waveshare's DIO5/DIO6 switch: RX=01, LF TX (LP/HP)=10.
  // HF TX/RX uses RFIO_HF on J2 directly, so HF TX leaves this switch off.
  return radio.setDioAsRfSwitch(0x03, 0x00, 0x01, 0x02, 0x02,
                              0x00, 0x00, 0x00) == RADIOLIB_ERR_NONE;
}

mesh::LocalIdentity radio_new_identity() {
  RadioNoiseListener rng(radio);
  return mesh::LocalIdentity(&rng);
}
