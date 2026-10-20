# DakeFPV 2.4G Nano Pro

ESP32-C3 / LR1121 target, using the ExpressLRS
[Generic C3 LR1121 layout](https://github.com/ExpressLRS/targets/blob/master/RX/Generic%20C3%20LR1121.json)
and [DakeFPV 1w overlay](https://github.com/ExpressLRS/targets/blob/master/targets.json).


- Default radio settings: 2483 MHz, 406.25 kHz, SF11, CR4/5.
  The board accepts 2400–2500 MHz only.
- TX settings represent **nominal board output dBm**, with a range of 14–30
  and a default of 20 (100mW). The driver translates output to chip drive
  using the ELRS calibration table. Intermediate settings use interpolation.
- DCDC regulation is enabled after initialization.


Serial uses UART0 on GPIO20 RX / GPIO21 TX, connected through a USB-to-UART
adapter. Connect adapter TX to receiver RX and adapter RX to receiver TX,
with a common ground and 3.3 V logic.
DO NOT try to power the board from the USB UART's 5v pin with high tx power set
or you will likely destroy your UART.
The `companion_radio_usb` environment retains the standard naming but
uses the UART pads for its serial protocol due to no native USB support on this board.

## Initial flashing

These modules ship pre-installed with ExpressLRS firmware. You can use the ELRS OTA Update
function to flash meshcore firmware. Use a **merged** bin.
The process is:
- Power the module
- Hold the user button down for 5 seconds. The RGB LED will change to indicate OTA mode.
- Connect to the **ExpressLRS RX** wifi network, default password is **expresslrs**.
- Point a broswer to http://10.0.0.1/ 
- Find the **Update** tab, select your firmware-merged.bin file, and click 'Update'.
You may get an error from the OTA flasher, but it should still succeed. 
 

## Companion firmware updates over Wi-Fi

The BLE companion retains a 500-contact table and uses a 32-frame offline queue
to leave more runtime heap for the ESP32-C3 Bluetooth stack and on-demand Wi-Fi
updater.

The BLE, UART/`companion_radio_usb`, and Wi-Fi companion builds enable
`ENABLE_COMPANION_OTA`. Start the updater using either:

- The companion CLI command `start ota`, including through a connected BLE client.
- Holding the user button (GPIO9, active low) for five seconds **after startup**.
  GPIO9 is a boot-strapping pin: holding it during reset will not start OTA.

The RGB LED turns blue when the updater starts, if `DAKE_RGB_PACKET_LED` is enabled.
Connect to the open **MeshCore-OTA** Wi-Fi network and open
**http://192.168.4.1/update** (the CLI also returns the updater URL).
Select a firmware update and upload `.pio/build/<environment>/firmware.bin`
for the Dake target. Do not upload the merged serial-flashing image or select a
filesystem update. The device reboots after the upload; power-cycling also exits
OTA mode. Repeated activation reuses the running updater.

Wi-Fi is started only when OTA is requested on BLE/UART builds. The Wi-Fi
companion can also keep its existing station connection while the OTA access
point runs. BLE remains enabled on the BLE companion during OTA.


### Opting in on other targets

Other ESP32 companion environments can add `-D ENABLE_COMPANION_OTA=1`, include
`${esp32_ota.lib_deps}` in their `lib_deps`, and use a partition layout with OTA
metadata and two application slots large enough for the resulting firmware.
This exposes `start ota`; the GPIO9 gesture and blue LED are Dake-specific.

`ENABLE_COMPANION_OTA` enables the ota CLI command, which calls
the board's `startOTAUpdate()` implementation. It does not add a new OTA transport
on other platforms. `DISABLE_WIFI_OTA` disables only the ESP32 Wi-Fi updater and
the Dake button action; if both flags are defined on ESP32, `start ota` reports
`Error: OTA unavailable`. 


## Optional RGB packet LED

To disable activity LED, comment out `-D DAKE_RGB_PACKET_LED=1` in the shared
variant build flags. Optional `DAKE_RGB_LED_BRIGHTNESS` sets each active
color's intensity from 0 to 255 (default 16).

- Blue when OTA update is active.
- Green flashes for 150 ms after a successful, nonempty radio packet read.
  CRC/read failures do not light it; MeshCore may subsequently reject the
  packet during protocol validation. Repeated receives restart the flash.
- Red stays on during transmission and clears on completion or a start error.
  Transmit takes priority over a receive flash.
- Off when idle. The receive flash uses a timer, with no 150 ms blocking delay,
  and LED updates run outside interrupts.
- Enabling the RGB LED may prevent powersaving sleep being entered for up to 150ms
  after receiving a packet. For maximum power saving, disable the RGB LED.


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

`MIN_NOISE_FLOOR_DBM=-127` applies to every DakeFPV environment to match the LR1121's capability.
Other boards retain the -120 dBm default unless they override this flag.

A lower measured noise floor reduces the RSSI-based busy threshold when
`int.thresh` is nonzero.

