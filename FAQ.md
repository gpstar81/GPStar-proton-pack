# Frequently Asked Questions

**Q: Can I send you my Proton Pack and Neutrona Wand so you can modify it for me?**

**A:** Sorry, no.

**Q: Can I help others install kit components or add-ons?**

**A:** Yes, if you are willing to offer your skills whether free or at-cost to assist others that is acceptable.

**Buyers:** Please be aware that anyone performing installation work on behalf of this project is doing so as a free agent and is not associated with gpstar Technologies. This project does not warranty work performed by a 3rd party, only parts produced and shipped via agents of gpstar Technologies or Frutto Technology are warrantied in the case of manufacturing defects.

**Q: Does this replace everything in the pack/wand?**

**A:** This solution focuses on replacement of the microcontrollers which run the pack and wand only. It is not necessary to replace all of the switches and lights unless you truly desire to do so. Although in the case of the wand the toggle switches and every other component are attached with very thin wires, so it may be worth considering temporarily removing some items to solder better (thicker) wires to avoid breakage later.

**Q: What are the power requirements?**

**A:** For most every component 5V is required. It is recommended not to run higher voltage into the VIN or barrel connector jacks on the Arduino Mega or WAV Trigger devices as they can generate quite some heat with their onboard regulators for prolonged use. A pre-regulated 5V is recommended. Total draw (amperage) depends on how many optional devices you add, but 5V at 2amps is more than enough to run various pumps, lights, etc. Please view the [POWER.md](POWER.md) guide for additional info and recommendations.

**Q: Is a Neutrona Wand required?**

**A:** No, a wand is not required for the pack-only conversion. However, a modified wand is required if you want full control of the pack from the wand. You can still connect a stock unmodified Hasbro Spengler Neutrona Wand, however you will not be able to control the pack with it. `** ATTENTION: Please keep in mind that this system is supplying 5V, while the stock Neutrona Wand from Hasbro is designed around 4.5V, so while a stock Hasbro Neutrona Wand can be connected, you do so at your own risk. **`

**Q: How many wires / pins are required for the connection between the Proton Pack and Neutrona Wand?**

**A:** 4 wires / pins are required (Positive, Negative, TX and RX). See the [Hose Examples](HOSE.md) page for more information.

**Q: Running my vibration motors or any other DC motor is giving me noise on my sound output.**

**A:** It is recommended as per the schematics to move all your DC motors onto the right hand +/- rail on the 1/2 breadboard in the pack. You can also add 0.1uf ceramic capacitors across your DC motor terminals, however during testing this was not required as keeping the DC motors on a separate power rail away from the Arduino and WAV Trigger boards was more than enough to avoid any motor noise. More information can be found here if you want to learn more: [Dealing with Motor Noise](https://www.pololu.com/docs/0J15/9)

**Q: Can this electronics setup run in any Proton Pack shell?**

**A:** In theory yes, with some minor modifications to the code depending on what type of LED setup you are using. This solution was designed primarily with the stock lights and switches in use by the Hasbro products.

**Q: Can I produce and sell kits for others for commerical purposes?**

**A:** Yes, the code is released under a GPLv3 License. Modifications or enhancements which may be useful to the community are encouraged to be contributed back to [the original respository](https://github.com/gpstar81/haslab-proton-pack).

**Q: I have some cool ideas, can you add some custom bits in the code for me?**

**A:** Feel free to use the [discussion section of the GitHub repository](https://github.com/gpstar81/haslab-proton-pack/discussions) to talk about them. Anybody can contribute modifications or enhancements back to the repository (by use of a branch or fork) for review and final acceptance into the main branch.

**Q: Help, I fried my stock equipment or components!**

**A:** OK, not really a question but a common occurrence. Electrical mistakes can happen, and unfortunately there's no warranty or guarantee that this won't happen to you. The best advice is to start small and slow, checking your components work before you connect them to a soldered protoboard by use of a breadboard, first. If everything works or there are mistakes, you can hopefully catch those issues quickly before they become permanently installed. ALWAYS check your input voltages with a multimeter, and verify that you've connected any power sources to the correct device pins BEFORE you apply power. Reversing polarity is a common mistake, so use consistent colors for your positive and negative wires. It is also easy to over-power an LED, so check your resistors using a multimeter and if in doubt or short on parts use the next higher resistance than the recommended value--at the worst your LEDs will just be slightly dimmer but won't burn out. Much of this advice comes from hard-learned personal experiences--we've all been there, done that.
