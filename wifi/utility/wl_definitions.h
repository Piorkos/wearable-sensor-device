#ifndef WL_DEFINITIONS_H_
#define WL_DEFINITIONS_H_

// // Maximum size of a SSID
// #define WL_SSID_MAX_LENGTH 32
// // Length of passphrase. Valid lengths are 8-63.
// #define WL_WPA_KEY_MAX_LENGTH 63
// // Length of key in bytes. Valid values are 5 and 13.
// #define WL_WEP_KEY_MAX_LENGTH 13
// // Size of a MAC-address or BSSID
// #define WL_MAC_ADDR_LENGTH 6
// // Size of a MAC-address or BSSID
// #define WL_IPV4_LENGTH 4
// // Maximum size of a SSID list
// #define WL_NETWORKS_LIST_MAXNUM	10
// // Maximum number of socket
// #define	WIFI_MAX_SOCK_NUM	10
// // Socket not available constant
// #define SOCK_NOT_AVAIL  255
// // Default state value for WiFi state field
// #define NA_STATE -1

typedef enum {
    WL_NO_SHIELD = 255,
    WL_NO_MODULE = WL_NO_SHIELD,
    WL_IDLE_STATUS = 0,
    WL_NO_SSID_AVAIL,
    WL_SCAN_COMPLETED,
    WL_CONNECTED,
    WL_CONNECT_FAILED,
    WL_CONNECTION_LOST,
    WL_DISCONNECTED,
    WL_AP_LISTENING,
    WL_AP_CONNECTED,
    WL_AP_FAILED
} wl_status_t;



#endif  // WL_DEFINITIONS_H_