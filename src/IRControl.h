#ifndef IRControl
#define IRControl
    #include <Arduino.h>
    #include <IRsend.h>
    #include <ArduinoJson.h>

    void hvac_send(String data, int pulse, int pdelay, IRsend irsend);
    void irblast(String type, String dataStr, unsigned int len, int rdelay, int pulse, int pdelay, int repeat, long address, IRsend irsend);
    void rawblast(JsonArray &raw, int khz, int rdelay, int pulse, int pdelay, int repeat, IRsend irsend);
#endif