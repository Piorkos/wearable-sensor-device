#ifndef WIFI_H
#define WIFI_H


#include <inttypes.h>

// extern "C" {
#include "utility/wl_definitions.h"
#include "utility/wl_types.h"
// }
#include "WiFiClient.h"

typedef void(*FeedHostProcessorWatchdogFuncPointer)();

class WiFiClass {
private:
    unsigned long _timeout;
    FeedHostProcessorWatchdogFuncPointer _feed_watchdog_func;

public:
    WiFiClass();

    /* Start WiFi connection with passphrase
     * the most secure supported mode will be automatically selected
     *
     * param ssid: Pointer to the SSID string.
     * param passphrase: Passphrase. Valid characters in a passphrase
     *        must be between ASCII 32-126 (decimal).
     */
    int begin(const char* ssid, const char *passphrase);

    void setFeedWatchdogFunc(FeedHostProcessorWatchdogFuncPointer func);
    void feedWatchdog();
};

extern WiFiClass WiFi;

#endif  // WIFI_H