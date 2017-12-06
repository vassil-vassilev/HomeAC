#include <Arduino.h>
#include <ArduinoJson.h>
#include <IRsend.h>
#include <HVAC.h>

//+=============================================================================
// Convert string to hex, borrowed from ESPBasic
//
unsigned long HexToLongInt(String h)
{
    // this function replace the strtol as this function is not able to handle hex numbers greather than 7fffffff
    // I'll take char by char converting from hex to char then shifting 4 bits at the time
    int i;
    unsigned long tmp = 0;
    unsigned char c;
    int s = 0;
    h.toUpperCase();
    for (i = h.length() - 1; i >= 0; i--)
    {
        // take the char starting from the right
        c = h[i];
        // convert from hex to int
        c = c - '0';
        if (c > 9)
            c = c - 7;
        // add and shift of 4 bits per each char
        tmp += c << s;
        s += 4;
    }
    return tmp;
}

void hvac_send(String data, int pulse, int pdelay, IRsend irsend)
{
    Serial.print("Sending HVAC data ");
    Serial.println(data);
    //holdReceive = true;
    Serial.println("Blocking incoming IR signals");

    for (int i = 0; i < pulse; i++)
    {
        ir_hvac_command(data, irsend);
        delay(pdelay);
    }

    //resetReceive();
}

//+=============================================================================
// Send IR codes to variety of sources
//
void irblast(String type, String dataStr, unsigned int len, int rdelay, int pulse, int pdelay, int repeat, long address, IRsend irsend)
{
    Serial.println("Blasting off");
    type.toLowerCase();
    unsigned long data = HexToLongInt(dataStr);
    //holdReceive = true;
    Serial.println("Blocking incoming IR signals");
    // Repeat Loop
    for (int r = 0; r < repeat; r++)
    {
        // Pulse Loop
        for (int p = 0; p < pulse; p++)
        {
            Serial.print(data, HEX);
            Serial.print(":");
            Serial.print(type);
            Serial.print(":");
            Serial.println(len);
            if (type == "nec")
            {
                irsend.sendNEC(data, len);
            }
            else if (type == "sony")
            {
                irsend.sendSony(data, len);
            }
            else if (type == "coolix")
            {
                irsend.sendCOOLIX(data, len);
            }
            else if (type == "whynter")
            {
                irsend.sendWhynter(data, len);
            }
            else if (type == "panasonic")
            {
                Serial.println(address);
                irsend.sendPanasonic(address, data);
            }
            else if (type == "jvc")
            {
                irsend.sendJVC(data, len, 0);
            }
            else if (type == "samsung")
            {
                irsend.sendSAMSUNG(data, len);
            }
            else if (type == "sharpRaw")
            {
                irsend.sendSharpRaw(data, len);
            }
            else if (type == "dish")
            {
                irsend.sendDISH(data, len);
            }
            else if (type == "rc5")
            {
                irsend.sendRC5(data, len);
            }
            else if (type == "rc6")
            {
                irsend.sendRC6(data, len);
            }
            else if (type == "denon")
            {
                irsend.sendDenon(data, len);
            }
            else if (type == "lg")
            {
                irsend.sendLG(data, len);
            }
            else if (type == "sharp")
            {
                irsend.sendSharpRaw(data, len);
            }
            else if (type == "rcmm")
            {
                irsend.sendRCMM(data, len);
            }
            else if (type == "hvac")
            {
                hvac_send(dataStr, pulse, pdelay, irsend);
            }
            if (p + 1 < pdelay)
                delay(pdelay);
        }
        if (r + 1 < rdelay)
            delay(rdelay);
    }

    Serial.println("Transmission complete");

    //resetReceive();
}