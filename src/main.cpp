#include <IRsend.h>
//#include <IRrecv.h>

#include <Ticker.h>                                           // For LED status
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <DHT.h>
#include <aws\AWS.h>
#include <HTTPServer.h>
#include <WiFiUdp.h>


//+=============================================================================
// Sensor definitions
//
// #define DHTPIN 5     // what digital pin the DHT22 is conected to
// #define DHTTYPE DHT22   // there are multiple kinds of DHT sensors

// DHT dht(DHTPIN, DHTTYPE);

int timeSinceLastRead = 0;

//+=============================================================================

const int configpin = 10;                                     // GPIO10
const int ledpin = BUILTIN_LED;                               // Built in LED defined for WEMOS people

Ticker ticker;

bool holdReceive = false;                                     // Flag to prevent IR receiving while transmitting

//int pinr1 = 14;                                               // Receiving pin
int pins1 = 4;                                                // Transmitting preset 1

//IRrecv irrecv(pinr1);
IRsend irsend(pins1);

const unsigned long resetfrequency = 259200000;                // 72 hours in milliseconds
const int timeOffset = -14400;                                 // Timezone offset in seconds
const char* poolServerName = "time.nist.gov";

const bool getTime = true;                                     // Set to false to disable querying for the time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, poolServerName, timeOffset, (int)resetfrequency);


//+=============================================================================
// Turn off the Led after timeout
//
/* void disableLed()
{
  digitalWrite(ledpin, HIGH);                           // Shut down the LED
  ticker.detach();                                      // Stopping the ticker
} */


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
  //irrecv.enableIRIn();
  Serial.println("Ready to send and receive IR signals");

  setupAWS();
}

void loop() {
  handleClient();
  
  //decode_results  results;                                        // Somewhere to store the results
  
  if (getTime) timeClient.update();                               // Update the time

  /* if (irrecv.decode(&results) && !holdReceive) {                  // Grab an IR code
    if(encoding(&results) != "UNKNOWN") {
      Serial.println("Signal received:");
      fullCode(&results);                                           // Print the singleline value
      dumpCode(&results);                                           // Output the results as source code
      Serial.println("");                                           // Blank line between entries
    }
    irrecv.resume();                                              // Prepare for the next value
    digitalWrite(ledpin, LOW);                                    // Turn on the LED for 0.5 seconds
    ticker.attach(0.5, disableLed);
  } */

  digitalWrite(ledpin, LOW);                                    // Turn on the LED for 0.5 seconds
  ticker.attach(0.5, disableLed);

  //+=============================================================================
  // Sensor Code
  //
  
  // if(timeSinceLastRead > 2000) {
  //   // Reading temperature or humidity takes about 250 milliseconds!
  //   // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  //   float h = dht.readHumidity();
  //   // Read temperature as Celsius (the default)
  //   float t = dht.readTemperature();

  //   // Check if any reads failed and exit early (to try again).
  //   if (isnan(h) || isnan(t)) {
  //     Serial.println("Failed to read from DHT sensor!");
  //     timeSinceLastRead = 0;
  //     return;
  //   }

  //   // Compute heat index in Celsius (isFahreheit = false)
  //   float hic = dht.computeHeatIndex(t, h, false);

  //   Serial.print("Humidity: ");
  //   Serial.print(h);
  //   Serial.print(" %\t");
  //   Serial.print("Temperature: ");
  //   Serial.print(t);
  //   Serial.print(" *C\t");
  //   Serial.print("Heat index: ");
  //   Serial.print(hic);
  //   Serial.println(" *C");

  //   sendAWSData(t, h, hic);

  //   timeSinceLastRead = 0;
  // }
  // timeSinceLastRead += 100;
  
  //
  // End of Sensor Code
  //+=============================================================================

  delay(200);

  if(WiFi.status() != WL_CONNECTED){
    Serial.println(WiFi.status());
    WiFi.disconnect();
    
    if (!setupWifi(digitalRead(configpin) == LOW))
      ESP.reset();

    connectWiFI();
  }
}
