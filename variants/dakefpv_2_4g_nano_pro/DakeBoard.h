#pragma once

#include <helpers/ESP32Board.h>

#ifndef DAKE_RGB_PACKET_LED
#define DAKE_RGB_PACKET_LED 0
#endif
#ifndef DAKE_RGB_LED_BRIGHTNESS
#define DAKE_RGB_LED_BRIGHTNESS 16
#endif

class DakeBoard : public ESP32Board {
#if DAKE_RGB_PACKET_LED
  bool transmitting = false;
  bool receiving_flash = false;
  uint32_t received_at = 0;

  void clearPacketLed() { neopixelWrite(8, 0, 0, 0); }
#endif
public:
  void begin() {
    ESP32Board::begin();
#if DAKE_RGB_PACKET_LED
    clearPacketLed();
#endif
  }

  void onBeforeTransmit() override {
#if DAKE_RGB_PACKET_LED
    transmitting = true;
    receiving_flash = false;
    neopixelWrite(8, DAKE_RGB_LED_BRIGHTNESS, 0, 0);
#endif
  }

  void onAfterTransmit() override {
#if DAKE_RGB_PACKET_LED
    transmitting = false;
    clearPacketLed();
#endif
  }

  void onPacketReceived() {
#if DAKE_RGB_PACKET_LED
    if (transmitting) return;
    received_at = millis();
    receiving_flash = true;
    neopixelWrite(8, 0, DAKE_RGB_LED_BRIGHTNESS, 0);
#endif
  }

  void updatePacketLed() {
#if DAKE_RGB_PACKET_LED
    if (receiving_flash && uint32_t(millis() - received_at) >= 150) {
      receiving_flash = false;
      clearPacketLed();
    }
#endif
  }

  void sleep(uint32_t secs) override {
#if DAKE_RGB_PACKET_LED
    // Let the receive flash expire before sleeping, so its timeout can run in the main loop.
    updatePacketLed();
    if (receiving_flash) {
      delay(1);
      return;
    }
#endif
    ESP32Board::sleep(secs);
  }

  uint16_t getBattMilliVolts() override { return 0; }
  const char* getManufacturerName() const override { return "DakeFPV 2.4G Nano Pro"; }
};
