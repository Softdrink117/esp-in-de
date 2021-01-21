# ESP In. De.
 **ESP**2866 **In**put **De**vice
*(eh-spin-day)*

-----

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
