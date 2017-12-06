#include <FS.h>                                                 // This needs to be first, or it all crashes and burns

#include <Arduino.h>
#include <WiFiManager.h>                                        // https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>                                        // Useful to access to ESP by hostname.local
#include <Ticker.h>

const int ledpin = BUILTIN_LED;                                 // Built in LED defined for WEMOS people

bool shouldSaveConfig = false; // Flag for saving data
unsigned long lastupdate = 0;
const bool getExternalIP = true; // Set to false to disable querying external IP
String _ip = "";

const char *wifi_config_name = "IR Controller Configuration";
int port = 80;
char passcode[40] = "myhvacp@ssc0de";
char host_name[40] = "blaster";
char port_str[20] = "80";

Ticker ticker1;

DynamicJsonBuffer jsonBuffer;
JsonObject& deviceState = jsonBuffer.createObject();

void setTicker (Ticker _ticker)
{
    ticker1 = _ticker;
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