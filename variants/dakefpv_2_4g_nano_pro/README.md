# DakeFPV 2.4G Nano Pro

ESP32-C3 / LR1121 target, using the ExpressLRS
[Generic C3 LR1121 layout](https://github.com/ExpressLRS/targets/blob/master/RX/Generic%20C3%20LR1121.json)
and [DakeFPV 1w overlay](https://github.com/ExpressLRS/targets/blob/master/targets.json).


- Default radio settings: 2483 MHz, 406.25 kHz, SF11, CR4/5.
  The board accepts 2400–2500 MHz only.
- TX settings represent **nominal board output dBm**, with a range of 14–30
  and a default/maximum of 30 (1 W). The driver translates output to chip drive
  using the ELRS calibration table. Intermediate settings use interpolation.
- DCDC regulation is enabled after initialization.


Serial uses UART0 on GPIO20 RX / GPIO21 TX, connected through a USB-to-UART
adapter. Connect adapter TX to receiver RX and adapter RX to receiver TX,
with a common ground and 3.3 V logic. 
DO NOT try to power the board from the USB UART's 5v pin, you will likely destroy your UART.
The `companion_radio_usb` environment retains the standard naming but
uses the UART pads for its serial protocol; this receiver has no native USB.



## Optional RGB packet LED

To enable activity LED, uncomment `-D DAKE_RGB_PACKET_LED=1` in the shared
variant build flags. Optional `DAKE_RGB_LED_BRIGHTNESS` sets each active
color's intensity from 0 to 255 (default 16).

- Green flashes for 150 ms after a successful, nonempty radio packet read.
  CRC/read failures do not light it; MeshCore may subsequently reject the
  packet during protocol validation. Repeated receives restart the flash.
- Red stays on during transmission and clears on completion or a start error.
  Transmit takes priority over a receive flash.
- Off when idle. The receive flash uses a timer, with no 150 ms blocking delay,
  and LED updates run outside interrupts.


## Output power settings

| Setting (output dBm) | Nominal output | LR1121 drive dBm |
| --- | --- | --- |
| 14 | 25 mW | -17 |
| 17 | 50 mW | -13 |
| 20 | 100 mW | -9 |
| 24 | 250 mW | -5 |
| 27 | 500 mW | -2 |
| 30 | 1 W | 5 |

For example, use `set tx 30` for 1 W or `set tx 20` for 100 mW.
Companion clients now receive a maximum TX setting of 30. Settings
below 14 are clamped to 14 and saved/reported as 14; values above 30 are
rejected. The radio driver independently limits
chip drive to -17..5 dBm.

## Noise-floor lower bound

`MIN_NOISE_FLOOR_DBM=-127` applies to every DakeFPV environment. Other boards
retain the shared -120 dBm default unless they override this flag.

A lower measured noise floor reduces the RSSI-based busy threshold when
`int.thresh` is nonzero. 
