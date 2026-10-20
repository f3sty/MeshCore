#pragma once

#include <helpers/ESP32Board.h>

#ifndef DAKE_RGB_PACKET_LED
#define DAKE_RGB_PACKET_LED 0
#endif
#ifndef DAKE_RGB_LED_BRIGHTNESS
#define DAKE_RGB_LED_BRIGHTNESS 16
#endif

class DakeBoard : public ESP32Board {
  bool ota_active = false;
#if defined(ENABLE_COMPANION_OTA) && !defined(DISABLE_WIFI_OTA)
  bool button_raw = false;
  bool button_pressed = false;
  bool button_handled = false;
  uint32_t button_changed_at = 0;
  uint32_t button_pressed_at = 0;
#endif
#if DAKE_RGB_PACKET_LED
  bool transmitting = false;
  bool receiving_flash = false;
  uint32_t received_at = 0;

  void clearPacketLed() { neopixelWrite(8, 0, 0, 0); }
#endif
public:
  void begin() {
    ESP32Board::begin();
#if defined(ENABLE_COMPANION_OTA) && !defined(DISABLE_WIFI_OTA)
    pinMode(9, INPUT_PULLUP);
#endif
#if DAKE_RGB_PACKET_LED
    clearPacketLed();
#endif
  }

  void onBeforeTransmit() override {
#if DAKE_RGB_PACKET_LED
    if (ota_active) return;
    transmitting = true;
    receiving_flash = false;
    neopixelWrite(8, DAKE_RGB_LED_BRIGHTNESS, 0, 0);
#endif
  }

  void onAfterTransmit() override {
#if DAKE_RGB_PACKET_LED
    if (ota_active) return;
    transmitting = false;
    clearPacketLed();
#endif
  }

  void onPacketReceived() {
#if DAKE_RGB_PACKET_LED
    if (ota_active) return;
    if (transmitting) return;
    received_at = millis();
    receiving_flash = true;
    neopixelWrite(8, 0, DAKE_RGB_LED_BRIGHTNESS, 0);
#endif
  }

  void updatePacketLed() {
#if DAKE_RGB_PACKET_LED
    if (ota_active) {
      neopixelWrite(8, 0, 0, DAKE_RGB_LED_BRIGHTNESS);
      return;
    }
    if (receiving_flash && uint32_t(millis() - received_at) >= 150) {
      receiving_flash = false;
      clearPacketLed();
    }
#endif
  }

#if defined(ENABLE_COMPANION_OTA) && !defined(DISABLE_WIFI_OTA)
  bool startOTAUpdate(const char* id, char reply[]) override {
    if (!ESP32Board::startOTAUpdate(id, reply)) return false;
    ota_active = true;
    updatePacketLed();
    return true;
  }

  void loop() override {
    ESP32Board::loop();
    if (ota_active) return;
    uint32_t now = millis();
    bool pressed = digitalRead(9) == LOW;
    if (pressed != button_raw) {
      button_raw = pressed;
      button_changed_at = now;
    }
    if (pressed != button_pressed && uint32_t(now - button_changed_at) >= 30) {
      button_pressed = pressed;
      button_pressed_at = now;
      button_handled = false;
    }
    if (button_pressed && pressed && !button_handled && uint32_t(now - button_pressed_at) >= 5000) {
      button_handled = true;  // retry only after releasing and holding again
      char reply[128];
      startOTAUpdate(getManufacturerName(), reply);
      Serial.println(reply);
    }
  }
#endif

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
