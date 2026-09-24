#pragma once

#define RADIOLIB_STATIC_ONLY 1
#include <RadioLib.h>
#include <helpers/radiolib/RadioLibWrappers.h>
#include <helpers/ESP32Board.h>
#include <helpers/radiolib/CustomLR1121Wrapper.h>
#include <helpers/AutoDiscoverRTCClock.h>
#include <helpers/SensorManager.h>
#ifdef WITH_RS232_BRIDGE
  #include <HardwareSerial.h>
#endif

class WaveshareS3LR1121XF : public CustomLR1121 {
public:
  using CustomLR1121::CustomLR1121;

  int16_t setOutputPower(int8_t power) override {
    // The board does not connect the LR1121 low-power LF output
		// so force low power tx via the high power output
    return LR1120::setOutputPower(power, true);
  }
};

class WaveshareS3LR1121XFBoard : public ESP32Board {
public:
  const char* getManufacturerName() const override {
    return "Waveshare S3 LR1121 XF";
  }
};

extern WaveshareS3LR1121XFBoard board;
extern WRAPPER_CLASS radio_driver;
extern AutoDiscoverRTCClock rtc_clock;
extern SensorManager sensors;
#ifdef WITH_RS232_BRIDGE
  extern HardwareSerial bridge_serial;
#endif

bool radio_init();
mesh::LocalIdentity radio_new_identity();
