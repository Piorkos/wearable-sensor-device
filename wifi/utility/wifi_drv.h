#ifndef WIFI_DRV_H
#define WIFI_DRV_H

#include <inttypes.h>


// 100 ms of delay to have the connection established
#define WL_DELAY_START_CONNECTION 100


class WiFiDrv
{
private:

public:
    /* Start WiFi connection with passphrase
     * the most secure supported mode will be automatically selected
     *
     * param ssid: Pointer to the SSID string.
     * param ssid_len: Length of SSID string.
     * param passphrase: Passphrase. Valid characters in a passphrase
     *        must be between ASCII 32-126 (decimal).
     * param len: Length of passphrase string.
     * return: WL_SUCCESS or WL_FAILURE
     */
    static int8_t wifiSetPassphrase(const char* ssid, uint8_t ssid_len, const char *passphrase, const uint8_t len);

    /*
     * Disconnect from the network
     *
     * return: one value of wl_status_t enum
     */
    static uint8_t getConnectionStatus();
};

extern WiFiDrv wiFiDrv;


#endif  // WIFI_DRV_H