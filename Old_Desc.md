### Original Scope and Description (Pure Input Viewer)

#### Notes:

Recommended circuit for handling the inputs (per Hatsune Mike):

```
Button
|
|--- level-shifting buffer (e.g. 74LVCH16245) --> GPIO input (in non-pullup mode)
|
PCB
```

> IDT74LVCH16245A explicitly calls out "All pins can be driven from either 3.3V or 5V devices"
"This feature allows the use of this device as a translator in a mixed 3.3/5V supply system." you can just die DIR and OE to GND, hook up the inputs to bus B, and read bus A using the MCU/Rpi/whatever

Super high level diagram:
```
Button
|
|--- Level-shifting buffer -- PISO register chain -- serial input* on ESP
|
PCB
```
*where serial is preferably a hardware serial; though the ESP has enough power that it can probably bitbang the serial just fine over regular GPIO

#### Description

This project is an attempt to make a real-time input display solution that can capture arcade hardware inputs (in JAMMA format) and produce an animated visualization that is usable as an OBS Browser Source for live streaming. It is inspired by https://github.com/wnka/arcadebuttons-node-pi, a similar project using a Raspberry Pi.

It is **not** any of the following:

- *A plug and play solution for console or PC controllers.*
  - Though they could be adapted to it, console and PC controllers use different standards and won't work without modification.
  - ESP In. De. relies on capturing digital signals from each button or lever microswitch. These signals can usually be tapped directly from a console controller PCB, but this is an advanced and unsupported use case.
  - For PC users looking to visualize keyboard or USB controller inputs, there are already many excellent tools available *(for example, https://github.com/univrsal/input-overlay, https://gamepadviewer.com/#about, etc.)*
- *A tool to capture inputs with 100% reliability.*
  - When connected to a good WiFi signal and in a normal home EMI (Electro-Magnetic Interference) environment, ESP In. De. actually does capture inputs with excellent accuracy. However, it uses WiFi connectivity, and uses a fast, but not guaranteed, network transport protocol (UDP WebSockets). Thus it cannot be relied upon for situations where every single input is critical.
- *A tool to capture 'TAS'es or input recordings of arcade games.*
  - As mentioned above, the accuracy of this tool is not guaranteed, and it does not preserve input 'recordings' in any meaningful way.
  - Furthermore, the technical feasibility of 'TAS'ing arcade hardware varies dramatically from game to game, and is well beyond the scope of this project.
