#include <IRsend.h>

#include <Ticker.h>                                           // For LED status
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <aws\AWS.h>
#include <HTTPServer.h>
#include <WiFiUdp.h>

#include <Constants.h>       //Constants.h is designed to be modified by the end user to adjust behavior of the ST_Anything library
#include <Device.h>          //Generic Device Class, inherited by Sensor and Executor classes
#include <Sensor.h>          //Generic Sensor Class, typically provides data to ST Cloud (e.g. Temperature, Motion, etc...)
#include <Executor.h>        //Generic Executor Class, typically receives data from ST Cloud (e.g. Switch)
#include <PollingSensor.h>   //Generic Polling "Sensor" Class, polls Arduino pins periodically
#include <Everything.h>      //Master Brain of ST_Anything library that ties everything together and performs ST Shield communications
#include <SmartThingsESP8266WiFi.h>
#include <MyTemperatureHumidity.h>

const int configpin = 10;                                     // GPIO10
const int ledpin = BUILTIN_LED;                               // Built in LED defined for WEMOS people

Ticker ticker;

int pins1 = 4;                                                // Transmitting preset 1

IRsend irsend(pins1);

const unsigned long resetfrequency = 259200000;                // 72 hours in milliseconds
const int timeOffset = -14400;                                 // Timezone offset in seconds
const char* poolServerName = "time.nist.gov";

const bool getTime = true;                                     // Set to false to disable querying for the time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, poolServerName, timeOffset, (int)resetfrequency);


// Smartthings Hub Information
IPAddress hubIp(192, 168, 1, 160);    // smartthings hub ip     //  <---You must edit this line!
const unsigned int hubPort = 39500;   // smartthings hub port

//******************************************************************************************
//st::Everything::callOnMsgSend() optional callback routine.  This is a sniffer to monitor 
//    data being sent to ST.  This allows a user to act on data changes locally within the 
//    Arduino sktech.
//******************************************************************************************
void callback(const String &msg)
{
  Serial.print(F("ST_Anything Callback: Sniffed data = "));
  Serial.println(msg);
  
  //TODO:  Add local logic here to take action when a device's value/state is changed
  
  //Masquerade as the ThingShield to send data to the Arduino, as if from the ST Cloud (uncomment and edit following line)
  //st::receiveSmartString("Put your command here!");  //use same strings that the Device Handler would send
}

void stSetup() {
  static st::MyTemperatureHumidity sensor1(F("temphumid1"), 30, 3, 5, irsend, "temperature1", "humidity1", true);

  st::Everything::debug=true;
  st::Executor::debug=true;
  st::Device::debug=true;
  st::PollingSensor::debug=true;

  //*****************************************************************************
  //Initialize the "Everything" Class
  //*****************************************************************************

  //Initialize the optional local callback routine (safe to comment out if not desired)
  st::Everything::callOnMsgSend = callback;
  
  //Create the SmartThings ESP8266WiFi Communications Object
  //STATIC IP Assignment - Recommended
  st::Everything::SmartThing = new st::SmartThingsESP8266WiFi(8090, hubIp, hubPort, st::receiveSmartString, "ESP8266WiFi", true);
 
    //DHCP IP Assigment - Must set your router's DHCP server to provice a static IP address for this device's MAC address
    //st::Everything::SmartThing = new st::SmartThingsESP8266WiFi(str_ssid, str_password, serverPort, hubIp, hubPort, st::receiveSmartString);

  st::Everything::init();
  st::Everything::addSensor(&sensor1);
  st::Everything::initDevices();
}

//+=============================================================================
// Setup web server and IR receiver/blaster
//
void setup() {
  Serial.println("Demo Started.");

  // Initialize serial
  Serial.begin(115200);
  Serial.println("");
  Serial.println("ESP8266 IR Controller");
  pinMode(configpin, INPUT_PULLUP);
  Serial.print("Config pin GPIO");
  Serial.print(configpin);
  Serial.print(" set to: ");
  Serial.println(digitalRead(configpin));
  if (!setupWifi(digitalRead(configpin) == LOW))
    return;

  connectWiFI();

  stSetup();

  if (getTime) timeClient.begin(); // Get the time

  //setupWiFiServer(irsend);

  irsend.begin();
  Serial.println("Ready to send IR signals");

  //setupAWS();
}

void loop() {  
  if (getTime) timeClient.update();                               // Update the time

  digitalWrite(ledpin, LOW);                                    // Turn on the LED for 0.5 seconds
  ticker.attach(0.5, disableLed);

  st::Everything::run();

  //st::PS_TemperatureHumidity *sensor = (st::PS_TemperatureHumidity*)st::Everything::getDeviceByName("temphumid1");
  //sendAWSData(sensor->getTemperatureSensorValue(), sensor->getHumiditySensorValue(), 0);

  if(WiFi.status() != WL_CONNECTED){
    Serial.println(WiFi.status());
    WiFi.disconnect();
    
    if (!setupWifi(digitalRead(configpin) == LOW))
      ESP.reset();

    connectWiFI();
  }
}
