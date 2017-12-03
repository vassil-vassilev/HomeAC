//*******************************************************************************
//	SmartThings Arduino Ethernet Library 
//
//	License
//	(C) Copyright 2017 Dan Ogorchock
//
//	History
//	2017-02-04  Dan Ogorchock  Created
//  2017-05-02  Dan Ogorchock  Add support for W5500 Ethernet2 Shield
//*******************************************************************************

#ifndef __SMARTTHINGSETHERNET_H__ 
#define __SMARTTHINGSETHERNET_H__

#include "SmartThings.h"

//*******************************************************************************
// Using Ethernet Shield
//*******************************************************************************
#include <IPAddress.h>
/*
//#if defined ARDUINO_ARCH_AVR
//#include <SPI.h>
//#include <Ethernet.h>
//#elif defined ARDUINO_ARCH_ESP8266
//#include <ESP8266WiFi.h>
//#endif
*/
namespace st
{
	class SmartThingsEthernet: public SmartThings
	{
	private:

	protected:
		IPAddress st_hubIP;
		uint16_t st_serverPort;
		uint16_t st_hubPort;
		bool st_DHCP;

	public:

		//*******************************************************************************
		/// @brief  SmartThings Ethernet Constructor - DHCP
		///   @param[in] serverPort - TCP/IP Port of the Arduino
		///   @param[in] hubIP - TCP/IP Address of the ST Hub
		///   @param[in] hubPort - TCP/IP Port of the ST Hub
		///   @param[in] callout - Set the Callout Function that is called on Msg Reception
		///   @param[in] shieldType (optional) - Set the Reported SheildType to the Server 
		///   @param[in] enableDebug (optional) - Enable internal Library debug
		//*******************************************************************************
		SmartThingsEthernet(uint16_t serverPort, IPAddress hubIP, uint16_t hubPort, SmartThingsCallout_t *callout, String shieldType = "EthernetShield", bool enableDebug = false, int transmitInterval = 100, bool DHCP = true);

		//*******************************************************************************
		/// Destructor 
		//*******************************************************************************
		~SmartThingsEthernet();

		//*******************************************************************************
		/// Initialize SmartThings Library 
		//*******************************************************************************
		virtual void init(void) = 0; //all derived classes must implement this pure virtual function

		//*******************************************************************************
		/// Run SmartThings Library 
		//*******************************************************************************
		virtual void run(void) = 0; //all derived classes must implement this pure virtual function

		//*******************************************************************************
		/// Send Message to the Hub 
		//*******************************************************************************
		virtual void send(String message) = 0; //all derived classes must implement this pure virtual function


	};
}
#endif
