#include "PS_TemperatureHumidity.h"
#include <ArduinoJson.h>
#include <IRControl.h>
#include <IRsend.h>

namespace st
{
	class MyTemperatureHumidity: public PS_TemperatureHumidity
	{
		private:
			IRsend irsend;

		public:
			//constructor - called in your sketch's global variable declaration section
			MyTemperatureHumidity(const __FlashStringHelper *name, unsigned int interval, int offset, byte digitalInputPin, IRsend _irsend, String strTemp = "temperature1", String strHumid = "humidity1", bool In_C = false, byte filterConstant = 100);
			
			//destructor
			virtual ~MyTemperatureHumidity();

			//SmartThings Shield data handler (receives configuration data from ST - polling interval, and adjusts on the fly)
			virtual void beSmart(const String &str);
	
	};
}