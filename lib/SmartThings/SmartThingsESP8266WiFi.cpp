//*******************************************************************************
//	SmartThings NodeMCU ESP8266 Wifi Library
//
//	License
//	(C) Copyright 2017 Dan Ogorchock
//
//	History
//	2017-02-10  Dan Ogorchock  Created
//*******************************************************************************

#include "SmartThingsESP8266WiFi.h"

namespace st
{

	//*******************************************************************************
	// SmartThingsESP8266WiFI Constructor - DHCP
	//*******************************************************************************
	SmartThingsESP8266WiFi::SmartThingsESP8266WiFi(uint16_t serverPort, IPAddress hubIP, uint16_t hubPort, SmartThingsCallout_t *callout, String shieldType, bool enableDebug, int transmitInterval) :
		SmartThingsEthernet(serverPort, hubIP, hubPort, callout, shieldType, enableDebug, transmitInterval, true),
		st_server(serverPort)
	{
		st_preExistingConnection = true;
	}

	//*****************************************************************************
	//SmartThingsESP8266WiFI::~SmartThingsESP8266WiFI()
	//*****************************************************************************
	SmartThingsESP8266WiFi::~SmartThingsESP8266WiFi()
	{

	}

	//*******************************************************************************
	/// Initialize SmartThingsESP8266WiFI Library
	//*******************************************************************************
	void SmartThingsESP8266WiFi::init(void)
	{
		while (WiFi.status() != WL_CONNECTED) {
			Serial.print(F("."));
			delay(500);	// wait for connection:
		}

		Serial.println();

		Serial.println(F(""));
		Serial.println(F("Enter the following three lines of data into ST App on your phone!"));
		Serial.print(F("localIP = "));
		Serial.println(WiFi.localIP());
		Serial.print(F("serverPort = "));
		Serial.println(st_serverPort);
		Serial.print(F("MAC Address = "));
		Serial.println(WiFi.macAddress());
		Serial.println(F(""));
		Serial.print(F("hubIP = "));
		Serial.println(st_hubIP);
		Serial.print(F("hubPort = "));
		Serial.println(st_hubPort);
		Serial.println(F(""));
		Serial.println(F("SmartThingsESP8266WiFI: Intialized"));
		Serial.println(F(""));

		//Turn off Wirelss Access Point
		Serial.println(F("Disabling ESP8266 WiFi Access Point"));
		Serial.println(F(""));
		WiFi.mode(WIFI_STA);

		st_server.on("/", [&]() {
			if (_isDebugEnabled)
			{
				Serial.println(F("**** Received connection ***"));
				Serial.println(st_server.arg("plain"));
			}

			String message;

			if(st_server.hasArg("device")) {
				message = st_server.arg("device") + " " + st_server.arg("plain");
			}
			else if(st_server.hasArg("refresh")) {
				message = "refresh";
			}

			_calloutFunction(message);

			st_server.send(200, "text/html", "Success");
		});

		st_server.begin();
	}

	//*****************************************************************************
	// Run SmartThingsESP8266WiFI Library
	//*****************************************************************************
	void SmartThingsESP8266WiFi::run(void)
	{
		if (WiFi.isConnected() == false)
		{
			if (_isDebugEnabled)
			{
				Serial.println(F("**********************************************************"));
				Serial.println(F("**** WiFi Disconnected.  ESP8266 should auto-reconnect ***"));
				Serial.println(F("**********************************************************"));
			}

			//init();
		}

		st_server.handleClient();
	}

	//*******************************************************************************
	/// Send Message out over Ethernet to the Hub
	//*******************************************************************************
	void SmartThingsESP8266WiFi::send(String message)
	{
		if (WiFi.status() != WL_CONNECTED)
		{
			if (_isDebugEnabled)
			{
				Serial.println(F("**********************************************************"));
				Serial.println(F("**** WiFi Disconnected.  ESP8266 should auto-reconnect ***"));
				Serial.println(F("**********************************************************"));
			}

			//init();
		}

		//Make sure the client is stopped, to free up socket for new conenction
		st_server.client().stop();

		if (st_server.client().connect(st_hubIP, st_hubPort))
		{
			st_server.client().println(F("POST / HTTP/1.1"));
			st_server.client().print(F("HOST: "));
			st_server.client().print(st_hubIP);
			st_server.client().print(F(":"));
			st_server.client().println(st_hubPort);
			st_server.client().println(F("CONTENT-TYPE: text"));
			st_server.client().print(F("CONTENT-LENGTH: "));
			st_server.client().println(message.length());
			st_server.client().println();
			st_server.client().println(message);
		}
		else
		{
			//connection failed;
			if (_isDebugEnabled)
			{
				Serial.println(F("***********************************************************"));
				Serial.println(F("***** SmartThings.send() - Ethernet Connection Failed *****"));
				Serial.println(F("***********************************************************"));
				Serial.print(F("hubIP = "));
				Serial.print(st_hubIP);
				Serial.print(F(" "));
				Serial.print(F("hubPort = "));
				Serial.println(st_hubPort);

				Serial.println(F("***********************************************************"));
				Serial.println(F("**** WiFi Disconnected.  ESP8266 should auto-reconnect ****"));
				Serial.println(F("***********************************************************"));
			}

			//init();      //Re-Init connection to get things working again

			if (_isDebugEnabled)
			{
				Serial.println(F("***********************************************************"));
				Serial.println(F("******        Attempting to resend missed data      *******"));
				Serial.println(F("***********************************************************"));
			}


			st_server.client().flush();
			st_server.client().stop();
			if (st_server.client().connect(st_hubIP, st_hubPort))
			{
				st_server.client().println(F("POST / HTTP/1.1"));
				st_server.client().print(F("HOST: "));
				st_server.client().print(st_hubIP);
				st_server.client().print(F(":"));
				st_server.client().println(st_hubPort);
				st_server.client().println(F("CONTENT-TYPE: text"));
				st_server.client().print(F("CONTENT-LENGTH: "));
				st_server.client().println(message.length());
				st_server.client().println();
				st_server.client().println(message);
			}

		}

		//if (_isDebugEnabled) { Serial.println(F("WiFi.send(): Reading for reply data "));}
		// read any data returned from the POST
		while (st_server.client().connected()) {
			//while (st_client.available()) {
			char c = st_server.client().read(); //gets byte from ethernet buffer
									   //if (_isDebugEnabled) { Serial.print(c); } //prints byte to serial monitor
									   //}
		}

		delay(1);
		st_server.client().stop();
	}

}
