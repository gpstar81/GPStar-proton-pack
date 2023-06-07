# Wand Bargraph Upgrade

This guide outlines the **optional** upgrade from the 5-LED stock bargraph to a more animated 28-segment bargraph. Additional parts will be required, as well as a more invasive disassembling of the wand.

### BARMETER 28 SEGMENT BARGRAPH

The stock Hasbro 5 LED bargraph indicated above, can be replaced with a Barmeter 28 Segment Bargraph (BL28Z-3005SA04Y). You can find the Barmeter pin reference below this connection guide for referencing the Barmeter Pin numbers. See the component fitment guide at the bottom of this page for how to mount this Bargraph into your wand body.

| HT16K33 Board | PCB    | Nano Pin | Barmeter Pin |
|---------------|--------|----------|--------------|
| VDD           | +5V    |          |              |
| GND           | Ground |          |              |
| SDA           | →      | Pin A5   |              |
| SCL           | →      | Pin A4   |              |
| A0            | →      | →        | 22 (C1)      |
| A1            | →      | →        |  1 (C2)      |
| A2            | →      | →        | 19 (C3)      |
| A3            | →      | →        | 18 (C4)      |
| A4            | →      | →        |  7 (C5)      |
| A5            | →      | →        | 10 (C6)      |
| A6            | →      | →        | 11 (C7)      |
| C0            | →      | →        | 21 (L1)      |
| C1            | →      | →        | 15 (L2)      |
| C2            | →      | →        | 13 (L3)      |
| C3            | →      | →        | 16 (L4)      |

**Note** - If using the all-in-one PCB controller, connect the SDA and SCL pins directly to the expansion port available on that board. Additionally, the VDD and GND pins may make use of the 5V-OUT connection on that controller.

![](images/barmeter.jpg)

### BARMETER 28 SEGMENT BARGRAPH - BL28Z-3005SA04Y - Pin Layout Reference

| Pin# | 1  | 2 | 3 | 4 | 5 | 6 | 7  | 8 | 9 | 10 | 11 | 12 | 13 | 14 | 15 | 16 | 17 | 18 | 19 | 20 | 21 | 22 |
|------|----|---|---|---|---|---|----|---|---|----|----|----|----|----|----|----|----|----|----|----|----|----|
|      | C2 |   |   |   |   |   | C5 |   |   | C6 | C7 |    |    |    |    |    |    | C4 | C3 |    |    | C1 |
|      |    |   |   |   |   |   |    |   |   |    |    |    | L3 |    | L2 | L4 |    |    |    |    | L1 |    |