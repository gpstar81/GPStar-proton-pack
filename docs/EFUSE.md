# eFuse Burning

&#128721; **STOP!**

This is an advanced guide for DIY users or for special purposes only. If you were not explicitly directed to perform these actions on your hardware, then DO NOT attempt these actions under any circumstances!

## Overview

Most ESP32 chips are shipped with a generic but usable configuration which reserves certain pins on the chip for specific uses. In the case of this project we must alter a default behavior and disable a debugging feature in order to make some reserved pins available for use as General Purpose Input-Output (GPIO) pins.

Specifically, we need to make pins 39-44 on the `ESP32-S3-WROOM` chips accessible as GPIO rather than use their shipped defaults. To accomplish this it is necessary to "burn" eFuses on the hardware. These are 1-time settable, PERMANENT changes to the hardware which affect its operation. **In other words, you can only do this once and after an eFuse is "burned" it cannot be reverted--your hardware will be irrevocably changed.**

(See now why this page starts with a disclaimer?)

## Burning

Fuses are burned using the aptly-named `espefuse.py` script (or the compiled binary version, in the case of Windows x64). For Linux and MacOS use please see the `scripts/burn_efuses_esp32s3.sh` script for usage instructions which will handle the necessary actions automatically.

## eFuses

The two fuses we need to burn along with their hex values are:

| eFuse Name = Value | Description |
|----|----|
| `UART_PRINT_CONTROL = 0b3` | Setting `UART_PRINT_CONTROL` to 3 disables printing debug messages to `UART0` on boot. This is required in order to use `GPIO43` (TX0) and `GPIO44` (RX0) by not writing console data via these pins. |
| `DIS_PAD_JTAG = 0b1` | Setting `DIS_PAD_JTAG` to 1 disables the JTAG engine at boot from using the physical `JTAG` pins and routes all JTAG functionality through the `USB-CDC` engine instead (via DFU mode), freeing `GPIO 39~42`. |

It is important to keep in mind when it comes to setting fuses is that you want to do as few writes as possible during that procedure. Thus `espefuse.py` includes a "batching" feature where you can pass multiple eFuses in a single command and the program will then do the write in a single pass rather than one write per fuse. This is handled automatically by the `scripts/burn_efuses_esp32s3.sh` script which will first read the current state, make changes if necessary, and report the new state of the eFuses.