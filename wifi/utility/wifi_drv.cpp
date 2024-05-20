#include <string.h>
#include <stdio.h>

#include "wifi_drv.h"
#include "spi_drv.h"

#include "wifi_spi.h"
#include "wl_types.h"

int8_t WiFiDrv::wifiSetPassphrase(const char* ssid, uint8_t ssid_len, const char *passphrase, const uint8_t len)
{
    printf("WiFiDrv::wifiSetPassphrase \n");

	WAIT_FOR_SLAVE_SELECT();
    // Send Command
    SpiDrv::sendCmd(SET_PASSPHRASE_CMD, PARAM_NUMS_2);
    SpiDrv::sendParam((uint8_t*)ssid, ssid_len, NO_LAST_PARAM);
    SpiDrv::sendParam((uint8_t*)passphrase, len, LAST_PARAM);

    // pad to multiple of 4
    int commandSize = 6 + ssid_len + len;
    while (commandSize % 4) 
    {
        printf("WiFiDrv::wifiSetPassphrase - while commandSize \% 4 \n");
        SpiDrv::readChar();
        commandSize++;
    }

    SpiDrv::spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv::waitForSlaveReady();
    SpiDrv::spiSlaveSelect();

    // Wait for reply
    uint8_t _data = 0;
    uint8_t _dataLen = 0;
    if (!SpiDrv::waitResponseCmd(SET_PASSPHRASE_CMD, PARAM_NUMS_1, &_data, &_dataLen))
    {
        // WARN("error waitResponse");
        printf("WiFiDrv::wifiSetPassphrase - error waitResponse \n");
        _data = WL_FAILURE;
    }
    SpiDrv::spiSlaveDeselect();
    return _data;
    // return 0;
}

uint8_t WiFiDrv::getConnectionStatus()
{
	WAIT_FOR_SLAVE_SELECT();

    // Send Command
    SpiDrv::sendCmd(GET_CONN_STATUS_CMD, PARAM_NUMS_0);

    SpiDrv::spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv::waitForSlaveReady();
    SpiDrv::spiSlaveSelect();

    // Wait for reply
    uint8_t _data = -1;
    uint8_t _dataLen = 0;
    SpiDrv::waitResponseCmd(GET_CONN_STATUS_CMD, PARAM_NUMS_1, &_data, &_dataLen);

    SpiDrv::spiSlaveDeselect();

    return _data;
}

WiFiDrv wiFiDrv;