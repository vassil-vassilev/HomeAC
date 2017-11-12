#ifndef HTTPServer
#define HTTPServer
    #include <IRsend.h>
    #include <ESP8266WebServer.h>    

    bool setupWifi(bool resetConf);
    void connectWiFI();
    void setupWiFiServer(IRsend &irsend);
    void handleClient();
    void disableLed();
#endif