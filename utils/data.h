#ifndef DATA_H
#define DATA_H

#include "hardware/i2c.h"
#include <string>

// *************************
// ---Uncomment below line to build for Arduino Nano RP2040 Connect. 
#define ARDUINO_NANO_RP2040
// ---Uncoment below line to switch to Sharp MIP display
#define SHARP_MIP


#ifdef ARDUINO_NANO_RP2040
namespace config
{
    // ---I2C
    inline constexpr uint kI2C_0_sda_pin{12};
    inline constexpr uint kI2C_0_scl_pin{13};
    // inline constexpr uint kI2C_0_sda_pin{0};
    // inline constexpr uint kI2C_0_scl_pin{1};
    inline constexpr uint kI2C_1_sda_pin{18};
    inline constexpr uint kI2C_1_scl_pin{15};   
    // ---SPI - Confirm pins!!!
    inline constexpr uint kSPI_sck_pin{26};          // SCLK / SCK
    inline constexpr uint kSPI_mosi_pin{27};         // MOSI / COPI
    inline constexpr uint kSPI_cs_pin{28};            // SS / CS
    // inline constexpr uint kSPI_sck_pin{6};          // SCLK / SCK
    // inline constexpr uint kSPI_mosi_pin{7};         // MOSI / COPI
    // inline constexpr uint kSPI_cs_pin{5};            // SS / CS
    // ---BUTTONS
    inline constexpr uint kButton_left_pin{29};        // left
    inline constexpr uint kButton_right_pin{6};        // right
}
#else
namespace config
{
    // ---I2C
    inline constexpr uint kI2C_1_sda_pin{2};
    inline constexpr uint kI2C_1_scl_pin{3};
    inline constexpr uint kI2C_0_sda_pin{4};
    inline constexpr uint kI2C_0_scl_pin{5};
    // ---SPI - Confirm pins!!!
    inline constexpr uint kSPI_sck_pin{6};          // SCLK / SCK
    inline constexpr uint kSPI_mosi_pin{7};         // MOSI / COPI
    inline constexpr uint kSPI_cs_pin{5};           // SS / CS
    // ---BUTTONS
    inline constexpr uint kButton_1_pin{};        // right top
    inline constexpr uint kButton_2_pin{};        // right bottom
}
#endif


#ifdef SHARP_MIP
// ---Sharp display
namespace config
{
    inline constexpr uint8_t kWidth{144};
    inline constexpr uint8_t kHeight{168};
}
#else
// ---OLED display
namespace config
{
    inline constexpr uint8_t kWidth{128};
    inline constexpr uint8_t kHeight{64};
    inline constexpr uint16_t kOledAddress{0x3C};
    inline constexpr uint8_t kSideFontSize{8};
}
#endif


enum StateId
{
    kInit,
    kStandby,
    kGpsSearch,
    kGpsReady,
    kTraining,
    kStopTraining,
    kReadData,
    kReadingInProgress,
    kEraseData,
    kErasingInProgress,
    kReturn,
    kError
};

std::string state_id_names[12] = {"Init",
    "Standby",
    "GpsSearch",
    "GpsReady",
    "Training",
    "StopTraining",
    "ReadData",
    "ReadingInProgress",
    "EraseData",
    "ErasingInProgress",
    "Return",
    "Error"
    };


#endif  // DATA_H