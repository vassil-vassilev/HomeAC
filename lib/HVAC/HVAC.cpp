#include <Arduino.h>
#include <ArduinoJson.h>
#include <IRsend.h>

#define HVAC_TOSHIBA_HDR_MARK    4400
#define HVAC_TOSHIBA_HDR_SPACE   4300
#define HVAC_TOSHIBA_BIT_MARK    543
#define HVAC_TOSHIBA_ONE_SPACE   1623
#define HVAC_MISTUBISHI_ZERO_SPACE  472
#define HVAC_TOSHIBA_RPT_MARK    440
#define HVAC_TOSHIBA_RPT_SPACE   7048 // Above original iremote limit
#define HVAC_TOSHIBA_DATALEN 9

void ir_hvac_command(String dataStr, IRsend irsend)
{
  Serial.println("Entering HVAC");
  Serial.println(dataStr);
  
  boolean error = false;
  const char* HVAC_Mode_Str;
  const char* HVAC_FanMode_Str;
  int HVAC_Temp = 21;
  int OFF = true;                  // Example false

  uint16_t rawdata[2 + 2*8*HVAC_TOSHIBA_DATALEN + 2];
  byte data[HVAC_TOSHIBA_DATALEN] = { 0xF2, 0x0D, 0x03, 0xFC, 0x01, 0x00, 0x00, 0x00, 0x00 };

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(dataStr);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }

  root.prettyPrintTo(Serial);
  
  HVAC_Mode_Str = root["HVAC_MODE"].as<char*>();
  HVAC_Temp = root["HVAC_TEMP"].as<int>();
  HVAC_FanMode_Str = root["HVAC_FANMODE"].as<char*>();
  OFF = root["OFF"].as<int>();

  Serial.println("HVAC_MODE: ");
  Serial.println(HVAC_Mode_Str);
  Serial.println("HVAC_TEMP: ");
  Serial.println(HVAC_Temp);
  Serial.println("HVAC_FANMODE: ");
  Serial.println(HVAC_FanMode_Str);
  Serial.println("OFF: ");
  Serial.println(OFF);


    if (HVAC_Mode_Str == NULL || !strcmp(HVAC_Mode_Str,"HVAC_HOT")) //default HVAC_HOT
    {
      data[6] = (byte) B00000011;
    } else if (HVAC_Mode_Str && !strcmp(HVAC_Mode_Str,"HVAC_COLD"))
    {
      data[6] = (byte) B00000001;
    } else if (HVAC_Mode_Str && !strcmp(HVAC_Mode_Str,"HVAC_DRY"))
    {
      data[6] = (byte) B00000010;
    } else if (HVAC_Mode_Str && !strcmp(HVAC_Mode_Str,"HVAC_AUTO"))
    {
      data[6] = (byte) B00000000;
    } else
    {
      error = true;
    }

    if (OFF) {
      data[6] = (byte) 0x07; // Turn OFF HVAC
    }

    if (HVAC_FanMode_Str && !strcmp(HVAC_FanMode_Str,"FAN_SPEED_1"))
    {
      data[6] = data[6] | (byte) B01000000;
    } else if (HVAC_FanMode_Str && !strcmp(HVAC_FanMode_Str,"FAN_SPEED_2"))
    {
      data[6] = data[6] | (byte) B01100000;
    } else if (HVAC_FanMode_Str && !strcmp(HVAC_FanMode_Str,"FAN_SPEED_3"))
    {
      data[6] = data[6] | (byte) B10000000;
    } else if (HVAC_FanMode_Str && !strcmp(HVAC_FanMode_Str,"FAN_SPEED_4"))
    {
      data[6] = data[6] | (byte) B10100000;
    } else if (HVAC_FanMode_Str && !strcmp(HVAC_FanMode_Str,"FAN_SPEED_5"))
    {
      data[6] = data[6] | (byte) B11000000;
    } else if (HVAC_FanMode_Str == NULL || !strcmp(HVAC_FanMode_Str,"FAN_SPEED_AUTO")) //default FAN_SPEED_AUTO
    {
      data[6] = data[6] | (byte) B00000000;
    } else if (HVAC_FanMode_Str && !strcmp(HVAC_FanMode_Str,"FAN_SPEED_SILENT"))
    {
      data[6] = data[6] | (byte) B00000000;
    }
    else
    {
      error = true;
    }


       byte Temp;
       if (HVAC_Temp > 30) { Temp = 30;}
       else if (HVAC_Temp < 17) { Temp = 17; }
       else { Temp = HVAC_Temp; };
       data[5] = (byte) Temp - 17<<4;

       data[HVAC_TOSHIBA_DATALEN-1] = 0;
       for (int x = 0; x < HVAC_TOSHIBA_DATALEN - 1; x++) {
        data[HVAC_TOSHIBA_DATALEN-1] = (byte) data[x] ^ data[HVAC_TOSHIBA_DATALEN -1];  // CRC is a simple bits addition
       }

       int i = 0;
       byte mask = 1;

       //header
       rawdata[i++] = HVAC_TOSHIBA_HDR_MARK;
       rawdata[i++] = HVAC_TOSHIBA_HDR_SPACE;

       //data
       for (int b = 0; b < HVAC_TOSHIBA_DATALEN; b++)
       {
         for (mask = 10000000; mask > 0; mask >>= 1) { //iterate through bit mask
           if (data[b] & mask) { // Bit ONE
            rawdata[i++] = HVAC_TOSHIBA_BIT_MARK;
            rawdata[i++] = HVAC_TOSHIBA_ONE_SPACE;
           }
           else { // Bit ZERO
            rawdata[i++] = HVAC_TOSHIBA_BIT_MARK;
            rawdata[i++] = HVAC_MISTUBISHI_ZERO_SPACE;
           }
         }
       }

       //trailer
       rawdata[i++] = HVAC_TOSHIBA_RPT_MARK;
       rawdata[i++] = HVAC_TOSHIBA_RPT_SPACE;

       noInterrupts();
       irsend.sendRaw(rawdata,i,38);
       irsend.sendRaw(rawdata,i,38);
       interrupts();
  
  if (error) {
    Serial.println("IRHVAC:Wrong parameters value for HVAC_Mode and/or HVAC_FanMode");
  }
}