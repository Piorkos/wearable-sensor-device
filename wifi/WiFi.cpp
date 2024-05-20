#include <string.h>
#include <stdio.h>

#include "pico/time.h"

#include "utility/wifi_drv.h"
#include "WiFi.h"



WiFiClass::WiFiClass() : _timeout(50000), _feed_watchdog_func(0) 
{
}

int WiFiClass::begin(const char* ssid, const char *passphrase)
{
	printf("WiFiClass::begin \n");

	uint8_t status = WL_IDLE_STATUS;

    // set passphrase
    if (WiFiDrv::wifiSetPassphrase(ssid, strlen(ssid), passphrase, strlen(passphrase))!= WL_FAILURE)
    {
		for (uint32_t start = to_ms_since_boot(get_absolute_time()); (to_ms_since_boot(get_absolute_time()) - start) < _timeout;)
		{
			feedWatchdog();
			sleep_ms(WL_DELAY_START_CONNECTION);
			status = WiFiDrv::getConnectionStatus();
			if ((status != WL_IDLE_STATUS) && (status != WL_NO_SSID_AVAIL) && (status != WL_SCAN_COMPLETED)) 
			{
				break;
			}
		}
    }
	else
	{
    	status = WL_CONNECT_FAILED;
    }
    return status;
}

void WiFiClass::setFeedWatchdogFunc(FeedHostProcessorWatchdogFuncPointer func)
{
    printf("WiFiClass::setFeedWatchdogFunc \n");
	
	_feed_watchdog_func = func;
}

void WiFiClass::feedWatchdog()
{
    printf("WiFiClass::feedWatchdog \n");
	
	if (_feed_watchdog_func)
		_feed_watchdog_func();
}

WiFiClass WiFi;