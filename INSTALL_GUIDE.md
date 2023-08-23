## Installation Guide Video

[![gpstar Proton Pack & Neutrona Wand Installation Guide Video](https://img.youtube.com/vi/zkFpTLQZsCI/maxresdefault.jpg)](https://www.youtube.com/watch?v=zkFpTLQZsCI)

## Installation Guide (Coming soon)


### Warning! Must read before proceeding

- There are minor modifications to the pack and medium-level of modifications to the
    wand.
- Do not use the DC jack on the Wav Trigger! We configured the boards to work with the
    5v pin ONLY! Plugging into the DC Jack could cause it to short circuit.
- 3D printed parts are printed in PLA so Do not leave your Proton pack inside a car when
    it’s hot outside or they can melt.
**- Proceed at your own risk!**

# Table of Contents

**Required Components (not included in any Kits) .................................................................... 3
Tools Used .................................................................................................................................... 3
Contents of the Kits ..................................................................................................................... 4
Pack Preparations ....................................................................................................................... 7
Pack Installation ........................................................................................................................ 15
Wand Preparations .................................................................................................................... 28
Wand Installation ....................................................................................................................... 36
Installing the Hose ..................................................................................................................... 56
Add-on: Inner Cyclotron Cake & Panel LEDs Kit .................................................................... 61
Post-installation Guides ............................................................................................................ 73**
Updating Firmware ................................................................................................................ 73
Custom configurations via PC ............................................................................................... 80
Loading music onto SD card .................................................................................................. 80


## Required Components (not included in any Kits)

We built our kit around specific components but other similar items would work too. If you are
new to all this it would be easier to go with our recommendations. For the DIY savvy people it’s
up to you to use whatever works and set it up however you want.

- Speakers & Amp combo or portable speaker w/ Aux output and required wiring/cables
	- We have built our kit around 3” 15w speakers and a 15w amp.
	- Something similar to our recommended speakers/amp is required to use our Speaker Support Kit.
- 2x MicroSD Cards (At least 2GB for all the sound effects)
	- It is suggested to purchase a quality name brand cards as the cheaper ones are prone to have issues that require reformatting.
- External Battery (5v is requried for the boards and 12v is required for the amp.
	- D-Cell batteries WILL NOT WORK.
	- We STRONGLY recommend the 3000mAh Talentcell battery w/ both 5v & 12v
output. *6000mAh works too but we built the kit around the 3000mAh version
because it is a better fit.*

## Tools Used

**Required**

- Drill
- Phillips-head screwdriver
- Flat-head screwdriver (small bit size needed)
- Allen key
- a small self tapping screw to remove the wand handle plugs
- Electrical Tape

**Helpful**

- Pliers
- Pointy tweezers
- Soldering iron
- Masking tape
- Zip ties


## Contents of the Kits

### Essential & Bare Basics Kit

Pack board Wand board Rotary Encoder
Pack Wav Trigger
(pre-soldered) **(Essential Kit
Only)**
Wand Wav Trigger
(pre-soldered, DC/Aux
barrels removed) **(Essential
Kit Only)**
2x Wav Triggers (unsoldered,
unmodified barrels)
**(Bare Basic Kit Only)
Package A
A1)** 6-pin double-end wired
JST-XH to JST-PH connector
**A2)** 3-pin double-end wired
JST-XH connector
**A3)** USB-A to JST-XH power
cable
**A4)** Programming USB cable **A5)** 2x toggle switches **A6)** Speaker terminal block
**(Bare Basics Kit Only)
A7)** 6-pin JST-XH Header
**(Bare Basics Kit Only)
A8)** 4-pin wired JST-PH
connector
**(Bare Basics Kit Only)
A9)** 3x Velcro


**3D Printed Parts**
Wand Cage 2x Wand connectors & set
screw **(Essential Kit Only)**
Powercell Panel
**Package B (Essential Kit Only)
B1)** Cnlinko Socket to 2x 2-pin
wired JST-XH connectors
**B2)** Cnlinko Connector to
4x wires
5’ Loom Hose
**Package C (Essential Kit Only)
C1)** Neutrik Connector
(Unassembled)
**C2)** Neutrik Socket **C3)** 2x 2-pin wired JST-XH
connector (short)
**Package D (Bare Basics Kit Only)
D1)** 2x 2-pin wired JST-XH
connector (short)
**D2)** 2x 2-pin wired JST-XH
connector (long)
**D3)** 4x exposed ended wires


### Speaker Support Kit

```
K1) 4x 6.3mm terminal
speaker wires
K2) 2x 2.8mm terminal
speaker wires
K3) Auxiliary cable
K4) 12v DC cable K5) Top speaker 3D printed
mount & bracket
K6) Bottom speaker 3D
printed mount & bracket
```
### Cyclotron Cake & Panel LED’s Kit

```
L1) Yellow & Green LED’s L2) 2x Green LED’s L3) 2x Yellow LED’s
L4) 2x red LED’s L5) 35 LED Ring L6) LED Ring Extension
cable
L7) 10 Cliplites L8) 3D Printed Panel L9) Plastic Guitar Pick
```

## Pack Preparations

During the installation process we will be jumping between the Essential Kit, Bare Basics Kit,
and Speaker Support Kit. Headers will be provided to let you know which ones the process
pertains.

First we need to upload the audio from the computer. **-Essential/Bare Basics Kit**

Please follow the Audio link below:

[https://github.com/gpstar81/haslab-proton-pack/blob/main/AUDIO.md
](https://github.com/gpstar81/haslab-proton-pack/blob/main/AUDIO.md)

**Prepping the Wav Triggers** **- Bare Basics Kit ONLY**
Have the orientation of the Wav Triggers with the aux port facing up_
1) Make sure the load/run switch on the top left is set to
load for both Wav Triggers.
2) On both Wav Triggers, drop a dab of solder joining 2 tiny
pads right next to where it says 5v[ ]. This will enable the 5v pin.
**DO NOT use the barrel jack anymore at this point as it could
fry the board.**
3) On Pack Wav Trigger, solder the 6-pin
JST-XH header onto the 6-pin holes on the
bottom right. **Make sure the front side is
facing OUTWARD from the Wav Trigger.**
4) On Wand Wav Trigger, pry off the aux
port and DC barrel jack using pliers and
tweezers. Lift up the thin metal connection
points and rock the connectors back and forth
several times until they snap off. They need to
be removed due to limited space in the wand
body.
5) On Wand Wav Trigger, solder the 4-pin
JST-XH wire to the 6-pin holes located on the
bottom right


- Black wire to GND
- Red wire to 5Vin
- Blue wire to RX
- Yellow wire to TX
Also solder the 2-pin screw terminal block on to SPKR - +
6) Double check your header orientation and wire placements. Make sure both SD cards
are installed.
**Add velcro to the back of the Pack Wav Trigger, Amp & Battery** _- Essential/Bare Basics Kit_
**Opening the Pack** _- Essential/Bare Basics Kit_


1) Open the cyclotron area by
unscrewing the shock
mount/bumper/cyclotron lid and
remove the cyclotron cake.
2) Using a Phillips-head screwdriver remove 17
screws from the motherboard. (15 long screws, 2
short screws). Remember where the 2 short
screws go.


3) Lift the motherboard up gently and unplug the yellow connector from the pack board and
place the motherboard aside.
**IMPORTANT TIP (do not unplug by pulling on the wires, be sure to pull from the
connector housing using your fingers or pliers)**


**Removing stock components** _- Essential/Bare Basics Kit
Save the screws! We will be reusing some of these_
1) Unplug the rest of the wire connectors from the board and put them aside. **Again be
careful to pull from the housing ONLY**
2) Remove 2 short screws holding the main board
3) Remove 2 long screws from the bottom speaker mount (you might need a shorter
screwdriver to fit in the space below it)


4) Remove 2 long screws holding the volume rotary board
5) Remove 2 long screws from sides of the speaker bracket
6) Remove 2 long screws holding down the powercell compartment panel and remove
panel


7) With the inside of motherboard facing up, remove 2 short screws holding the strap cover
on the right side
8) Using an allen key, remove the 4 screws holding the hose connector. This is connected
to the purple JST-XH connector that goes to the main board. You can remove this entire
thing we won’t be using it.


## Pack Installation

The pack boards, speakers/amp and battery all work in conjunction with each other and are
difficult to break apart into individual installation segments. So we will be jumping back and forth
between the Kits.
For those of you who have your own speaker/amp and battery setup you can just skip the steps
that do not pertain to you.
**Installing the Boards** _- Essential/Bare Basics Kit
This is where we will be using all the existing screws that we saved._
1) Secure the new pack board in the existing slot with existing 2 short screws
2) Secure the rotary encoder in the existing slot with existing 2 long screws
3) Connect A2 to the rotary encoder and pack board
_Speaker Support Kit Only_
4) Install bottom speaker mount
5) Install top speaker mount bracket and
secure using existing 2 long screws


6) Insert top speaker and secure with 1 screw (long) in the top/middle hole. Ensure the
terminals are **pointing out to the left**.
7) Using 2 short screws secure the bottom speaker to the bottom speaker bracket at
adjacent corners test the install on the bottom speaker mount. Ensure the bottom
terminals are pointing out to the left. Take off and set aside for now.


_Essential/Bare Basics Kit_
8) Break off this screw hole in the powercell panel area by rocking back and forth with
some pliers as well as the tiny nubs behind it. Don’t worry, this isn’t needed anymore.
9) Peel off the back of the velcro stickers and tape the battery down into the slot. Test the
velcro by removing the battery and ensuring everything is stuck down.


10) Take A5 and remove the nuts then insert the toggle switches into the holes of the 3D
printed Powercell Panel. Make sure they line up vertically.
11) Screw in the nuts on the front and tighten with pliers. Test the toggles making sure they
flip on/off vertically.


12) Prep a long screw in the panel screw hole, install the panel, and secure it.
13) Install the Pack Wav Trigger in the top right of the pack. Ensure that the bottom edge of
the Wav Trigger reaches all the way to the bottom.


14) Reconnect some of the existing Haslab connectors. Refer to this diagram if it helps.
There should be 5 connected at this point: 2-pin white Ion arm, 4-pin red cyclotron, 2-pin
red alarm switch, 4-pin blue panel toggle switches, 4-pin white powercell. Double check
your connections.


15) Connect the bigger end of A1 to the Wav Trigger. To ensure correct orientation the black
wire should be connected to the top of the 6-pin header with the pin labeled “GND”.
Connect the other end to the grey 6-pin on the board labeled “Wav Trigger”.
16) Then connect the 2 toggle switches, A5 from the powercell panel to the blue 2-pin
labeled “Cyclotron Direction” on the top-left of the board and the purple 2-pin labeled
“Smoke Toggle” on the bottom-right.


17) Thread A3 through the middle hole in the powercell panel and connect the usb to the
battery.
18) Connect the other end to the yellow 2-pin on the board labeled “Battery”


_Speaker Support Kit Only_
19) Thread K4 through the other hole and connect the DC jack to the battery. Insert the other
end to the middle terminal blocks of the amp and turn the screws above them with a
flathead screwdriver. Red wire to positive (+), black wire to negative (-).
**IMPORTANT TIP: Terminal blocks usually come already clamped down. Ensure
they are open before inserting wires by turning the screws to the left.**
20) Connect K1 and K2 wires to the terminal ends of both speakers and the other ends to
the amp. Ensure one speaker goes to L-/L+ and the other goes to R+/R-. Be mindful that
the wide terminal end is positive (+) and the narrow terminal end is negative (-)[
21) Place the majority of the wires along the top of the bottom speaker mount and the
cyclotron lid and 12v dc wires along the bottom and then place the bottom speaker.


22) This is up to you but leaving the knob at 50% is a good level. Take K3, aux cable and
connect to the Wav Trigger and Amp. Run the wire along with the rest above the bottom
speaker.
23) Stick the amp inside the left side of the pack next to the vibration motor.


_Essential/Bare Basics Kit_
24) **Optional:** Take A4 (the programming cable for software updates to the Pack & Wand
boards) and connect it to the 6 exposed pins on the bottom left of the Pack board. Match
the black wire to the left pin labeled “GND”. Stick the USB end through the rectangle
hole in the powercell panel. You can now pull it out and connect it to the PC whenever
you need to without removing the motherboard.
_Essential Kit ONLY_
25) Take B1 and run the wires through the pack hose hole and where it says “wand” on the
board connect the red/black wires to the yellow 2-pin labeled “5v out/ + -” and the
blue/yellow wires to the purple 2-pin labeled “RX2 TX2”.
26) Using the stock Haslab hex screws connect the CNLinko end to the pack. Orient it
however you want but the one that makes most sense is having the label facing to the
top.You will notice that the holes do not line up at all. To get around this you will need to
screw them in at an angle and only a few turns at a time. Be patient and go slowly
alternating the turns for each screw. You can back some off if you need to. Eventually
they’ll all fit. I believe it’s magic, magic... Cut off the rubber tab cover.


_Bare Basics Kit ONLY_ Take D2 and refer to step 27 above. The only difference is the
other end is open tinned wires free for you to do as you like.
_Essential/Bare Basics Kit_
27) Now that the pack is fully connected we will do a test run. First **DOUBLE CHECK YOUR
CONNECTIONS**. Is the Wav Trigger header facing the right way? Did you screw in the
speaker wires into the correct terminal blocks? Are the wires connected to the right
connectors on the board?
28) When you’re done that... **TRIPLE CHECK YOUR CONNECTIONS!**
29) On the motherboard there are 2 tabs on the inside-top. Snap these off. Then place the
motherboard back on but don’t screw anything down yet. We are just holding the bottom
speaker in place and ensuring everything fits okay.


30) Flip it over and place the cyclotron lid back on.
31) Turn on the battery....
32) Say a prayer....
_33) “Switch me on”_
If it turns on with lights and sound... **CONGRATS** !! You have successfully completed the
pack install portion of the kit! We are halfway there!
Now test out all the switches, knobs and modes to ensure it’s all working. Take a break
and have some fun with it!


## Wand Preparations

_Essential/Bare Basics Kit_
1) **NOTE: The V-Hook can remain on the gun rail.** Remove 4 covered plugs on the
bottom of the gun rail by making a hole with a small drill bit, small scissors, or whatever
tools you have laying around. Then use a self-tapping screw and threading into them
and pull the plugs out. Alternatively a swiss army knife reamer can do the job on its own.
How well they are glued in varies from wand to wand.
2) Remove the 4 screws underneath with a phillips-head screwdriver and take off the gun
rail.


3) Remove an additional 4 screws on the bottom of the gun body. Note the one near the
barrel is a longer screw than the rest.
4) Open the body halves being mindful of the heatsink where the speaker is attached. Try
not to let this part fall out or dangle via the wires as the **wires attached to the speaker
are delicate**. It helps to tape it down to be safe.


5) Mark the speakers with a negative (-) and positive (+) as shown below:
6) Remove the tape and break off the glue holding down the wires. Be careful not to cut the
wires.


7) Remove the 4 screws on the stock board.
8) There are 3 connectors that can be unplugged. They are quite tight. It helps to use
tweezers to pry open a little where the little tabs are and then pull on the connector with
some pliers.


```
9) Removing the soldered connections via 2 options:
Option #1: Snip, strip & twist
```
- Cut the wires off as close to the board as you can. Use wire strippers to strip the
    wire ends (26 AWG, stranded wires). You can CAREFULLY use scissors to cut
    around it and pull but you run the risk of cutting it off by accident and then would
    have to go down the wire to try again, risking shortening the length of the wire.
**Option #2: Desoldering** _*Ideal method_
- Use a soldering iron and apply to the back side of the connected wires. As it melts the
solder, gently pull the wire out on the other end.
- **This method is PREFERRED to everyone** , even those who don’t solder. I encourage
everyone to try this method instead. You don’t need to know how to solder or have any
other soldering equipment. It’s quite easy and leaves you with a tinned end and doesn't
run the risk of cutting the wires too short or the stranded wires fraying.


10) Carefully take the speaker out and put a piece of tape around the wire that goes to the
positive (+) end. Reinsert it and ensure the wires go back in under and up the body.
11) Remove the lever latch for now by unscrewing these two screws. We will reattach later
on.


12) Take the 4-pin red/white connector and slide it through the narrow gap between the lever
and the inside body. This will take a bit of finessing. It’s best to allow some slack to the
bottom part of the wire and use tweezers to hold it down. Make it as flat as possible
against the wall and shimmy it through. The last bit will require some force in pulling.
Take your time.
13) With the connector out of the way we can safely drill a hole into the inside of the wand
handle to feed some wires through.. **IMPORTANT: You will need to drill into it at a 45°
angle. If you drill straight down the wires will not feed through the handle.**


Start with a small drill bit to drill a pilot hole then work your way up. Careful not to go all
the way through to the other end. When finished it’ll look like this:
14) Take the 2-pin red/black and 2-pin blue/yellow connectors, C3 _(Essential Kit)_ or D1 _(Bare
Basics Kit)_ and feed the wires through the hole until they come out the other end. Taping
the ends up may help with feeding it through.
15) Reconnect the lever latch that we removed from step 11.


## Wand Installation

_Essential/Bare Basics Kit_
**_Important Tips about the terminal blocks before starting..._**

- The Terminal blocks on the wand board may come with the “elevator doors” closed. Be
    sure to open them by turning all the screws to the right. As you can see in the image,
    terminal block #1 on the right is opened while the rest are closed.
- You will run into a situation where no matter how much you turn the screw to close the
    terminal block it just won’t close. Take some tweezers and apply pressure to the edge of
    the exposed door while pushing upwards towards the screw. At the same time use your
    other hand to turn the screw.This will reconnect the door and the screw. Yes, it’s a little
    tricky to hold everything with both hands as well as working with such small objects but it
    doesn't take much to reconnect it.


**Reconnect the exposed wires to the terminal blocks on the new board.**
Here is a diagram for you to follow. Alternatively check out the github page for a bigger image as
well as a chart. https://github.com/gpstar81/haslab-proton-pack/blob/main/WAND_PCB.md
Otherwise you can follow the step by step instructions here per set of wires on the next page.


1) Barrel Extension: Orange wires to A7/GND


2) Slo Blo LED: Red wire to D8, black wire to GND
**IMPORTANT TIP:** _there are 3 sets of red/black wires, 2 of the sets are joined together.
Be sure to use the one that is connected to the Slo Blo LED. You can tug at them and
see if the wires connected to the LED move to know which one is correct. The other set
is for the stock Hasbro battery tray. We Do not need this anymore. You can either snip it
off, or tape up the ends and shove it down inside the body._


3) Lower Toggle: Brown wires to D4/GND


4) Upper Toggle: Red wires to A0/GND


5) Clippard LED: Red wire to D9, Yellow wire to GND. It is recommended to feed these
wires underneath the Wand board to get to the terminal block.


6) Rumble Motor: Red wire to R+, black wire to R-


7) Stock Bargraph: Blue wire to VCC, green wire to A5, yellow wire to A4, orange wire to
A3, red wire to A2, brown wire to A1


8) Blinking Top Right/Vent Light LED: White wire to D12, Black wire to D13, Red wire to
VL+
**IMPORTANT TIP:** _Do not mix up the connections here! Double check D12 and D13 are
correct. Mixing them up can short out the LED. Although the white wire is in the middle it
needs to cross over and connect to D12 while the black wire crosses over to D13._


9) Rotary Encoder (Top Knob): Orange wire to D7, red wire to D6, brown wire to ROT-


**Installing the 3D printed Cage**
1) Feed the speaker wires through the wand cage. It will be thread through the gap in the
bottom middle. With the wand board terminals facing you, seat it on the cage by going
under and up with the right side sitting on the ledge first. Then push up on the other end
until it is flush with the cage. Make sure to push the board down gently so the solder
joints under the board sit into the cutout on the platform.
2) Make sure the wires aren’t blocking the cage’s feet and seat it properly in the wand. The
solder joints on the bottom of the wand at HAT2 will seat into the long horizontal hole on
the wand cage tab that is circled in green in the above image.
3) Shove the terminal block wires down and inside the pit of the body. The area between
the cage and the speaker MUST be clear for the barrel to retract.


**Reconnect the stock & new connectors to the board**
1) Barrel LEDs: 3-pin connector to Q2


2) Mode/Alt Switch: 2-pin connector to SW6


3) Intensify/Activate Toggle: 4-pin connector to SW45/SW4


4) Wand handle power: 2-pin red/white to 5V-IN


5) Wand handle data: 2-pin blue/yellow connector to TX1/RX1



It will look like this when finished.
**Installing Wav Trigger Sound Board**
With the Wav Trigger facing up screw in the speaker wires to the speaker terminals with
the positive wire to the top terminal


Connect the 4 wire/6pin connector to AUDIOBOARD (grey 6-pin housing)


Sit the wav trigger on the pegs on top of the cage. Use tape if it helps hold it down.
Make sure all the wires are clear of the barrel. Move it back and forth ensuring it moves
smoothly without snagging on anything. Reattach the body and test that it closes
properly. Test the barrel again to make sure it glides smoothly.


```
Take the gun track and snip off the 2 nubs.
```
## Installing the Hose

```
Essential Kit ONLY
1) Take the 3D printed wand connector...you’ll notice that there are 2 of them. This is
because Hasbro released 2 versions of the wand where the inner handle threads start
from 2 different spots. Test out the connectors until you get the one with the screw hole
that lines up with the bottom of the wand. Take the screw (final product screw may differ)
and prepare it by screwing it in partially.
```

2) Take the 4 wires coming out of the wand handle and connect it to C2, via the screw
terminals.Black wire to 1-, red wire to 1+, blue wire to 2-, yellow wire to 2+.
**IMPORTANT TIP** _Make sure the terminals clamp down on the tinned wire end and NOT
the plastic covering aka do not insert the wire too deep._
3) You are given 5’ of loom hose to use. If you plan to mod it (ex. Make an Afterlife hose)
then do so before you do any of the following wiring.
4) Take B2 and insert the 4 long wires and feed them through your hose. It helps to tape up
the ends together. If you are using ONLY the loom hose, remove the end cap on the
Cnlinko and the hose fits perfectly around it.


5) Take those 4 wire ends and run them through 2 out of 4 pieces of C1.
6) Then connect the wires to the 3rd piece. The numbers are a lot smaller and harder to
read than the other connector. Black wire to 1-, red wire to 1+, blue wire to 2-, yellow
wire to 2+.
**IMPORTANT TIP** _Make sure the terminals clamp down on the tinned wire end and NOT
the plastic covering aka do not insert the wire too deep._
7) Do a test fit to make sure everything is lined up correctly. With the connectors attached
you can trace the wires and see if they are lined up...black to black, red to red, blue to
blue, yellow to yellow.
8) Line up the holes of C2 with the hole in the wand connector and screw it in together with
the provided screw (final product screw may differ).


9) Complete the assembly of C1.
10) Using a lot of electrical tape and tape up the ends to the connectors.On the wand end it
helps to have the tape go up high along the hose as it helps with the bending.


_Bare Basics Kit ONLY_
The connections are the same as above. Whatever custom hose you are planning to use, the
important thing is to ensure the wires in package D line up from the pack to the wand via the
colors.
**FINAL MAIN KIT TEST!** - _Essential/Bare Basics Kit_
1) Connect the hose to both the pack and wand ends. Turn on the talentcell battery. You
will notice the vent light on the wand lights up for a split second followed by the bargraph
lights. This is a good indication that power is successfully routed to the wand board.
2) Next–you know the drill at this point... double triple check the hose connections and
anything else you feel the need to.
3) Say that prayer....
4) LIGHT ‘EM UP!
For full details on operating the kit please refer to the physical manual, digital manual on Github ,
or live walk through via Dustin Grau on Youtube.


## Add-on: Inner Cyclotron Cake & Panel LEDs Kit

```
1) If you have your own 3D printed cake you can skip step 1.
The cyclotron base and lid are glued together. Take L9 and insert into the little space
between the base and lid. Push up until you hear the glue break with an audible “Pop!”.
DO NOT pry the lid outward. You want to go up and in and back out, move over a bit,
then up and in and back out again. You will repeat this going all the way around the
cyclotron cake. It will take some time so be patient. Once you have successfully broken
off the glue you can pry it open.
```

2) Place L5 inside the cake until it is centered and mark the inner posts that it’s sitting on.
Then take some snipper and break off the post that you marked all the way to the
bottom. Depending on how good your snippers are it helps to twist as you cut.


3) Find the hexagon piece on the side of the cake and drill a big hole in the cake enough
for the 3-pin LED ring connector to feed through.
4) Take some velcro and stick it on adjacent sides of the LED ring and stick it on. Trim off
the excess Make sure the wires line up with the hole. Then put the lid back on securing it
with your glue of choice.


5) On the inside of the pack cyclotron area, unscrew these 4 screws.
6) Unplug the 4-pin blue connector from the Pack board and take out the toggle switches in
the inner panel by unscrewing the nuts. **IMPORTANT TIP** The wires connected to the
toggle switches are delicate so don’t tug on them.


7) Then unscrew these 4 and slide out the panel.
8) With the 3D printed replacement panel, feed L1, L2, L3 and L4 through the
corresponding holes in the panel. Note: L2, the green LED’s have blue wires.


9) Insert the LED’s into the corresponding cliplites that match the color of the LEDs and
plug them into the panel.
10) Reapply the toggle switches and slide the panel back in. The toggle switches should be
on the bottom.


11) Take L6 and place it in the gap where the N-filter slot is and then place the top back on
making sure those wires feed through the gap. Use tape to hold it down flat.
12) Screw all the partsback together and reattach to the pack. Be mindful of L6 making sure
it clears the gaps in the n-filter area and isn’t pinched by the outer shell.


13) Plug the connectors in and do not mix them up starting with L4 - Red LEDS
14) L3 - Yellow LEDs


15) L2 - Green LEDs
16) L1 - Yellow & Green LEDs


17) L6 - LED Ring Extension Cable
18) Connect the Cyclotron Cake to the extension cable.


19) Triple check the connections.
20) Test it out!
21) If you have the Frutto technology add-ons (28 segment bargraph, 15 LED powercell, 5
LED x4 cyclotron lid replacement) then check out our installation videos.If you don’t then
go ahead and close it all back up.
22) Starting with some cable management use tape and zip ties to secure all the wires down
making sure they don’t impede the motherboard when placing it back on.
You can use this alternate route on the middle-right side. Just limit it to 4 wires and
ensure they are flat along the inner wall otherwise it’ll block the d-cell battery tray.


Now place the mother back on the pack and secure it. Close up your wand.

## CONGRATS YOU ARE FINISHED!! GET OUT THERE AND

## BUST SOME GHOSTS!

For an operation manual please visit
https://github.com/gpstar81/haslab-proton-pack/blob/main/OPERATION.md


## Post-installation Guides

## Updating Firmware

**Install required software**
1) Download Arduino from https://www.arduino.cc/en/software
2) Open the downloaded file and follow the installation process


```
3) Run Arduino
4) Go to Sketch -> Include Library -> Manage Libraries to access the Library Manager
Search for the following libraries by name and install the latest version available:
```
- **FastLED** by Daniel Garcia
- **ezButton** by ArduinoGetStarted.com
- **Ramp** by Sylvain Garnavault
- **AltSoftSerial** by Paul Stoffregen
- **simple ht16k33** library by lpaseen
- **SerialTransfer** by PowerBroker2
- **millisDelay** _See Below_
- **WavTrigger** _See Below_
**_MillisDelay_**
The MillisDelay library must be downloaded from the project GitHub page. Download the code
as a zip and use the Sketch -> Add .ZIP Library option to import the downloaded file.
https://github.com/ansonhe97/millisDelay
_No further configuration is needed for this library._
**_WavTrigger_**
The WavTrigger library must be downloaded from the project GitHub page. Download the code
as a zip and use the Sketch -> Add .ZIP Library option to import the downloaded file.
https://github.com/robertsonics/WAV-Trigger-Arduino-Serial-Library
**_+++ IMPORTANT +++_**
Documented on both the Proton Pack and Neutron Wand code at the top of each file, is
instructions on modifying the wavTrigger.h file. You will need to open the wavTrigger.h file and
comment out the necessary _#define_ required for the Proton Pack and Neutrona Wand.


(see For compiling code on the gpstar Proton Pack and gpstar Neutrona Wand below for more
information).
The wavTrigger.h file can be located in your Arduino/Libraries/<wav trigger folder> (MacOS) or
C:\Arduino\Libraries (Windows). See the special notes below for compiling the code for the
gpstar Proton Pack and Neutrona Wand boards.
For compiling code for the gpstar Proton Pack PCB and gpstar Neutrona Wand PCB:
You need to enable __WT_USE_SERIAL_3__ by uncommenting this line inside the
wavTrigger.h file. Be sure to comment out any other serial class option previously in use.
5) Open ProtonPack folder and run ProtonPack.ino


6) Select the correct board by clicking Tools -> Board: -> Arduino AVR Boards -> Arduino
Mega or Mega 2560
**Download & uploaded latest firmware**
1) Download files from [http://www....](http://www....)
2) Navigate to ProtonPack Folder and open ProtonPack.ino


7) Using the programming cable connect to your PC and Plug in the Pack Board. Ensure
the **black wire lines up with GND** and **green wire lines up with DTR**
8) Check that the correct port is selection and the board is set to Arduino Mega or Mega
2560


9) Upload
10) If done correctly you sure see this
11) Close the file and navigate to NeutronaWand folder and run NeutronaWand.ino


12) Using the programming cable connect to your PC and Plug in the Wand Board. Ensure
the **black wire lines up with GND** and **green wire lines up with DTR**
13) Upload


## Custom configurations via PC

_There are numerous custom settings you can alter within the wand sub menus (see operation
manual ). An alternate route is to change it on the configuration.h files on the PC and then
reflash it._
1) In either the ProtonPack or NeutronaWand folder open configuration.h
2) Follow the instructions within the file on how to edit certain features
3) Reflash the board as per the previous instructions

## Loading music onto SD card

- Sound effects are prefixed with numbers 001-499
- Music files will be prefixed as 500 and above.
**DO NOT LEAVE GAPS IN PREFIXES WHEN NUMBERING FILES!** The only exception is
between the last sound effect and first music track
To add more music


1) Using Audacity or similar free audio software convert music files to wav (44khz stereo).
**To batch convert several songs go to File→ Export→ Export Multiple...**
2) Rename files.File naming of the music tracks start at 500. So for example,
500_somesong.wav, 501_somesong.wav, 502_somesong.wav, etc. Do not leave any
gaps in the number sequencing! (For example of what NOT to do, 500_somesong.wav
followed by 502_somesong.wav leaving off 501_somesong.wav on the SD card.) The
most and only important part in the naming is the XXX_ prefix for all tracks.
If you have many tracks use Mp3tag to do a batch renaming of the files.
3) Load up all the music and select them all.
4) Click Rename files


5) Under format select **“%# - %T”** and add a 5 in front of it. It should look like **“5%# - %T”**.
Select **“Replace spaces by _”**
6) Click “rename files”
7) Double check that the tracks start from 501 in ascending order with no gaps in the
numbering.
8) Copy them to both SD cards.
