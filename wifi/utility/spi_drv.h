#ifndef SPI_DRV_H
#define SPI_DRV_H

#include <inttypes.h>

#define NO_LAST_PARAM   0
#define LAST_PARAM      1

#define DUMMY_DATA  0xFF

#define WAIT_FOR_SLAVE_SELECT()	      \
	if (!SpiDrv::initialized) {           \
		SpiDrv::begin();      \
	}                             \
	SpiDrv::spiSlaveSelect();
    // SpiDrv::waitForSlaveReady();  \
	// SpiDrv::spiSlaveSelect();


class SpiDrv
{
private:
	static void getParam(uint8_t* param);
    static int transfer(uint8_t buf_to_write, uint8_t buf_to_read = 11);

public:
	static bool initialized;

    static void begin();
    static void spiSlaveSelect();
	static void spiSlaveDeselect();
    static void waitForSlaveReady(bool const feed_watchdog = false);
    static int waitSpiChar(unsigned char waitChar);
    static int readAndCheckChar(char checkChar, char* readChar);
    static char readChar();
    static int waitResponseCmd(uint8_t cmd, uint8_t numParam, uint8_t* param, uint8_t* param_len);
	static void sendParam(uint8_t* param, uint8_t param_len, uint8_t lastParam = NO_LAST_PARAM);
    static uint8_t readParamLen8(uint8_t* param_len = NULL);
    static void sendParam(uint16_t param, uint8_t lastParam = NO_LAST_PARAM);
    static void sendCmd(uint8_t cmd, uint8_t numParam);
};



#endif  // SPI_DRV_H