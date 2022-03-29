# ESP In. De.
 **ESP**2866 **In**put **De**vice  
エスピンデ  
*(ehsp-in-day)*

-----

## New Scope (c. Feb 10 2021)

This project describes a multi-function input intermediary for arcade cabinets that acts as a button remapper, autofire circuit, and input viewer for live streams.

### Currently Defined Features

- Support for 2L16B + Start
- Input viewer: animated web visualization that can be used as an OBS Browser Source for live streams
- Input remapper: arbitrary reassignment of buttons and lever to user preference
- Sideswap: allow a 1LxB control panel to be used as Player 1 or Player 2
  - Optional: Trigger sideswap from a button combination when Start is pressed
- Autofire:
  - Interval-based autofire
     - Input for X ticks, wait for Y ticks, repeat
  - Frequency-based autofire
     - Available as a fallback if sync is not connected
     - Implemented on a clock timer (if sync disconnected) or through dividing the vertical sync (EG: ~60Hz sync / 3 ≈ 20Hz auto, / 4 ≈ 15Hz auto, / 5 ≈ 12Hz auto, etc.)
- Configuration interface:
  - Available over the same web interface as the input display
  - Optional: onboard graphical hardware interface as well
     - Current theory: I2C OLED with persistence + rotary encoder with push switch

### Feature Requests

   - Profile support
     - Save a configuration to a profile and reload from the interface
   - On-the-fly profile swap
     - Change between configurations with a button press. Example per Kiwi:
     > For example, profile 1 has all the same settings as profile 2, but button D has 12hz A instead of 6hz A.  Then, a profile switch button could be bound to something such as player 2 start to swap between the two.  This way you could swap between the supershot rates of W and S in the middle of a run based on what powerup you collect.  I can think of some more practical and complex applications for this than just the D button, but either way it could be an interesting addition for complex setups and casual use alike.

   - Macros (multiple buttons triggered in sequence with one press)
     - Another example from Kiwi:

     ```
     Macro to reliably perform the Mushihimesama glitch
     Frame | 0 1 2
     - - - | - - -
     Btn A | 0 1 1
     Btn C | 1 1 0
     ```
   - Delays (before an interval - mostly meant as an 'if we can't do a macro, this can be used to do it manually' thing)
     - IE: When button pressed, wait for X frames before starting the interval, then repeat it as normal

### Decision Points

   - Physical design:
     - ~~Cabinet interstitial connectors / 'plug and play'~~
     - ~~Sync board~~
     - ~~Interconnect between sync board and main board~~
     - For now, decided on developing the JAMMA version first. Quick way to start, scalable to a variety of installs, and integrates sync handling (since JAMMA already contains video signals).
   - Supported features:
     - Onboard configuration adds complexity to the PCB design and programming, and web UI is sufficient for all main functions. But web UI is not always ideal. Plus, ability to set/view IP from the hardware would be great for multi-cabinet setups.
     - Requested features above - of particular note is the macro functionality, which is a value add but also complicates interface design.

## Inputs and Outputs

### Supported Inputs

One Joystick Lever and eight Button inputs per player are supported, with up to 5 buttons on the JAMMA edge and up to three buttons on a CPS2 Kick Harness compatible header. Between both players, a total of 2 Levers and 16 Buttons are supported.

All eight button inputs are considered separately by *ESP In. De.*, and are fully configurable and remappable.

### Supported Outputs

One Joystick Lever and eight Button outputs per player are supported, with up to 5 buttons on the JAMMA edge and up to three buttons on a Kick Harness connector. Between both players, a total of 2 Levers and 16 Buttons are supported.

**Note: while any input may be bound to any output, care must be taken when binding outputs. Some games do not respond well to a button signal being present on both the JAMMA edge and a kick harness.** As an example, CPS2 Street Fighter games seem to read both JAMMA button 4 and kick button 4 as the same input, meaning that some inputs will not register properly if both of them are connected.

### DIP Switches

Several DIP switches are available for configuration and quickly enabling/disabling different features. The DIPs control the following features (NOT FINALIZED):

| Switch | Name | Description |
| :----: | :--- | :---------- |
| SW1 | Bypass Auto | A hardware switch to disable all 'autofire' input processing (autofire, macros, etc.). <br>It still allows remapping and sideswap to be used. |
| SW2 | Bypass Remap | A hardware switch to disable all remap (and duplication) features. <br>It still allows 'autofire' processing and sideswap to be used. |
| SW3 | Bypass Sideswap | A hardware switch to disable sideswap. <br>It still allows remap and 'autofire' processing to be used. |
| SW4 | | |
| SW5 | | |
| SW6 | | |

Combinations of switches can be used to create 'lockout' conditions quickly, for tournament play or to quickly revert to stock. For example, `SW1 ON`, `SW2 ON`, and `SW3 ON` together revert the cabinet to straight input passthrough, without any modifications.



## Circuit Design

Very very high level, pls no bully

Power and other 'infrastructure' omitted for clarity

```
Buttons/Levers
|
|
Pullup resistor array (10kOhm)
|
PISO register(s)
|
[SPI Input          ]
[                   ]
[MCU            GPIO]----- Optional Sync Sense ----- [ Sync Processing ]
[                   ]                                         |
[SPI Output         ]                                         |
|                                                             |
SIPO register(s)                                              |
|                                                             |
Open-collector/drain buffer/inverter                          |
|                                                             |
Cabinet Input Harness(es)                           Cabinet Video Harness
```

If the onboard hardware interface is included, that adds another couple buttons (which can probably be wired through unused bits of the input register chain) and an I2C display

### Sync Processing

Signal should be buffered (or inverted) before being processed. TTL level according to JAMMA spec, so should be pretty straightforward.

Two options stand out:

- **RC circuit low-pass filter** (tuned around 120Hz) to pass only vertical sync pulses, then Schmitt trigger or software debounce.
- **LM1881** (or the similar LMH1980) sync ICs. Though designed for use with composite video, they have been proven to work with composite sync through JAMMA in other devices.

It would also be possible to directly connect to the GPIO and simply use software sampling and filtering to evaluate, but this requires a high polling rate to ensure accuracy (ideally around 60kHz). Alternatively, an interrupt could be used. Per Hatsune Mike:

> Could hook up TTL sync to an interrupt, and then in the ISR poll until half a line’s duration (0.5 x 1/15khz) and if it’s still asserted then chances are you’re in vblank. Then ignore sync pulses for the next ~8ms or so

As long as that ends up being <= sync pulse duration at 31kHz (which is theoretically 1/2 the pulse at 15kHz), that should be scalable to higher resolutions as well. Maybe would need to do `0.4 * 1/15kHz` or something to give a bit more buffer, but the basic theory seems sound.

### BOM (Still WIP, not yet prototyped in hardware!!!)

#### Input

| <div style="width:200px">Part</div> | Quantity | Through-Hole | SMD |
| ---: | :------: | :----------- | :--- |
| Pullup Resistor Array | 4 | [4609X-101-103LF](https://www.digikey.com/en/products/detail/4609X-101-103LF/4609X-101-103LF-ND/2634616) |   |
| PISO Shift Register | 4 | 74HC165
| SIPO Shift Register | 4 | [SN74AHC594N‎](https://www.digikey.com/en/products/detail/SN74AHC594N/296-33681-5-ND/1566900)
| Open-Drain Inverter | 5 | [CD74AC05E](https://www.digikey.com/en/products/detail/texas-instruments/CD74AC05E/375662) | [SN74LVC06AD](https://www.digikey.com/en/products/detail/texas-instruments/SN74LVC06AD/277342)
| Rotary Encoder with Switch | 1 | [PEC12R-2217F-S0024](https://www.digikey.com/en/products/detail/bourns-inc/PEC12R-2217F-S0024/4499642) | |

#### Sync

| <div style="width:200px">Part</div>  | Quantity | <div style="width:200px">Through-Hole</div> | SMD |
| ---: | :------: | :----------- | :--- |
| Non-Inverting Buffer | 1 | | [SN74LVC1G34](https://www.digikey.com/en/products/detail/texas-instruments/SN74LVC1G34DBVR/738115) |

## Data Format

Inputs are ingested through shift registers, so it comes in as binary. Registers are usually available in 8 or 16-bit widths, which also aligns with some input formats neatly.

I propose the following bit layout:

```
2L16B + Start -> 26 bits (round up to 32, 6 extra for onboard hw)
-P1----- -------- -P2----- --------
UDLRSABC DEFGH000 UDLRSABC DEFGH000
```

I can stuff extra inputs into the remaining space, for DIP switches:
```
2L16B + Start -> 26 bits (round up to 32, 6 extra for onboard hw)
-P1----- -------- -P2----- --------
UDLRSABC DEFGHijk UDLRSABC DEFGHxyz

Where ijkxyz correspond to DIP switches 01-06
```

Ingest as binary means quick operations with bitmasks, eg:

```
Mask out DIPs   |  output = (input & 11111111 11111000 11111111 11111000)
Remap P1 to P2  |  output = (input >> 16)
Remap P2 to P1  |  output = (input << 16)
```

#### Onboard Interface Notes

Basic text/ASCII art based interface?

Idea: Can visualize button state with a pretty simple representation of `[XYZ]`, where `X` is the button identifier, `Y` is the auto mode, and `Z` is the rate/interval. For example:
```
[A]    [B]     [C] []
[Ar30] [Ai1-2] [B] []
```
The configuration above reads as follows:

- Physical Button A - Logical Button A
- Physical Button B - Logical Button B
- Physical Button C - Logical Button C
- Physical Button D - Logical Button A, 30Hz autofire (or whatever `roundToInt(sync/2)` is)
- Physical Button E - Logical Button A, 1-on 2-off interval
- Physical Button F - Logical Button B

Does this work or is it actually bad?

-----

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


  -----

  ### Installation instructions:

  Required libraries:
  - **ESP8266 Core Libraries** (installed via Board Manager)
  - **ESP8266FS plugin** for Arduino IDE (https://github.com/esp8266/arduino-esp8266fs-plugin)
  - **Arduino WebSockets Server and Client** from links2004 (https://github.com/Links2004/arduinoWebSockets) installed as a ZIP library in the IDE


  To flash resources (html, css, etc.) to the SPI filesystem:
  - Make sure *Tools>Flash Size* is greater than the combined size of entire sketch + data folder structure
  - Make sure *Tools>Flash Size* is less than or equal to the size of the installed memory on the board
  - Close serial monitor before upload
  - Enter Program mode on ESP if necessary (some boards have auto-program)
  - Ensure the ESP8266FS Tool is installed in Arduino Sketchbook>tools folder
  - https://github.com/esp8266/arduino-esp8266fs-plugin
  - Upload the data folder contents using *Tools>ESP8266 Data Upload*
  - Upload the program data as normal from the Arduino IDE
