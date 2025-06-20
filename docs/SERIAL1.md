<h1><img src="../images/gpstar_logo.png" width="50"/>GPStar Serial1 Expansion Controller</h1>

The "Serial1" expansion port on the GPStar Proton Pack PCB allows you to connect a device which can offer more functionality with your entire Proton Pack and Neutrona Wand system.

At present the only supported uses are as a "Pack Attenuator" or a "Wireless Adapter". Both use-cases are provided by the same software which normally drives the Attenuator device--in the case of the wireless adapter is simply used without switches or other peripherals to only provide WiFi capability to your Proton Pack. Please read the following guides for more specific details. *Please note that the original Attenuator DIY guide used an Arduino Nano which is no longer capable of providing the storage space required by the full range of features supported and that hardware has been deprecated in favor of the ESP32 platform.*

1. [Pack Attenuator](ATTENUATOR.md) - Full Device Implementation (w/ Switches and Inputs)

1. [Wireless Adapter](WIRELESS.md) - Scaled-Down Attenuator (No Physical User Inputs)
