#pragma once

#define RADIOLIB_STATIC_ONLY 1
#include <RadioLib.h>
#include <helpers/radiolib/RadioLibWrappers.h>
#include <helpers/ESP32Board.h>
#include <helpers/radiolib/CustomLR1121Wrapper.h>
#include <helpers/AutoDiscoverRTCClock.h>
#include <helpers/SensorManager.h>

class WaveshareLR1121 : public CustomLR1121 {
public:
  WaveshareLR1121(Module* mod) : CustomLR1121(mod) { }

  // RFO_LP_LF is disconnected (R4 is NC); all sub-GHz TX must use RFO_HP_LF.
  // RFIO_HF is a separate bidirectional port connected directly to J2.
  int16_t setOutputPower(int8_t power) override {
    return LR1121::setOutputPower(power, getFreqMHz() < 1000.0f);
  }

  int16_t checkOutputPower(int8_t power, int8_t* clipped) override {
    return LR1121::checkOutputPower(power, clipped, getFreqMHz() < 1000.0f);
  }
};

class WaveshareLR1121Wrapper : public CustomLR1121Wrapper {
  int8_t tx_power = LORA_TX_POWER;

public:
  WaveshareLR1121Wrapper(WaveshareLR1121& radio, mesh::MainBoard& board)
      : CustomLR1121Wrapper(radio, board) { }

  void setParams(float freq, float bw, uint8_t sf, uint8_t cr) override {
    // Changing bands doesn't select the LR1121 PA, and can leave RX active.
    idle();
    bool legacy_hf_bw_mode = (freq > 1000) &&
        ((fabsf(bw - 203.125f) <= 0.001f) ||
         (fabsf(bw - 406.25f) <= 0.001f) ||
         (fabsf(bw - 812.5f) <= 0.001f));
    radio().setFrequency(freq);
    radio().setSpreadingFactor(sf);
    radio().setBandwidth(bw, legacy_hf_bw_mode);
    radio().setCodingRate(cr);
    updatePreamble(sf);
    setTxPower(tx_power);
  }

  void setTxPower(int8_t dbm) {
    int8_t clipped = dbm;
    auto& r = radio();
    r.checkOutputPower(dbm, &clipped);
    if (r.setOutputPower(clipped) == RADIOLIB_ERR_NONE) tx_power = dbm;
  }

private:
  WaveshareLR1121& radio() { return *static_cast<WaveshareLR1121*>(_radio); }
};

extern ESP32Board board;
extern WRAPPER_CLASS radio_driver;
extern AutoDiscoverRTCClock rtc_clock;
extern SensorManager sensors;

bool radio_init();
mesh::LocalIdentity radio_new_identity();
