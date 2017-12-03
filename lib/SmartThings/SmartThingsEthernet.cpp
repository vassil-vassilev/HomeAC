//*******************************************************************************
//	SmartThings Arduino Ethernet Library 
//
//	License
//	(C) Copyright 2017 Dan Ogorchock
//
//	History
//	2017-02-04  Dan Ogorchock  Created
//*******************************************************************************

#include "SmartThingsEthernet.h"

namespace st
{
	//*******************************************************************************
	// SmartThingsEthernet Constructor - DHCP
	//*******************************************************************************
	SmartThingsEthernet::SmartThingsEthernet(uint16_t serverPort, IPAddress hubIP, uint16_t hubPort, SmartThingsCallout_t *callout, String shieldType, bool enableDebug, int transmitInterval, bool DHCP) :
		SmartThings(callout, shieldType, enableDebug, transmitInterval),
		st_hubIP(hubIP),
		st_serverPort(serverPort),
		st_hubPort(hubPort),
		st_DHCP(DHCP)
	{

	}

	//*****************************************************************************
	//SmartThingsEthernet::~SmartThingsEthernet()
	//*****************************************************************************
	SmartThingsEthernet::~SmartThingsEthernet()
	{

	}

}
