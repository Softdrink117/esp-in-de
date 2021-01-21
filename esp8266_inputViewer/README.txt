Installation instructions:

Required libraries:
- ESP8266 Core Libraries (installed via Board Manager)
- ESP8266FS plugin for Arduino IDE (https://github.com/esp8266/arduino-esp8266fs-plugin)
- Arduino WebSockets Server and Client from links2004 (https://github.com/Links2004/arduinoWebSockets) installed as a ZIP library in the IDE

To flash the resources to the SPI filesystem:
- Make sure Tools>Flash Size is > combined size of entire sketch + data folder structure
- Make sure Tools>Flash Size matches the size of the installed memory on the board
- CLOSE SERIAL MONITOR BEFORE UPLOAD
- Enter Program mode on ESP if necessary (some boards have auto-program)
- Ensure the ESP8266FS Tool is installed in Arduino Sketchbook>tools folder
- https://github.com/esp8266/arduino-esp8266fs-plugin
- Upload the data folder contents using Tools>ESP8266 Data Upload
- Upload the program data as normal