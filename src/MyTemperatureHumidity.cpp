#include "MyTemperatureHumidity.h"

namespace st
{
//private
	

//public
	//constructor - called in your sketch's global variable declaration section
	MyTemperatureHumidity::MyTemperatureHumidity(const __FlashStringHelper *name, unsigned int interval, int offset, byte digitalInputPin, IRsend _irsend, String strTemp, String strHumid, bool In_C, byte filterConstant) :
		PS_TemperatureHumidity(name, interval, offset, digitalInputPin, strTemp, strHumid, In_C, filterConstant),
		irsend(_irsend)
	{

	}
	
	//destructor
	MyTemperatureHumidity::~MyTemperatureHumidity()
	{
		
	}

	//SmartThings Shield data handler (receives configuration data from ST - polling interval, and adjusts on the fly)
	void MyTemperatureHumidity::beSmart(const String &str)
	{
		String s = str.substring(str.indexOf(' ') + 1);

		if (s.toInt() != 0) {
			st::PollingSensor::setInterval(s.toInt() * 1000);
			if (st::PollingSensor::debug) {
				Serial.print(F("MyTemperatureHumidity::beSmart set polling interval to "));
				Serial.println(s.toInt());
			}
		}
		else {
			DynamicJsonBuffer jsonBuffer;
			JsonArray &root = jsonBuffer.parseArray(s);

			if (!root.success())
			{
				Serial.println("JSON parsing failed");
				Serial.println(s);
			}
			else
			{
				Serial.println("JSON parsed");
				Serial.println(s);

				for (int x = 0; x < root.size(); x++)
				{
					String type = root[x]["type"];
					String ip = root[x]["ip"];
					int rdelay = root[x]["rdelay"];
					int pulse = root[x]["pulse"];
					int pdelay = root[x]["pdelay"];
					int repeat = root[x]["repeat"];
					int out = root[x]["out"];
					String data = root[x]["data"];
					long address = root[x]["address"];
					int len = root[x]["length"];

					if (pulse <= 0)
                    	pulse = 1; // Make sure pulse isn't 0
                	if (repeat <= 0)
                    	repeat = 1; // Make sure repeat isn't 0
                	if (pdelay <= 0)
                    	pdelay = 100; // Default pdelay
                	if (rdelay <= 0)
                    	rdelay = 1000; // Default rdelay

					irblast(type, data, len, rdelay, pulse, pdelay, repeat, address, irsend);
				}
			}
		}
	}
}