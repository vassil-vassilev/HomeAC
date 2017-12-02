#include <IRsend.h>

#include <Ticker.h>                                           // For LED status
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <aws\AWS.h>
#include <HTTPServer.h>
#include <WiFiUdp.h>

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

  if (getTime) timeClient.begin(); // Get the time

  setupWiFiServer(irsend);

  irsend.begin();
  Serial.println("Ready to send IR signals");

  setupAWS();
}

void loop() {
  handleClient();
  
  if (getTime) timeClient.update();                               // Update the time

  digitalWrite(ledpin, LOW);                                    // Turn on the LED for 0.5 seconds
  ticker.attach(0.5, disableLed);

  delay(200);

  if(WiFi.status() != WL_CONNECTED){
    Serial.println(WiFi.status());
    WiFi.disconnect();
    
    if (!setupWifi(digitalRead(configpin) == LOW))
      ESP.reset();

    connectWiFI();
  }
}
