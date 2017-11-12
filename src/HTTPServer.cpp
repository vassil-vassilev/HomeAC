#include <FS.h>                                                 // This needs to be first, or it all crashes and burns

#include <Arduino.h>
#include <WiFiManager.h>                                        // https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>                                        // Useful to access to ESP by hostname.local
#include <Ticker.h>
#include <IRsend.h>
#include <IRControl.h>

const int ledpin = BUILTIN_LED;                                 // Built in LED defined for WEMOS people

bool shouldSaveConfig = false; // Flag for saving data
unsigned long lastupdate = 0;
const bool getExternalIP = true; // Set to false to disable querying external IP
String _ip = "";

const char *wifi_config_name = "IR Controller Configuration";
int port = 80;
char passcode[40] = "";
char host_name[40] = "";
char port_str[20] = "80";

Ticker ticker1;

ESP8266WebServer server(port);
HTTPClient http;
DynamicJsonBuffer jsonBuffer;
JsonObject& deviceState = jsonBuffer.createObject();

void setTicker (Ticker _ticker)
{
    ticker1 = _ticker;
}

void handleClient ()
{
    Serial.println("Memory: ");
    Serial.println(ESP.getFreeHeap());
    server.handleClient();
    //Serial.println("Memory2: ");
    //Serial.println(ESP.getFreeHeap());
}

//+=============================================================================
// Turn off the Led after timeout
//
void disableLed()
{
  digitalWrite(ledpin, HIGH);                           // Shut down the LED
  ticker1.detach();                                      // Stopping the ticker
}

//+=============================================================================
// IP Address to String
//
String ipToString(IPAddress ip)
{
    String s = "";
    for (int i = 0; i < 4; i++)
        s += i ? "." + String(ip[i]) : String(ip[i]);
    return s;
}

//+=============================================================================
// Callback notifying us of the need to save config
//
void saveConfigCallback()
{
    Serial.println("Should save config");
    shouldSaveConfig = true;
}

//+=============================================================================
// Toggle state
//
void tick()
{
    int state = digitalRead(ledpin); // get the current state of GPIO1 pin
    digitalWrite(ledpin, !state);    // set pin to the opposite state
}

//+=============================================================================
// Split string by character
//
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

//+=============================================================================
// Gets called when WiFiManager enters configuration mode
//
void configModeCallback(WiFiManager *myWiFiManager)
{
    Serial.println("Entered config mode");
    Serial.println(WiFi.softAPIP());
    //if you used auto generated SSID, print it
    Serial.println(myWiFiManager->getConfigPortalSSID());
    //entered config mode, make led toggle faster
    ticker1.attach(0.2, tick);
}

//+=============================================================================
// First setup of the Wifi.
// If return true, the Wifi is well connected.
// Should not return false if Wifi cannot be connected, it will loop
//
bool setupWifi(bool resetConf)
{
    // set led pin as output
    pinMode(ledpin, OUTPUT);
    // start ticker with 0.5 because we start in AP mode and try to connect
    ticker1.attach(0.6, tick);

    // WiFiManager
    // Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    // reset settings - for testing
    if (resetConf)
        wifiManager.resetSettings();

    // set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
    wifiManager.setAPCallback(configModeCallback);
    // set config save notify callback
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    if (SPIFFS.begin())
    {
        Serial.println("mounted file system");
        if (SPIFFS.exists("/config.json"))
        {
            //file exists, reading and loading
            Serial.println("reading config file");
            File configFile = SPIFFS.open("/config.json", "r");
            if (configFile)
            {
                Serial.println("opened config file");
                size_t size = configFile.size();
                // Allocate a buffer to store contents of the file.
                std::unique_ptr<char[]> buf(new char[size]);

                configFile.readBytes(buf.get(), size);
                DynamicJsonBuffer jsonBuffer;
                JsonObject &json = jsonBuffer.parseObject(buf.get());
                json.printTo(Serial);
                if (json.success())
                {
                    Serial.println("\nparsed json");

                    if (json.containsKey("hostname"))
                        strncpy(host_name, json["hostname"], 40);
                    if (json.containsKey("passcode"))
                        strncpy(passcode, json["passcode"], 40);
                    if (json.containsKey("port_str"))
                    {
                        strncpy(port_str, json["port_str"], 20);
                        port = atoi(json["port_str"]);
                    }
                }
                else
                {
                    Serial.println("failed to load json config");
                }
            }
        }
    }
    else
    {
        Serial.println("failed to mount FS");
    }
    // end read

    WiFiManagerParameter custom_hostname("hostname", "Choose a hostname to this IR Controller", host_name, 40);
    wifiManager.addParameter(&custom_hostname);
    WiFiManagerParameter custom_passcode("passcode", "Choose a passcode", passcode, 40);
    wifiManager.addParameter(&custom_passcode);
    WiFiManagerParameter custom_port("port_str", "Choose a port", port_str, 40);
    wifiManager.addParameter(&custom_port);

    // fetches ssid and pass and tries to connect
    // if it does not connect it starts an access point with the specified name
    // and goes into a blocking loop awaiting configuration
    if (!wifiManager.autoConnect(wifi_config_name))
    {
        Serial.println("failed to connect and hit timeout");
        // reset and try again, or maybe put it to deep sleep
        ESP.reset();
        delay(1000);
    }

    // if you get here you have connected to the WiFi
    strncpy(host_name, custom_hostname.getValue(), 40);
    strncpy(passcode, custom_passcode.getValue(), 40);
    strncpy(port_str, custom_port.getValue(), 20);
    port = atoi(port_str);

    if (port != 80)
    {
        Serial.println("Default port changed");
        server = ESP8266WebServer(port);
    }

    Serial.println("WiFi connected! User chose hostname '" + String(host_name) + String("' passcode '") + String(passcode) + "' and port '" + String(port_str) + "'");

    // save the custom parameters to FS
    if (shouldSaveConfig)
    {
        Serial.println(" config...");
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.createObject();
        json["hostname"] = host_name;
        json["passcode"] = passcode;
        json["port_str"] = port_str;

        File configFile = SPIFFS.open("/config.json", "w");
        if (!configFile)
        {
            Serial.println("failed to open config file for writing");
        }

        json.printTo(Serial);
        Serial.println("");
        json.printTo(configFile);
        configFile.close();
    }
    ticker1.detach();

    // keep LED on
    digitalWrite(ledpin, LOW);
    return true;
}

void connectWiFI()
{
    WiFi.hostname(host_name);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    wifi_set_sleep_type(LIGHT_SLEEP_T);
    digitalWrite(ledpin, LOW);
    // Turn off the led in 2s
    ticker1.attach(2, disableLed);

    // Configure mDNS
    if (MDNS.begin(host_name))
    {
        Serial.println("mDNS started. Hostname is set to " + String(host_name) + ".local");
    }
    Serial.print("Local IP: ");
    Serial.println(ipToString(WiFi.localIP()));
    MDNS.addService("http", "tcp", port); // Announce the ESP as an HTTP service
    Serial.println("URL to send commands: http://" + String(host_name) + ".local:" + port_str);
}

//+=============================================================================
// Send header HTML
//
void sendHeader(int httpcode)
{
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(httpcode, "text/html; charset=utf-8", "");
    server.sendContent("<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN' 'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'>\n");
    server.sendContent("<html xmlns='http://www.w3.org/1999/xhtml' xml:lang='en'>\n");
    server.sendContent("  <head>\n");
    server.sendContent("    <meta name='viewport' content='width=device-width, initial-scale=.75' />\n");
    server.sendContent("    <link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css' />\n");
    server.sendContent("    <style>@media (max-width: 991px) {.nav-pills>li {float: none; margin-left: 0; margin-top: 5px; text-align: center;}}</style>\n");
    server.sendContent("    <title>ESP8266 IR Controller (" + String(host_name) + ")</title>\n");
    server.sendContent("  </head>\n");
    server.sendContent("  <body>\n");
    server.sendContent("    <div class='container'>\n");
    server.sendContent("      <h1><a href='https://github.com/mdhiggins/ESP8266-HTTP-IR-Blaster'>ESP8266 IR Controller</a></h1>\n");
    server.sendContent("      <div class='row'>\n");
    server.sendContent("        <div class='col-md-12'>\n");
    server.sendContent("          <ul class='nav nav-pills'>\n");
    server.sendContent("            <li class='active'>\n");
    server.sendContent("              <a href='http://" + String(host_name) + ".local" + ":" + String(port) + "'>Hostname <span class='badge'>" + String(host_name) + ".local" + ":" + String(port) + "</span></a></li>\n");
    server.sendContent("            <li class='active'>\n");
    server.sendContent("              <a href='http://" + ipToString(WiFi.localIP()) + ":" + String(port) + "'>Local <span class='badge'>" + ipToString(WiFi.localIP()) + ":" + String(port) + "</span></a></li>\n");
    server.sendContent("            <li class='active'>\n");
    server.sendContent("              <a href='#'>MAC <span class='badge'>" + String(WiFi.macAddress()) + "</span></a></li>\n");
    server.sendContent("          </ul>\n");
    server.sendContent("        </div>\n");
    server.sendContent("      </div><hr />\n");
}

void sendHeader()
{
    sendHeader(200);
}

//+=============================================================================
// Send footer HTML
//
void sendFooter()
{
    server.sendContent("      <div class='row'><div class='col-md-12'><em>" + String(millis()) + "ms uptime</em></div></div>\n");
    server.sendContent("    </div>\n");
    server.sendContent("  </body>\n");
    server.sendContent("</html>\n");
    server.client().stop();
}

//+=============================================================================
// Stream home page HTML
//
void sendHomePage(String message, String header, int type, int httpcode)
{
    sendHeader(httpcode);
    if (type == 1)
        server.sendContent("      <div class='row'><div class='col-md-12'><div class='alert alert-success'><strong>" + header + "!</strong> " + message + "</div></div></div>\n");
    if (type == 2)
        server.sendContent("      <div class='row'><div class='col-md-12'><div class='alert alert-warning'><strong>" + header + "!</strong> " + message + "</div></div></div>\n");
    if (type == 3)
        server.sendContent("      <div class='row'><div class='col-md-12'><div class='alert alert-danger'><strong>" + header + "!</strong> " + message + "</div></div></div>\n");
    sendFooter();
}

void sendHomePage(String message, String header, int type)
{
    sendHomePage(message, header, type, 200);
}

void sendHomePage(String message, String header)
{
    sendHomePage(message, header, 0);
}

void sendHomePage()
{
    sendHomePage("", "");
}

void setupWiFiServer(IRsend &irsend)
{    
    // Configure the server
    server.on("/json", [&irsend]() { // JSON handler for more complicated IR blaster routines
        Serial.println("Connection received - JSON");

        DynamicJsonBuffer jsonBuffer;
        JsonArray &root = jsonBuffer.parseArray(server.arg("plain"));

        int simple = 0;
        if (server.hasArg("simple"))
            simple = server.arg("simple").toInt();

        if (!root.success())
        {
            Serial.println("JSON parsing failed");
            if (simple)
            {
                server.send(400, "text/plain", "JSON parsing failed");
            }
            else
            {
                sendHomePage("JSON parsing failed", "Error", 3, 400); // 400
            }
        }
        else if (server.arg("pass") != passcode)
        {
            Serial.println("Unauthorized access");
            if (simple)
            {
                server.send(401, "text/plain", "Unauthorized, invalid passcode");
            }
            else
            {
                sendHomePage("Invalid passcode", "Unauthorized", 3, 401); // 401
            }
        }
        else
        {
            digitalWrite(ledpin, LOW);
            ticker1.attach(0.5, disableLed);

            // Handle device state limitations for the global JSON command request
            if (server.hasArg("device"))
            {
                String device = server.arg("device");
                Serial.println("Device name detected " + device);
                int state = (server.hasArg("state")) ? server.arg("state").toInt() : 0;
                if (deviceState.containsKey(device))
                {
                    Serial.println("Contains the key!");
                    Serial.println(state);
                    int currentState = deviceState[device];
                    Serial.println(currentState);
                    if (state == currentState)
                    {
                        if (simple)
                        {
                            server.send(200, "text/html", "Not sending command to " + device + ", already in state " + state);
                        }
                        else
                        {
                            sendHomePage("Not sending command to " + device + ", already in state " + state, "Warning", 2); // 200
                        }
                        Serial.println("Not sending command to " + device + ", already in state " + state);
                        return;
                    }
                    else
                    {
                        Serial.println("Setting device " + device + " to state " + state);
                        deviceState[device] = state;
                    }
                }
                else
                {
                    Serial.println("Setting device " + device + " to state " + state);
                    deviceState[device] = state;
                }
            }

            if (simple)
            {
                server.send(200, "text/html", "Success, code sent");
            }

            String message = "Code sent";

            for (int x = 0; x < root.size(); x++)
            {
                String type = root[x]["type"];
                String ip = root[x]["ip"];
                int rdelay = root[x]["rdelay"];
                int pulse = root[x]["pulse"];
                int pdelay = root[x]["pdelay"];
                int repeat = root[x]["repeat"];
                int out = root[x]["out"];

                if (pulse <= 0)
                    pulse = 1; // Make sure pulse isn't 0
                if (repeat <= 0)
                    repeat = 1; // Make sure repeat isn't 0
                if (pdelay <= 0)
                    pdelay = 100; // Default pdelay
                if (rdelay <= 0)
                    rdelay = 1000; // Default rdelay

                // Handle device state limitations on a per JSON object basis
                String device = root[x]["device"];
                if (device != "")
                {
                    int state = root[x]["state"];
                    if (deviceState.containsKey(device))
                    {
                        int currentState = deviceState[device];
                        if (state == currentState)
                        {
                            Serial.println("Not sending command to " + device + ", already in state " + state);
                            message = "Code sent. Some components of the code were held because device was already in appropriate state";
                            continue;
                        }
                        else
                        {
                            Serial.println("Setting device " + device + " to state " + state);
                            deviceState[device] = state;
                        }
                    }
                    else
                    {
                        Serial.println("Setting device " + device + " to state " + state);
                        deviceState[device] = state;
                    }
                }

                if (type == "delay")
                {
                    delay(rdelay);
                }
                else if (type == "raw")
                {
                    JsonArray &raw = root[x]["data"]; // Array of unsigned int values for the raw signal
                    int khz = root[x]["khz"];
                    if (khz <= 0)
                        khz = 38; // Default to 38khz if not set
                    rawblast(raw, khz, rdelay, pulse, pdelay, repeat, irsend);
                }
                else
                {
                    String data = root[x]["data"];
                    long address = root[x]["address"];
                    int len = root[x]["length"];
                    irblast(type, data, len, rdelay, pulse, pdelay, repeat, address, irsend);
                }
            }

            if (!simple)
            {
                Serial.println("Sending home page");
                sendHomePage(message, "Success", 1); // 200
            }
        }
    });

    // Setup simple msg server to mirror version 1.0 functionality
    server.on("/msg", [&irsend]() {
        Serial.println("Connection received - MSG");
        int simple = 0;
        if (server.hasArg("simple"))
            simple = server.arg("simple").toInt();

        if (server.arg("pass") != passcode)
        {
            Serial.println("Unauthorized access");
            if (simple)
            {
                server.send(401, "text/plain", "Unauthorized, invalid passcode");
            }
            else
            {
                sendHomePage("Invalid passcode", "Unauthorized", 3, 401); // 401
            }
        }
        else
        {
            digitalWrite(ledpin, LOW);
            ticker1.attach(0.5, disableLed);
            String type = server.arg("type");
            String data = server.arg("data");
            String ip = server.arg("ip");

            // Handle device state limitations
            if (server.hasArg("device"))
            {
                String device = server.arg("device");
                Serial.println("Device name detected " + device);
                int state = (server.hasArg("state")) ? server.arg("state").toInt() : 0;
                if (deviceState.containsKey(device))
                {
                    Serial.println("Contains the key!");
                    Serial.println(state);
                    int currentState = deviceState[device];
                    Serial.println(currentState);
                    if (state == currentState)
                    {
                        if (simple)
                        {
                            server.send(200, "text/html", "Not sending command to " + device + ", already in state " + state);
                        }
                        else
                        {
                            sendHomePage("Not sending command to " + device + ", already in state " + state, "Warning", 2); // 200
                        }
                        Serial.println("Not sending command to " + device + ", already in state " + state);
                        return;
                    }
                    else
                    {
                        Serial.println("Setting device " + device + " to state " + state);
                        deviceState[device] = state;
                    }
                }
                else
                {
                    Serial.println("Setting device " + device + " to state " + state);
                    deviceState[device] = state;
                }
            }

            int len = server.arg("length").toInt();
            long address = (server.hasArg("address")) ? server.arg("address").toInt() : 0;
            int rdelay = (server.hasArg("rdelay")) ? server.arg("rdelay").toInt() : 1000;
            int pulse = (server.hasArg("pulse")) ? server.arg("pulse").toInt() : 1;
            int pdelay = (server.hasArg("pdelay")) ? server.arg("pdelay").toInt() : 100;
            int repeat = (server.hasArg("repeat")) ? server.arg("repeat").toInt() : 1;
            int out = (server.hasArg("out")) ? server.arg("out").toInt() : 0;
            if (server.hasArg("code"))
            {
                String code = server.arg("code");
                char separator = ':';
                data = getValue(code, separator, 0);
                type = getValue(code, separator, 1);
                len = getValue(code, separator, 2).toInt();
            }

            if (simple)
            {
                server.send(200, "text/html", "Success, code sent");
            }

            irblast(type, data, len, rdelay, pulse, pdelay, repeat, address, irsend);
            
            if (!simple)
            {
                sendHomePage("Code Sent", "Success", 1); // 200
            }
        }
    });

    server.on("/", []() {
        Serial.println("Connection received");
        sendHomePage(); // 200
    });

    server.begin();
    Serial.println("HTTP Server started on port " + String(port));
}