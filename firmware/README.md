# ESP32-C6-ePaper-1.54 Firmware

[中文说明](README_ZH.md)

This directory contains factory firmware for the ESP32-C6-ePaper-1.54. Use it to restore the out-of-box demonstration.

## Firmware files

| File | Purpose |
| --- | --- |
| `ESP32-C6-ePaper-1.54_FactoryOnly_20260422.bin` | Factory firmware to restore the out-of-box demonstration. |

The date in the filename identifies the firmware version. The factory BIN is a merged image; flash the entire file at **0x0**.

## Flashing

**Erasing Flash deletes all existing firmware, settings, and user data on the device. Back up any data you need before proceeding.**

1. Install Python and the latest esptool:

   ```sh
   python -m pip install --upgrade esptool
   ```

2. Connect the ESP32-C6-ePaper-1.54 to your computer with a USB data cable. Close any serial monitor using the device's port.
3. Open a terminal in this `firmware/` directory. Replace `<PORT>` with the device's actual serial port, such as `COM5` on Windows or `/dev/ttyACM0` on Linux.
4. Erase the entire Flash, then write the merged firmware:

   ```sh
   python -m esptool --chip esp32c6 --port <PORT> erase-flash
   python -m esptool --chip esp32c6 --port <PORT> write-flash 0x0 ESP32-C6-ePaper-1.54_FactoryOnly_20260422.bin
   ```

5. After flashing completes successfully, reset or power-cycle the board. Confirm that the factory demonstration starts and its functions work as expected.

If the tool cannot connect, put the board into download mode following the product's instructions, then retry.

For command details, see the [official esptool documentation](https://docs.espressif.com/projects/esptool/en/latest/esp32c6/esptool/basic-commands.html).
