# Frequently Asked Questions

**Q: What are the power requirements?**

**A:** For most every component 5V is required. It is recommended not to run higher voltage into the VIN or barrel connector jacks on the Arduino Mega or WavTrigger devices as they can generate quite some heat with their onboard regulators for prolonged use. A pre-regulated 5V is recommended. Total draw (amperage) depends on how many optional devices you add, but 5V at 2amps is more than enough to run various pumps, lights, etc. Please view the [POWER.md](POWER.md) guide for additional info and recommendations.

**Q: Is a Neutrona Wand required?**

**A:** No, a wand is not required for the pack-only conversion. However, a modified wand is required if you want full control of the pack from the wand. You can still connect a stock unmodified Hasbro Spengler Neutrona wand, however you will not be able to control the pack with it.

**Q: Running my vibration motors or any other DC motor is giving me noise on my sound output.**

**A:** It is recommended as per the schematics to move all your DC motors onto the right hand +/- rail on the 1/2 breadboard in the pack. You can also add 0.1uf ceramic capacitors across your DC motor terminals, however during testing this was not required as keeping the DC motors on a separate power rail away from the Arduino and Wav Trigger boards was more than enough to avoid any motor noise. More information can be found here if you want to learn more: [Dealing with Motor Noise](https://www.pololu.com/docs/0J15/9)

**Q: Can this electronics setup run in any proton pack shell?**

**A:** In theory yes, with some minor modifications to the code depending on what type of LED setup you are using. This solution was designed primarily with the stock lights and switches in use by the Hasbro products.

**Q: Can I sell and produce kits for others for commerical purposes?**

**A:** Yes, the code is released under a GPLv3 License. Modifications or enhancements which may be useful to the community are encouraged to be contributed back to this original respository.

**Q: Help, I fried my stock equipment or components!**

**A:** OK, not really a question but a common occurrence. Electrical mistakes can happen, and unfortunately there's no warranty or guarantee that this won't happen to you. The best advice is to start small and slow, checking your components work before you connect them to a soldered protoboard by use of a breadboard, first. If everything works or there are mistakes, you can hopefully catch those issues quickly before they become permanently installed. ALWAYS check your input voltages with a multimeter, and verify that you've connected any power sources to the correct device pins BEFORE you apply power. Reversing polarity is a common mistake, so use consistent colors for your positive and negative wires. It is also easy to over-power an LED, so check your resistors using a multimeter and if in doubt or short on parts use the next higher resistance than the recommended value--at the worst your LED's will just be slightly dimmer but won't burn out.
