# Battery Recommendations

To adequately run your full Proton Pack + Neutrona Wand and any accessories you're going to need a suitable power source. As these kits have evolved to handle more accessories and features so have the requirements for battery recommendations.

## Available Voltages

The go-to maker of rechargeable lithium-ion batteries in a customer-friendly package is TalentCell. They offer a range of options from solely 12V to a combination of 24V/12V models, with some offering built-in USB (5V) output. With the introduction of the [GPStar Amplifier II](https://gpstartechnologies.com/products/gpstar-amplifier-ii) and [Power Hub II](https://gpstartechnologies.com/products/gpstar-power-hub-ii) devices your Proton Pack can be adapted to run on a 24V input voltage!

The following table list various models which have been used and tested, or deemed suitable for use. Please see the footnotes as necessary to learn of any special caveats when using certain models. The column "Normalized Capacity" is a measure in watt-hours of potential battery capacity taking the internal battery cell voltage and available amp-hour ratings into consideration (V * A = W). This provides an apples-to-apples comparison for battery power across both single-voltage and dual-voltage models, as for the latter models power can come from either the raw 24V or regulated 12V outputs and does not directly translate total amp-hours to actual runtime.

| Model ID | Model<br/>Voltage | Approx.<br/>Weight | Normalized<br/>Capacity (Wh) | 24V | 12V | 24V | 12V |
|----------|-------------------|--------------------|------------------------------|-----|-----|-----|-----|
| [YB1203000](https://talentcell.com/lithium-ion-battery/12v/yb1203000.html)                      | 12V | 190g | 33.30 Wh  | &mdash; | ~11.1V<sup>3</sup> | &mdash; | 3A |
| [YB1203000-USB](https://talentcell.com/lithium-ion-battery/12v/yb1203000-usb.html) <sup>1</sup> | 12V | 190g | 33.30 Wh  | &mdash; | ~11.1V<sup>3</sup> | &mdash; | 3A |
| [YB1206000](https://talentcell.com/lithium-ion-battery/12v/yb1206000.html)                      | 12V | 370g | 66.60 Wh  | &mdash; | ~11.1V<sup>3</sup> | &mdash; | 3A |
| [YB1206000-USB](https://talentcell.com/lithium-ion-battery/12v/yb1206000-usb.html) <sup>1</sup> | 12V | 380g | 66.60 Wh  | &mdash; | ~11.1V<sup>3</sup> | &mdash; | 3A |
| [PB120B1](https://talentcell.com/lithium-ion-battery/12v/pb120b1.html)                          | 12V | 730g | 142.08 Wh | &mdash; | ~11.1V<sup>3</sup> | &mdash; | 6A |
| [PB120B2](https://talentcell.com/lithium-ion-battery/12v/pb120b2.html)                          | 12V | 730g | 142.08 Wh | &mdash; | ~11.1V<sup>3</sup> | &mdash; | 6A |
| [PB240A1](https://talentcell.com/lithium-ion-battery/24v/pb240a1.html) <sup>2</sup>             | 24V | 500g | 82.88 Wh  | ~25.9V<sup>3</sup> | 12V | 3A | 2.0A |
| [PB240B1](https://talentcell.com/lithium-ion-battery/24v/pb240b1.html)                          | 24V | 500g | 129.50 Wh | ~25.9V<sup>3</sup> | 12V | 5A | 2.5A |
| [PB240B2](https://talentcell.com/lithium-ion-battery/24v/pb240b2.html)                          | 24V | 500g | 156.00 Wh | ~25.9V<sup>3</sup> | 12V | 5A | 2.5A |

<sup>1</sup> Use of a USB port for powering any GPStar electronics is **STRONGLY NOT RECOMMENDED**. All systems should be driven by the 24V or 12V barrel jacks as appropriate for the GPStar Amplifier installed.

<sup>2</sup> This 24V model is usable but not fully recommended as it can only supply up to 3A of power for the 24V port, and only 2A for the 12V port. This has been determined to not provide enough headroom to run the GPStar Amplifier II at full volume, especially when paired with larger or 4-ohm speakers which are intended to maximize the amplifier output.

<sup>3</sup> Per the manufacturer's stated specs, the internal cells are comprised of common 18650 batteries arranged in series (for voltage) and in parallel groups (for amperage). Therefore, the exact "raw" voltage of the battery is based on 3.7V cells and may result in a nominal voltage slightly under or over the stated battery rating. Please know that the [GPStar Amplifier](https://gpstartechnologies.com/products/gpstar-amplifier) and [GPStar Amplifier II](https://gpstartechnologies.com/products/gpstar-amplifier-ii) devices can accept a range of input voltage without issue, while providing a constant and regulated 5V output.

## Using Multiple Power Sources

For those who wish to make use of smoke effects, the need for multiple batteries has become more apparent and recommended. Every addition and upgrade to your pack and wand comes at a cost of power, and as of 2024 the number of LEDs in use may require up to 1.2A just for the base system functionality. While most DIY and community smoke kits may require 1.5-2.5A each, which easily pushes the lowest stated limit (2.5-3A) of a single 12V TalentCell battery output.

Please follow our diagrams in the [Smoke Guide](SMOKE.md) for more information about how to make use of multiple batteries. With the addition of the [GPStar Power Hub](https://gpstartechnologies.com/products/gpstar-power-hub) or [GPStar Power Hub II](https://gpstartechnologies.com/products/gpstar-power-hub-ii) this is much easier as you can use a single device with a standalone battery to operate up to 2 smoke kits simultaneously. Per our recommendations, smoke kits can and should be triggered by use of a relay which also makes this scenario possible, as it provides isolation between the power source to the primary electronics from the high-draw heating elements used in most smoke kits.