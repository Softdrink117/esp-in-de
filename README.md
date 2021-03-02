# ESP In. De.
 **ESP**2866 **In**put **De**vice  
エスピンデ  
*(ehsp-in-day)*

-----

## New Scope (c. Feb 10 2021)

This project describes a multi-function input intermediary for arcade cabinets that acts as a button remapper, autofire circuit, and input viewer for live streams.

### Currently Defined Features

- Support for 2L8B (JAMMA spec)
  - Optional: Support for 2L12B or 2L16B
    - Normally I wouldn't be a big fan of 16B support (too many darn buttons) but for the specific case of autofire and remapping I think it actually has some merit
  - Start buttons are supported
- Input viewer: animated web visualization that can be used as an OBS Browser Source for live streams
- Input remapper: arbitrary reassignment of buttons to user preference
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
     - Current theory: I2C OLED with persistence + some tac switches (maybe 3?) to navigate options. Text-based interface should be sufficient

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

- What input format(s) are supported?
- Physical design:
  - Is it a JAMMA interstitial, or something that integrates more directly with the cabinet?
    - Cabinet is preferable, but adds complexity:
      - The input side probably goes between the CP and the cabinet harness.
        - This is super convenient on SEGA cabs since they offer power and ground to the CP connectors. Do other cabs do this?
        - How should this be connected? Does it make sense to use an existing connector format (AMP-UP for example), or would it be better to have headers on the PCB and small breakout harnesses to adapt to various cabinets?
      - How does sync sense work?
        - Is there a separate 'sync sense PCB' that can be physically located separately from the main input PCB?
          - This would add a lot of flexibility to the supported cabinet types, but also adds complexity to design and fabrication
    - JAMMA is easier but limits scope to JAMMA games or JAMMA/JVS IO
- Supported features:
  - Onboard configuration adds complexity to the PCB design and programming, and web UI is sufficient for all main functions. But web UI is not always ideal. Plus, ability to set/view IP from the hardware would be great for multi-cabinet setups.
  - Requested features above - of particular note is the macro functionality, which is a value add but also complicates interface design.

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
[GPIO/serial input* ]
[                   ]
[MCU            GPIO]----- Optional Sync Sense ----- [ Sync processing (splitting etc.)]
[                   ]                                         |
[GPIO/serial output*]                                         |
|                                                             |
SIPO register(s)                                              |
|                                                             |
Open-collector/drain buffer/inverter                          |
|                                                             |
Cabinet Input Harness(es)                           Cabinet Video Harness
```
*Serials could be hardware, which would be ideal, but given the input polling/output frequency it's honestly probably fine to just bitbang on regular GPIO if needed

If the onboard hardware interface is included, that adds another couple buttons (which can probably be wired through unused bits of the input register chain) and an I2C display

### BOM (Still WIP, not yet prototyped in hardware!!!)

#### Input

| Part | Quantity | Through-Hole | SMD |
| ---: | :------: | :----------- | :--- |
| Input Pullup Resistor Array | 4 | [4609X-101-103LF](https://www.digikey.com/en/products/detail/4609X-101-103LF/4609X-101-103LF-ND/2634616) |   |
| PISO Shift Register | 4 |
| SIPO Shift Register | 4 | [SN74AHC594N‎](https://www.digikey.com/en/products/detail/SN74AHC594N/296-33681-5-ND/1566900)
| Open-Drain Inverter | 5 | [CD74AC05E](https://www.digikey.com/en/products/detail/texas-instruments/CD74AC05E/375662) | [SN74LVC06AD](https://www.digikey.com/en/products/detail/texas-instruments/SN74LVC06AD/277342)

#### Sync

## Data Format

Inputs are ingested through shift registers, so it comes in as binary. Registers are usually available in 8 or 16-bit widths, which also aligns with some input formats neatly.

I propose the following bit layouts, depending on our decision for the supported input options. Right now the demonstration is already using the 32-bit format specified below, but that is easily changed:

```
2L6B + Start -> 16 bits
-P1----- -P2-----
UDLRSABC UDLRSABC

2L8B -> 16 bits
-P1----- -P2-----
UDLRABCD UDLRABCD

2L12B + Start -> 20 bits (round up to 24, can add Starts, 2 extra for onboard hw)
-P1----- -Shared- -P2-----
UDLRSABC DEF0DEF0 UDLRSABC

2L16B + Start -> 26 bits (round up to 32, 6 extra for onboard hw)
-P1----- -------- -P2----- --------
UDLRSABC DEFGH000 UDLRSABC DEFGH000
```

#### Onboard Interface Notes

Basic text/ASCII art based interface.

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

This project is an attempt to make a real-time input display solution that can capture arcade hardware inputs (in JAMMA format) and produce an animated visualization that is usable as an OBS Browser Source for live streaming.

It is **not** any of the following:

- *A plug and play solution for console or PC controllers.*
  - Though they could be adapted to it, console and PC controllers use different standards and won't work without modification.
  - ESP In. De. relies on capturing digital signals from each button or lever microswitch. These signals can usually be tapped directly from a console controller PCB, but this is an advanced and unsupported use case.
  - For PC users looking to visualize keyboard or USB controller inputs, there are already many excellent tools available *(PROVIDE A LINK HERE)*
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
