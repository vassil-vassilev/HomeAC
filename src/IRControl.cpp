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

void rawblast(JsonArray &raw, int khz, int rdelay, int pulse, int pdelay, int repeat, IRsend irsend)
{
    Serial.println("Raw transmit");
    //holdReceive = true;
    Serial.println("Blocking incoming IR signals");
    // Repeat Loop
    for (int r = 0; r < repeat; r++)
    {
        // Pulse Loop
        for (int p = 0; p < pulse; p++)
        {
            Serial.println("Sending code");
            irsend.enableIROut(khz);
            for (unsigned int i = 0; i < raw.size(); i++)
            {
                int val = raw[i];
                if (i & 1)
                    irsend.space(max(0, val));
                else
                    irsend.mark(val);
            }
            irsend.space(0);
            if (p + 1 < pdelay)
                delay(pdelay);
        }
        if (r + 1 < rdelay)
            delay(rdelay);
    }

    Serial.println("Transmission complete");

    //resetReceive();
}

//+=============================================================================
// Reenable IR receiving
//
/* void resetReceive()
{
    if (holdReceive)
    {
        Serial.println("Reenabling receiving");
        irrecv.resume();
        holdReceive = false;
    }
} */

//+=============================================================================
// Display encoding type
//
// Display encoding type
//
/* String encoding(decode_results *results) {
    String output;
    switch (results->decode_type) {
      default:
      case UNKNOWN:      output = "UNKNOWN";            break;
      case NEC:          output = "NEC";                break;
      case SONY:         output = "SONY";               break;
      case RC5:          output = "RC5";                break;
      case RC6:          output = "RC6";                break;
      case DISH:         output = "DISH";               break;
      case SHARP:        output = "SHARP";              break;
      case JVC:          output = "JVC";                break;
      case SANYO:        output = "SANYO";              break;
      case SANYO_LC7461: output = "SANYO_LC7461";       break;
      case MITSUBISHI:   output = "MITSUBISHI";         break;
      case SAMSUNG:      output = "SAMSUNG";            break;
      case LG:           output = "LG";                 break;
      case WHYNTER:      output = "WHYNTER";            break;
      case AIWA_RC_T501: output = "AIWA_RC_T501";       break;
      case PANASONIC:    output = "PANASONIC";          break;
      case DENON:        output = "DENON";              break;
      case COOLIX:       output = "COOLIX";             break;
    }
    return output;
    if (results->repeat) Serial.print(" (Repeat)");
  } */

//+=============================================================================
// Code to string
//
/* void fullCode (decode_results *results)
  {
    Serial.print("One line: ");
    serialPrintUint64(results->value, 16);
    Serial.print(":");
    Serial.print(encoding(results));
    Serial.print(":");
    Serial.print(results->bits, DEC);
    if (results->overflow)
      Serial.println("WARNING: IR code too long."
                     "Edit IRrecv.h and increase RAWBUF");
    Serial.println("");
  } */

//+=============================================================================
// Dump out the decode_results structure.
//
/* void dumpCode(decode_results *results)
{
    // Start declaration
    Serial.print("uint16_t  ");             // variable type
    Serial.print("rawData[");               // array name
    Serial.print(results->rawlen - 1, DEC); // array size
    Serial.print("] = {");                  // Start declaration

    // Dump data
    for (uint16_t i = 1; i < results->rawlen; i++)
    {
        Serial.print(results->rawbuf[i] * RAWTICK, DEC);
        if (i < results->rawlen - 1)
            Serial.print(","); // ',' not needed on last one
        if (!(i & 1))
            Serial.print(" ");
    }

    // End declaration
    Serial.print("};"); //

    // Comment
    Serial.print("  // ");
    Serial.print(encoding(results));
    Serial.print(" ");
    serialPrintUint64(results->value, 16);

    // Newline
    Serial.println("");

    // Now dump "known" codes
    if (results->decode_type != UNKNOWN)
    {
        // Some protocols have an address &/or command.
        // NOTE: It will ignore the atypical case when a message has been decoded
        // but the address & the command are both 0.
        if (results->address > 0 || results->command > 0)
        {
            Serial.print("uint32_t  address = 0x");
            Serial.print(results->address, HEX);
            Serial.println(";");
            Serial.print("uint32_t  command = 0x");
            Serial.print(results->command, HEX);
            Serial.println(";");
        }

        // All protocols have data
        Serial.print("uint64_t  data = 0x");
        serialPrintUint64(results->value, 16);
        Serial.println(";");
    }
} */

//+=============================================================================
// Dump out the decode_results structure.
//
/* void dumpRaw(decode_results *results)
{
    // Print Raw data
    Serial.print("Timing[");
    Serial.print(results->rawlen - 1, DEC);
    Serial.println("]: ");

    for (uint16_t i = 1; i < results->rawlen; i++)
    {
        if (i % 100 == 0)
            yield(); // Preemptive yield every 100th entry to feed the WDT.
        uint32_t x = results->rawbuf[i] * RAWTICK;
        if (!(i & 1))
        { // even
            Serial.print("-");
            if (x < 1000)
                Serial.print(" ");
            if (x < 100)
                Serial.print(" ");
            Serial.print(x, DEC);
        }
        else
        { // odd
            Serial.print("     ");
            Serial.print("+");
            if (x < 1000)
                Serial.print(" ");
            if (x < 100)
                Serial.print(" ");
            Serial.print(x, DEC);
            if (i < results->rawlen - 1)
                Serial.print(", "); // ',' not needed for last one
        }
        if (!(i % 8))
            Serial.println("");
    }
    Serial.println(""); // Newline
} */

//+=============================================================================
// Dump out the decode_results structure.
//
/*   void dumpInfo(decode_results *results) {
    if (results->overflow)
      Serial.println("WARNING: IR code too long."
                     "Edit IRrecv.h and increase RAWBUF");
  
    // Show Encoding standard
    Serial.print("Encoding  : ");
    Serial.print(encoding(results));
    Serial.println("");
  
    // Show Code & length
    Serial.print("Code      : ");
    serialPrintUint64(results->value, 16);
    Serial.print(" (");
    Serial.print(results->bits, DEC);
    Serial.println(" bits)");
  } */