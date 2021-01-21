#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WebSocketsServer.h>

#include "credentials.h";
// For dev purposes, WiFi credentials are stored
// in a non-tracked 'credentials.h' file.
// A 'credentials.h.template' file has been included;
// put your credentials there and rename it to
// 'credentials.h', or just remove the include above
// and directly enter your details below

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* hostName = "ESP-Input";
const int port = 80;
const int webSocketPort = 81;


// Define pins as constants
// Current pinout is based on 'D1 Mini' board:
// UP    = GPIO 5  = pin D1
// DOWN  = GPIO 13 = pin D7
// LEFT  = GPIO 4  = pin D2
// RIGHT = GPIO 12 = pin D6

#define PIN_UP 5
#define PIN_DOWN 13
#define PIN_LEFT 4
#define PIN_RIGHT 12


ESP8266WebServer server(port);
WebSocketsServer webSocket(webSocketPort);

void setup()
{
  Serial.begin(115200);
  StartWiFi();
  StartSPIFFS();
  SetupPins();
  StartWebSocket();
  StartWebServer();
}

void StartWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostName);
  WiFi.begin(ssid, password );
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nJoined WiFi Network");
  Serial.println(WiFi.SSID());
  Serial.println(String("Using hostname ") + String(hostName));
}

void StartSPIFFS()
{
  Serial.print("Mouting SPIFFS Filesystem...");
  if(!SPIFFS.begin())
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  Serial.println(" Done.");
  Serial.println("Contents:");
  ListFiles();
}

void SetupPins()
{
  Serial.print("Setting input pin modes...");
  pinMode(PIN_UP, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);
  pinMode(PIN_LEFT, INPUT_PULLUP);
  pinMode(PIN_RIGHT, INPUT_PULLUP);
  Serial.println(" Done.");
}

void StartWebSocket()
{
  Serial.print(String("Starting WebSocket Server on port ") + String(webSocketPort) + String("..."));
  webSocket.begin();
  webSocket.onEvent(WebSocketEvent);
  Serial.println(" Done.");
}

void StartWebServer()
{
  Serial.println(String("Starting web server at ") + WiFi.localIP().toString() + String(":") + String(port));
  Serial.println();

  server.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });

  server.begin();
}

// MAIN LOOP =================================================================================================

void loop(void){
  webSocket.loop();
  server.handleClient();
  ReadInputs();
}

// INPUT HANDLING ============================================================================================

uint8_t state = 0;
uint8_t pState = 0;
// Read input state into bitflags
// UDLRABCD format
void ReadInputs()
{
  pState = state;
  state = 0;
  if(digitalRead(PIN_UP) == LOW)
    state |= 1 << 7;
  if(digitalRead(PIN_DOWN) == LOW)
    state |= 1 << 6;
  if(digitalRead(PIN_LEFT) == LOW)
    state |= 1 << 5;
  if(digitalRead(PIN_RIGHT) == LOW)
    state |= 1 << 4;

  if(state != pState)
  {
//    Serial.println("UDLRABCD - " + StateToString());
    Serial.println("UDLRABCD - " + StateToBinaryString());

//    webSocket.sendBIN(0, (uint8_t*) &state, sizeof(state));
    webSocket.broadcastBIN((uint8_t*) &state, sizeof(state));
  }
}

String StateToString()
{
  String out = "";
  if(state & (1 << 7)) out += "U";
  if(state & (1 << 6)) out += "D";
  if(state & (1 << 5)) out += "L";
  if(state & (1 << 4)) out += "R";

  return out;
}

String StateToBinaryString()
{
  String out = "";
  for(int i = 7; i >= 0; i--)
  {
    if(state & (1 << i)) out += "1";
    else out += "0";
  }
  return out;
}


// WEBSOCKET HANDLING ======================================================================================

// Called when a WebSocket message is received
// Though there are other WStypes, rx is minimal for this project, so text is fine as payload
// See https://github.com/Links2004/arduinoWebSockets for other type definitions
void WebSocketEvent(byte num, WStype_t type, uint8_t* payload, size_t length)
{
  switch(type)
  {
    case WStype_ERROR:
      break;
    case WStype_DISCONNECTED:
      break;
    case WStype_CONNECTED:
      break;
    case WStype_TEXT:
      break;
  }
}


// FILE HANDLING ===========================================================================================

// Adapted from https://tttapa.github.io/ESP8266/Chap11%20-%20SPIFFS.html

void ListFiles()
{
  Dir root = SPIFFS.openDir("/");
  while(root.next())
  {
    String filename = root.fileName();
    Serial.println(String("  File: ") + String(filename.c_str()));
  }
}

// convert the file extension to the MIME type
String getContentType(String filename)
{
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

// send the right file to the client (if it exists)
bool handleFileRead(String path)
{
  Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.html";           // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){  // If the file exists, either as a compressed archive, or normal
    if(SPIFFS.exists(pathWithGz))                          // If there's a compressed version available
      path += ".gz";                                         // Use the compressed version
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);
  return false;                                          // If the file doesn't exist, return false
}
