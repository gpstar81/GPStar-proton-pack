# Wireless Adapter (Optional)

Use of the serial expansion port on the gpstar Proton Pack can bring WiFi capabilities to your unlicensed nuclear reactor! 

- Instructions TBD

## Operation

When using the ESP32 controller it is possible to connect to the device via WiFi. The SSID (Network Name) will be broadcast as **"ProtonPack_####"** where "####" is the last 4 hexadecimal values for the MAC address of the WiFi interface. This will differ for each ESP32 device, making each network unique to the attached pack. The default password is **"555-2368"** and can (and should) be changed via the web interface after successfully connecting to the network. The IP address for the device will be hardcoded as "192.168.1.2" with the web interface accessible at "http://192.168.1.2".
