#include <stdio.h>
#include "spi_drv.h"
#include "spi_drv.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/time.h"

#include "wifi_spi.h"
#include "../WiFi.h"

// From NANO_RP2040_CONNECT/pins_arduino.h
#define PINS_COUNT      (30u)
#define NINA_RESETN		(3u)
#define NINA_GPIO0		(2u)
#define SPIWIFI_SS		(9u)
#define SPIWIFI_ACK		(10u)
#define SPIWIFI_RESET	(NINA_RESETN)
#define SPIWIFI 		spi1

#define SPI1_CIPO       (8u)
#define SPI1_COPI       (11u)
#define SPI1_SCK        (14u)


static uint8_t SLAVESELECT = SPIWIFI_SS; // ss
static uint8_t SLAVEREADY  = SPIWIFI_ACK;  // handshake pin
static uint8_t SLAVERESET  = SPIWIFI_RESET;  // reset pin
static bool inverted_reset = false;

bool SpiDrv::initialized = false;

void SpiDrv::begin()
{
    printf("SpiDrv::begin() 2 \n");

    gpio_set_dir(SLAVESELECT, GPIO_OUT);
    gpio_set_dir(SLAVEREADY, GPIO_IN);
    gpio_set_dir(SLAVERESET, GPIO_OUT);
    gpio_set_dir(NINA_GPIO0, GPIO_OUT);

    gpio_put(NINA_GPIO0, 1);
    gpio_put(SLAVESELECT, 1);
    gpio_put(SLAVERESET, 0);
    sleep_ms(10);
    gpio_put(SLAVERESET, 1);
    sleep_ms(750);

    gpio_put(NINA_GPIO0, 0);
    gpio_set_dir(NINA_GPIO0, GPIO_IN);

    // initialize SPI
    spi_init(SPIWIFI, 8000000);
    spi_set_format(SPIWIFI, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(SPI1_COPI, GPIO_FUNC_SPI);
    gpio_set_function(SPI1_SCK, GPIO_FUNC_SPI);

    gpio_init(SPIWIFI_SS);
    gpio_set_dir(SPIWIFI_SS, GPIO_OUT);
    gpio_put(SPIWIFI_SS, 1);
    sleep_ms(10);

    initialized = true;
}


#define waitSlaveReady() (gpio_get(SLAVEREADY) == 0)

void SpiDrv::spiSlaveSelect()
{
    printf("SpiDrv::spiSlaveSelect \n");

    gpio_put(SLAVESELECT, 0);

    // wait for up to 5 ms for the NINA to indicate it is not ready for transfer
    // the timeout is only needed for the case when the shield or module is not present
    uint32_t start_timestamp = to_ms_since_boot(get_absolute_time());
    while(!waitSlaveReady() && (to_ms_since_boot(get_absolute_time()) - start_timestamp) < 5)
    {
        printf("SpiDrv::spiSlaveSelect - wait for Slave Ready \n");   
    }
}

void SpiDrv::spiSlaveDeselect()
{
    printf("SpiDrv::spiSlaveDeselect \n");

    gpio_put(SLAVESELECT, 1);
}

int SpiDrv::waitSpiChar(unsigned char waitChar)
{
    printf("SpiDrv::waitSpiChar - waitChar = %u \n", waitChar);

    int timeout = TIMEOUT_CHAR;
    unsigned char _readChar = 0;
    do{
        printf("SpiDrv::waitSpiChar - do: timeout = %i, _readChar = %u  \n", timeout, _readChar);
        _readChar = readChar(); //get data byte
        if (_readChar == ERR_CMD)
        {
        	printf("Err cmd received\n");
        	return -1;
        }
    }while((timeout-- > 0) && (_readChar != waitChar));
    
    printf("SpiDrv::waitSpiChar - return = %b \n", (_readChar == waitChar));

    return  (_readChar == waitChar);
}

int SpiDrv::readAndCheckChar(char checkChar, char* readChar)
{
    printf("SpiDrv::readAndCheckChar \n");

    getParam((uint8_t*)readChar);

    return  (*readChar == checkChar);
}

char SpiDrv::readChar()
{
    printf("SpiDrv::readChar \n");
	uint8_t readChar = 6;
	getParam(&readChar);
    printf("SpiDrv::readChar - readChar = %u \n", readChar);
	return readChar;
}

#define WAIT_START_CMD(x) waitSpiChar(START_CMD)

#define IF_CHECK_START_CMD(x)                      \
    if (!WAIT_START_CMD(_data))                 \
    {                                           \
        printf("Error waiting START_CMD \n");        \
        return 0;                               \
    }else                                       \

#define CHECK_DATA(check, x)                   \
        if (!readAndCheckChar(check, &x))   \
        {                                               \
            printf("Reply error \n");                        \
            return 0;                                   \
        }else                                           \

#define waitSlaveSign() (digitalRead(SLAVEREADY) == HIGH)
#define waitSlaveSignalH() while(digitalRead(SLAVEREADY) != HIGH){}
#define waitSlaveSignalL() while(digitalRead(SLAVEREADY) != LOW){}

// Ta funkcja wywołuje jedynie feedWatchdog(), więc jeśli nie uzywam watchdoga, to prawdopodobnie mogę to usunąć.
void SpiDrv::waitForSlaveReady(bool const feed_watchdog)
{
    printf("SpiDrv::waitForSlaveReady \n");
    uint32_t start_timestamp = to_ms_since_boot(get_absolute_time());
	while (!waitSlaveReady())
    {
        printf("SpiDrv::waitForSlaveReady - waitSlaveReady = 0 \n");
        if (feed_watchdog) 
        {
            printf("SpiDrv::waitForSlaveReady - feed_watchdog = 1 \n");
            uint32_t now_timestamp = to_ms_since_boot(get_absolute_time());
            if ((now_timestamp - start_timestamp) < 10000) 
            {
                WiFi.feedWatchdog();
            }
        }
    }
}

void SpiDrv::getParam(uint8_t* param)
{
    printf("SpiDrv::getParam \n");
    // Get Params data
    int response{0};
    uint8_t data_to_write{DUMMY_DATA};
    // response = spi_read_blocking(SPIWIFI, DUMMY_DATA, param, 1);
    response = spi_write_read_blocking(SPIWIFI, &data_to_write, param, 1);
    printf("SpiDrv::getParam - response = %i, param = %u \n", response, *param);
}

int SpiDrv::waitResponseCmd(uint8_t cmd, uint8_t numParam, uint8_t* param, uint8_t* param_len)
{
    printf("SpiDrv::waitResponseCmd - cmd = %u, numParam = %u, param = %u, param_len = %u \n", cmd, numParam, *param, *param_len);
    
    char _data = 0;
    int ii = 0;

    IF_CHECK_START_CMD(_data)
    {
        printf("SpiDrv::waitResponseCmd - IF_CHECK_START_CMD = false \n");
    
        CHECK_DATA(cmd | REPLY_FLAG, _data){};

        CHECK_DATA(numParam, _data)
        {
            readParamLen8(param_len);
            for (ii=0; ii<(*param_len); ++ii)
            {
                // Get Params data
                //param[ii] = spiTransfer(DUMMY_DATA);
                getParam(&param[ii]);
            } 
        }         

        readAndCheckChar(END_CMD, &_data);
    }     
    
    return 1;
}

void SpiDrv::sendParam(uint8_t* param, uint8_t param_len, uint8_t lastParam)
{
    printf("SpiDrv::sendParam (3) - param = %u, param_len = %u, lastParam = %u \n", *param, param_len, lastParam);
    // Send param len
    uint8_t buf_to_write{param_len};
    transfer(buf_to_write);

    // Send SPI param data
    int i = 0;
    for (i=0; i<param_len; ++i)
    {
        printf("SpiDrv::sendParam (3) - 2. param = %u \n", param[i]);
        buf_to_write = param[i];
        transfer(buf_to_write);
    }

    // if lastParam==1 Send SPI END CMD
    if (lastParam == 1)
    {
        printf("SpiDrv::sendParam (3) - 3. END_CMD = %u \n", END_CMD);
        buf_to_write = END_CMD;
        transfer(buf_to_write);
    }
}

uint8_t SpiDrv::readParamLen8(uint8_t* param_len)
{
    printf("SpiDrv::readParamLen8 \n");

    uint8_t _param_len[1];
    int response{0};
    response = spi_read_blocking(SPIWIFI, DUMMY_DATA, _param_len, 1);
    printf("SpiDrv::getParam - response = %i, param = %u \n", response, *_param_len);
    if (param_len != NULL)
    {
        printf("SpiDrv::readParamLen8 - param_len!= NULL \n");

        *param_len = _param_len[0];
    }
    return _param_len[0];
}

void SpiDrv::sendParam(uint16_t param, uint8_t lastParam)
{
    printf("SpiDrv::sendParam (2) \n");

    uint8_t buf_to_write{2};
    transfer(buf_to_write);
    buf_to_write = (uint8_t)((param & 0xff00)>>8);
    transfer(buf_to_write);
    buf_to_write = (uint8_t)(param & 0xff);
    transfer(buf_to_write);

    if(lastParam == 1)
    {
        printf("SpiDrv::sendParam (2) - lastParam = 1 \n");
        buf_to_write = END_CMD;
        transfer(buf_to_write);
    }
}

/* Cmd Struct Message */
/* _________________________________________________________________________________  */
/*| START CMD | C/R  | CMD  |[TOT LEN]| N.PARAM | PARAM LEN | PARAM  | .. | END CMD | */
/*|___________|______|______|_________|_________|___________|________|____|_________| */
/*|   8 bit   | 1bit | 7bit |  8bit   |  8bit   |   8bit    | nbytes | .. |   8bit  | */
/*|___________|______|______|_________|_________|___________|________|____|_________| */

void SpiDrv::sendCmd(uint8_t cmd, uint8_t numParam)
{
    printf("SpiDrv::sendCmd - cmd = %u, numParam = %u \n", cmd, numParam);

    uint8_t buf_to_write{START_CMD};
    transfer(buf_to_write);
    buf_to_write = cmd & ~(REPLY_FLAG);
    transfer(buf_to_write);
    buf_to_write = numParam;
    transfer(buf_to_write);

    if(numParam == 0)
    {
        printf("SpiDrv::sendCmd - END_CMD \n");
        buf_to_write = END_CMD;
        transfer(buf_to_write);
    }
}

int SpiDrv::transfer(uint8_t buf_to_write, uint8_t buf_to_read)
{
    printf("SpiDrv::transfer - buf_to_write = %u, buf_to_read = %u \n", buf_to_write, buf_to_read);
    int response{0};
    response = spi_write_read_blocking(SPIWIFI, &buf_to_write, &buf_to_read, 1);
    printf("SpiDrv::transfer - response = %u, buf_to_read = %u \n", response, buf_to_read);

    return response;
}