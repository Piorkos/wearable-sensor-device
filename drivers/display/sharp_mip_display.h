#ifndef SHARP_MIP_DISPLAY_H
#define SHARP_MIP_DISPLAY_H


#include <bitset>
#include <algorithm> // for std::reverse
#include "hardware/spi.h"
#include "hardware/gpio.h"

#include "display.h"
#include "fonts.h"

class SharpMipDisplay : public Display
{
public:
    SharpMipDisplay(uint8_t width, uint8_t height, spi_inst_t *spi, uint display_cs_pin);
    // ~SharpMipDisplay() override;

    /**
     * @brief Updates screen buffer (array) with given text. The text is put in the screen buffer at given position.
     * 
     * @param x column, position at which the text starts. It is the number of columns (screen_width_in_pixels/8), NOT pixels.
     * @param y row, position at which the text starts.
     * @param new_string string which needs to be put in screen buffer on given position.
     */
    void DrawLineOfText(uint16_t x, uint16_t y, const std::string& new_string) override;

    /**
     * @brief Sends new pixel values to the screen. It updates all lines between line_start and line_end.
     * 
     * @param line_start number of the first row which should be updated.
     * @param line_end number of the last row which should be updated.
     */
    void RefreshScreen(uint8_t line_start, uint8_t line_end) override;

    /**
     * @brief Clears the screen.
     * 
     */
    void ClearScreen() override;

    /**
     * @brief Toggles the state of VCOM. Sharp MIP requires to toggle VCOM at least once per second. 
     * 
     */
    void ToggleVCOM();

private:
    uint8_t SwapBigToLittleEndian(uint8_t big_endian);

    /**
     * @brief Helper function to print array of pixels in the terminal. Used only during debugging.
     * 
     * @param array_to_print 
     * @param width 
     * @param heigth 
     */
    void PrintBinaryArray(const uint8_t* array_to_print, size_t width, size_t heigth);


    spi_inst_t *kSPI_;
    const uint kDisplaySpiCsPin_;
    bool vcom_bool_{false};

    const uint8_t kScreenWidthInWords_ = kScreenWidth_ / 8;
    uint8_t* screen_buffer_ = new uint8_t[kScreenWidthInWords_ * kScreenHeight_]{};
};




#endif // SHARP_MIP_DISPLAY_H