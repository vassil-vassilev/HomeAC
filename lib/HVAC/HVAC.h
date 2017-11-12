#ifndef HVAC
#define HVAC
    #include <Arduino.h>
    #include <IRsend.h>

    void ir_hvac_command(String dataStr, IRsend irsend);

#endif