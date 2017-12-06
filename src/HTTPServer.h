#ifndef HTTPServer
#define HTTPServer
    #include <ESP8266WebServer.h>    

    bool setupWifi(bool resetConf);
    void connectWiFI();
    void disableLed();
#endif