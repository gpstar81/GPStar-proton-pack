# Smoke Effects

Both the [gpstar Proton Pack PCB](PACK_PCB.md) and [DIY Arduino](PACK.md) approaches can support up to 2 separate smoke options for the N-Filter and Booster Tube to provide atmospheric effects while operating your Ghostbusters equipment. These effects are automatically triggered during certain interactions and typically after pre-set times through use of 4 JST-XH connections configured to provide 5 volts DC during smoke effect phases:

- Smoke1 aka. N-Filter Smoke `SM-1 - gpstar Proton Pack PCB` or `Pin 39 - Arudino Mega`
- Smoke2 aka. Booster Smoke `SM-2 - gpstar Proton Pack PCB` or `Pin 35 - Arduino Mega`
- Fan1 aka. N-Filter Fan `FN-1 - gpstar Proton Pack PCB` or `Pin 33 - Arduino Mega`
- Fan2 aka. Booster Fan `FN-2 - gpstar Proton Pack PCB` or `Pin 50 - Arduino Mega`
- LED-W aka. N-Filter Light `LED-W - gpstar Proton Pack PCB` or `Pin 46 - Arduino Mega`

**Note: Fan1 is set to run during overheat sequence only. It is useful for pushing smoke through the N-Filter**

**Note: Fan2 is set to run at the same time as Smoke2. This was requested if you want to instead use Smoke2 from the booster tube for another purpose. For the purpose of smoke in the booster tube, Fan2 is not required.**

## What is a "Smoke Kit"?

Before proceeding it should be made clear what exactly a "smoke kit" consists of. In simplest term this is a combination of 4 components: a vape coil (atomizer), an air pump, a fan or blower, and a white LED. Traditionally, the vape coil will be paired directly with the air pump as it is dangerous to run a vape coil without proper airflow. The fan and LED may either be separated or combined--there are many community makers of smoke kits with various implementations where components may be bundled differently.

For modders who are comfortable with building their own smoke solution we recommend [this guide to creating your own smoke generator](SMOKE_DIY.md). For purposes of integrating and connecting a smoke solution please read the remainder of this guide.

### Common Materials

If not provided with a kit purchased within the community, the following may be necessary to connect your smoke generator to the Proton Pack.

- [5V Relay Module with Optocoupler](https://a.co/d/h0b5SN5) - Must support a HIGH signal to turn the relay on!
- [6mm OD / 4mm ID Silicone Tubing](https://a.co/d/7qeekAG) - For routing smoke from the vape coil

### Disclaimers

This project does not make any direct endorsements of kits available in the Ghostbusters community. As stated above most kits consist of common components which can be integrated in a few standard ways. All references to kits or makers is for illustrative purposes only as this team has directly confirmed operation with the kit makers. If you have a smoke kit and wish to validate integration, please reach out to the team via the [gpstar Technologies Facebook group](https://www.facebook.com/groups/gpstartechnologies) for assistance.

Most smoke kits utilize a combination of vegetable glycerine (VG) and propylene glycol (PG). Please follow any kit maker's instructions for filling or replacing your smoke fluid. Some ratios work better for the type of smoke desired and may be a personal preference. The most common are either 100% VG or 

### Warnings

Modification of your pack is unavoidable for proper installation as the areas where smoke must be delivered is isolated from the inside of the pack. Please proceed only if you are comfortable with **irreversible modifications**. One such modification is the full removal of the D-Cell battery compartment which allows access through the battery door to the vape coils for easy refilling.

In both the PCB and DIY approaches each of the smoke and fan pins is designed to make use of a transistor to run connected devices, as most will require more than the recommended 40mA maximum of current through the ATMega controller.

- All connectors provide 5V of power and are intended to trigger or power devices such as pumps and fans.
- For devices which require a dedicated or non-5V power source **ALWAYS** use a relay module.
- The Proton Pack PCB has been tested using up to 2 Amps of current but is **NOT** recommended as some components may be pushed to their thermal limits and could potentially fail prematurely under extended use.
- **Do not connect any device which draws more than 500mA of current.** Most fans/blowers and relays can be safely attached to the Fan1 or Fan2 ports without concern as these require 500mA or less to operate.

### Powering a Kit

Power should be provided as recommended by either your kit manufacturer. Most kits run on either 5V or 12V sources so this will determine the best way to implement a power connection. Though in most cases kits operate on the principle of "apply power, get smoke" so the only concern is how to turn the 5V (HIGH) signal from the PCB or Arduino into something which can safely run a vape coil and air pump combination. **Enter: The 5V Relay Module.**

For a 5V relay, this device will electrically isolate the power from the PCB or Arduino from the power source running the smoke generator. This is the recommended solution for safely turning on your smoke generator. Remember that most vape coils will draw 1.5 to 2 Amps so your power supply must be able to provide that safely without triggering a safety shutoff.

![](images/RelayModule.jpg)

Most relays work using 3 wires: 2 for power (+/-) to the relay itself, and 1 for the signal to turn the relay on or off. One solution is to use a relay which can accept a HIGH signal to turn on and use the + connection to drive both the relay power and input signal. This arrangement typically uses less than 100mA and can safely connect to the Smoke1 or Smoke2 connectors.

![](images/RelayTrigger.jpg)

Typically the vape coil and air pump will have their positive leads connected together, and the negative leads connected together. The positive from the power source will be connected to the NO (normally-open) terminal on the relay module while the combined positive leads are connected to the COM (common) terminal. The negative leads should connect directly to the power source for the smoke generator.

![](images/RelayPower.jpg)

## Connection Examples

For a more in-depth look at the electrical connections, the following diagrams demonstrate what is required for integrating several smoke kits available from the community. These have been either directly verified or confirmed as "expected" by their respective makers. Other kits or DIY solutions may be used, provided they adhere to the use of a relay for applying power directly to a vape coil device.

![Built-in Battery](images/Smoke-NoBatt.png)

![DIY - Single Battery](images/Smoke-DIY-1.png)

![DIY - Dual Battery](images/Smoke-DIY-2.png)

![5V - Single Battery](images/Smoke-5V-1.png)

![5V - Dual Battery](images/Smoke-5V-2.png)

![12V - Single Battery](images/Smoke-12V-1.png)

![12V - Dual Battery](images/Smoke-12V-2.png)

## Booster Tube Installation

For this component a fan is not necessary as the smoke will rise through use of only the air pump device.

![Booster Tube](images/SmokeBooster1.jpg)

![Booster Tube 2](images/SmokeBooster2.jpg)

![Booster Setup](images/SmokeBooster3.jpg)

![](images/BoosterSmoke.gif)

## N-Filter Installation

The following 3D models are provided for those able to print components for their pack. These are designed to fit perfectly within the Hasbro equipment and provide a familiar look to the end solution for getting smoke out of your pack.

- [N-Filter Blower Mount](stl/pack/N-Filter Smoke/N-Filter Blower.stl) - Holds the 50mm blower in an upright position
- [N-Filter Blower Mount](stl/pack/N-Filter Smoke/N-Filter Cone.stl) - Replaces the stock cone and allows smoke and air to exit
- [N-Filter Blower Mount](stl/pack/N-Filter Smoke/N-Filter Template.stl) - Used to mark where to cut into the pack for smoke tubing and blower air hole

![](images/NFilterTemplate.jpg)

The template print will help you to identify the correct placement for holes which will allow the silicone tubing and LED power wires to pass through, as well as the port opening for the blower fan. Use with the word TEMPLATE facing up for the proper orientation.

![](images/NFilterFan.jpg)

![](images/NFilterMount.jpg)

Once the holes are drilled you can simply drop the blower mount into the socket. There will be space to access the screw which holds the cone in place, as well as routing the silicone tubing and LED power wires. The blower will be held in place by the other half of the inner cyclotron housing, so it will simply slot into place prior to closing up and reinstalling the cyclotron assembly.

![](images/NFilterCone1.jpg)

![](images/NFilterCone2.jpg)

The LED and wires should be installed first, pushing the LED to the very tip of the cone. Once in place, the silicone tubing can be inserted into the same hole to keep the wires (and LED) in place. Adding some hot glue will keep these in place and prevent smoke from escaping out this hole in the cone. Once those are installed the tubing and wires can be gently pulled from the backside of the N-filter as the cone is pushed into place and secured using the original screw.

![](images/NFilterSmoke.gif)

## Testing

Congratulations, you are now ready to test your smoke solution! While in the Video Game mode, use the Barrel Wing Button to select the Venting menu option (after all firing mode options). Put the Neutrona Wand into firing state and press the Intensify button to activate the N-Filter smoke option directly. Alternatively, regardless of mode set your wand to level 5 and throw a proton stream (or activate Cross the Streams) and wait for the smoke effects to kick in.

Remember to activate all smoke options for the pack via the EEPROM menu if you previously changed these.