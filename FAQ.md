# Frequently Asked Questions

**Q: What are the power requirements?**

**A:**I think 5V is required. It is recommended not to run higher voltage into the VIN or barrel connector jack on the Arduino Mega or Wav Trigger, as they can generate quite some heat with there onboard regulators over prolong use. A regulated 5V is recommended. Total draw depends on how many optional devices you add, but 5V 2amps is more than enough to run various pumps, lights, etc. Please view the [POWER.md](POWER.md) guide for additional info and recommendations.

**Q: Is a wand required?**

**A:** No, a wand is not required for the pack-only conversion. However a modified wand is required if you want full control of the pack from the wand. You can still connect a stock unmodified Hasbro Spengler Neutrona wand, however you will not be able to control the pack with it.

**Q: Running my vibration motors or any other DC motor is giving me noise on my sound output.**

**A:** It is recommended as per the schematics to move all your DC motors onto the right hand +/- rail on the 1/2 breadboard in the pack. You can also add 0.1uf ceramic capacitors across your DC motor terminals, however during testing this was not required as keeping the DC motors on a separate power rail away from the Arduino and Wav Trigger boards was more than enough to avoid any motor noise. More information can be found here if you want to learn more: [Dealing with Motor Noise](https://www.pololu.com/docs/0J15/9)

**Q: Can this electronics setup run in any proton pack shell?**

**A:** In theory yes with some minor modifications to the code depending on what type of LED setup you are using.

**Q: Can I sell and produce kits for others for commerical purposes?**

**A:** Yes, the code is released under a GPLv3 License.