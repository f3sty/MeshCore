# Waveshare ESP32-S3 LR1121 XF

The module has 4 MB flash. Flash `firmware-merged.bin` at offset `0x0`
when installing the complete PlatformIO image.

## Antenna routing

- **J2:** LR1121 `RFIO_HF`, for 2.4 GHz TX and RX (maximum 13 dBm).
- **J4:** sub-GHz LoRa antenna, through the DIO5/DIO6-controlled RF switch.
- **J1:** ESP32 Wi-Fi/Bluetooth antenna.

The LR1121 sub-GHz low-power output `RFO_LP_LF` is disconnected because R4
is not populated. The board driver forces `RFO_HP_LF` for all sub-GHz TX
power settings. This does not route 2.4 GHz through the sub-GHz switch;
2.4 GHz uses the separate bidirectional HF port on J2.

The RF-switch configuration follows Waveshare's example: standby `00`,
RX `01`, sub-GHz TX `10`, HF TX `00` (DIO6/DIO5).

References:
- [Schematic](https://github.com/waveshareteam/ESP32-S3-LR1121-XF/blob/main/hardware/schematics/ESP32-S3-LR1121-HF.pdf)
- [Manufacturer RF-switch configuration](https://github.com/waveshareteam/ESP32-S3-LR1121-XF/blob/main/examples/arduino/waveshare_lroa_1121/src/lr1121_common/lr1121_common.c)
