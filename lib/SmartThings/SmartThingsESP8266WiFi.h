//*******************************************************************************
//	SmartThings Arduino ESP8266 Wifi Library
//
//	License
//	(C) Copyright 2017 Dan Ogorchock
//
//	History
//	2017-02-05  Dan Ogorchock  Created
//*******************************************************************************

#ifndef __SMARTTHINGSESP8266WIFI_H__
#define __SMARTTHINGSESP8266WIFI_H__

#include "SmartThingsEthernet.h"

//*******************************************************************************
// Using ESP8266 WiFi
//*******************************************************************************
#include <ESP8266WiFi.h>

namespace st
{
	class SmartThingsESP8266WiFi: public SmartThingsEthernet
	{
	private:
		//ESP8266 WiFi Specific
		char st_ssid[50];
		char st_password[50];
		boolean st_preExistingConnection = false;
		WiFiServer st_server; //server
		WiFiClient st_client; //client

	public:

		//*******************************************************************************
		/// @brief  SmartThings ESP8266 WiFi Constructor - Pre-existing connection
		///   @param[in] serverPort - TCP/IP Port of the Arduino
		///   @param[in] hubIP - TCP/IP Address of the ST Hub
		///   @param[in] hubPort - TCP/IP Port of the ST Hub
		///   @param[in] callout - Set the Callout Function that is called on Msg Reception
		///   @param[in] shieldType (optional) - Set the Reported SheildType to the Server
		///   @param[in] enableDebug (optional) - Enable internal Library debug
		//*******************************************************************************
		SmartThingsESP8266WiFi(uint16_t serverPort, IPAddress hubIP, uint16_t hubPort, SmartThingsCallout_t *callout, String shieldType = "ESP8266Wifi", bool enableDebug = false, int transmitInterval = 100);

		//*******************************************************************************
		/// Destructor
		//*******************************************************************************
		~SmartThingsESP8266WiFi();

		//*******************************************************************************
		/// Initialize SmartThingsESP8266WiFI Library
		//*******************************************************************************
		virtual void init(void);

		//*******************************************************************************
		/// Run SmartThingsESP8266WiFI Library
		//*******************************************************************************
		virtual void run(void);

		//*******************************************************************************
		/// Send Message to the Hub
		//*******************************************************************************
		virtual void send(String message);

	};
}
#endif
