#include <Arduino.h>
#include "target.h"

DakeBoard board;
#if defined(DAKE_OTA_STOP_BLE) && defined(ENABLE_COMPANION_OTA) && !defined(DISABLE_WIFI_OTA)
#include <helpers/MultiSerialInterface.h>
#include <helpers/esp32/SerialBLEInterface.h>
extern MultiSerialInterface interface_manager;
extern SerialBLEInterface bluetooth_interface;

void dakeStopBluetoothForOTA() {
  if (bluetooth_interface.isConnected()) bluetooth_interface.disable();
  interface_manager.removeInterface(&bluetooth_interface);
  BLEDevice::deinit(true);
  Serial.printf("OTA: Bluetooth released (heap=%u, largest=%u)\n",
    ESP.getFreeHeap(), ESP.getMaxAllocHeap());
}
#endif
// Arduino defaults to HSPI, but ESP32-C3 only exposes FSPI.
static SPIClass spi(FSPI);
RADIO_CLASS radio = new Module(P_LORA_NSS, P_LORA_DIO_1, P_LORA_RESET, P_LORA_BUSY, spi);
WRAPPER_CLASS radio_driver(radio, board);
ESP32RTCClock rtc_clock;
#if ENV_INCLUDE_GPS
  #include <helpers/sensors/MicroNMEALocationProvider.h>
  MicroNMEALocationProvider nmea(Serial1, &rtc_clock);
  EnvironmentSensorManager sensors(nmea);
#else
EnvironmentSensorManager sensors;
#endif

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
