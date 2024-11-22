#ifndef CONFIG_H
#define CONFIG_H

#include <string>

// *****************************
// Select used hardware platform
// *****************************
// BOARD
#define ARDUINO_NANO_RP2040
// #define RP_PI_PICO
// DISPLAY
#define DISPLAY_SHARP_MIP
// #define DISPLAY_SSD1306
// #define DISPLAY_SH1106
// GYROSCOPE AND ACCELEROMETER


#if defined(ARDUINO_NANO_RP2040)
namespace config
{
    // ---I2C
    inline constexpr uint kI2C_0_sda_pin{12};
    inline constexpr uint kI2C_0_scl_pin{13};
    // inline constexpr uint kI2C_0_sda_pin{0};
    // inline constexpr uint kI2C_0_scl_pin{1};
    inline constexpr uint kI2C_1_sda_pin{18};
    inline constexpr uint kI2C_1_scl_pin{15};   
    // ---SPI
    inline constexpr uint kSPI_sck_pin{26};          // SCLK / SCK
    inline constexpr uint kSPI_mosi_pin{27};         // MOSI / COPI
    inline constexpr uint kSPI_cs_pin{28};            // SS / CS
    // ---BUTTONS
    inline constexpr uint kButton_left_pin{29};        // left
    inline constexpr uint kButton_right_pin{6};        // right
}
#elif defined(RP_PI_PICO)
namespace config
{
    // ---I2C
    inline constexpr uint kI2C_1_sda_pin{2};
    inline constexpr uint kI2C_1_scl_pin{3};
    inline constexpr uint kI2C_0_sda_pin{16};
    inline constexpr uint kI2C_0_scl_pin{17};
    // ---SPI - Confirm pins!!!
    inline constexpr uint kSPI_sck_pin{6};          // SCLK / SCK
    inline constexpr uint kSPI_mosi_pin{7};         // MOSI / COPI
    inline constexpr uint kSPI_cs_pin{5};           // SS / CS
    // ---BUTTONS
    inline constexpr uint kButton_left_pin{14};        // right top
    inline constexpr uint kButton_right_pin{15};        // right bottom
}
#else
#error "Select hardware platform: board not selected"
#endif


#if defined(DISPLAY_SHARP_MIP)
namespace config
{
    inline constexpr uint8_t kWidth{144};
    inline constexpr uint8_t kHeight{168};
}
#elif defined(DISPLAY_SSD1306)
namespace config
{
    inline constexpr uint8_t kWidth{128};
    inline constexpr uint8_t kHeight{64};
    inline constexpr uint16_t kOledAddress{0x3C};
}
#elif defined(DISPLAY_SH1106)
namespace config
{
    inline constexpr uint8_t kWidth{128};
    inline constexpr uint8_t kHeight{64};
    inline constexpr uint16_t kOledAddress{0x3C};
}
#else
#error "Select hardware platform: display not selected"
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

// std::string state_id_names[12] = {"Init",
//     "Standby",
//     "GpsSearch",
//     "GpsReady",
//     "Training",
//     "StopTraining",
//     "ReadData",
//     "ReadingInProgress",
//     "EraseData",
//     "ErasingInProgress",
//     "Return",
//     "Error"
//     };


#endif  // CONFIG_H