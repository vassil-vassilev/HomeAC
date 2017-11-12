#include "..\config\ConnectionParams.h"
#include "MqttClient.h"
#include "AwsIotSigv4.h"
#include "ESP8266DateTimeProvider.h"
#include <ArduinoJson.h>

ESP8266DateTimeProvider dtp;
AwsIotSigv4 sigv4(&dtp);
ConnectionParams cp(sigv4);
WebSocketClientAdapter adapter(cp);
MqttClient client(adapter, cp);

void setupAWS()
{
    int res = client.connect();
    Serial.printf("mqtt connect=%d\n", res);

    if (res == 0)
    {
        client.subscribe("/device/livingroom/dht", 1,
                         [](const char *topic, const char *msg) { Serial.printf("Got msg on topic %s - %s\n", msg, topic); });
    }
}

void sendAWSData(float t, float h, float hic)
{
    if (client.isConnected())
    {
        Serial.println("Memory3: ");
        Serial.println(ESP.getFreeHeap());

        char result[200];

        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.createObject();
        json["temperature"] = t;
        json["humidity"] = h;
        json["heatindex"] = hic;

        json.printTo(result);

        client.publish("/device/livingroom/dht", result, 0, false);
        client.yield();

        Serial.println("Memory3: ");
        Serial.println(ESP.getFreeHeap());
    }
    else
    {
        Serial.println("Not connected...");
        // reconnect
        setupAWS();
    }
}