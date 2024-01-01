#ifndef WRITE_H
#define WRITE_H

#include <stdio.h>
#include <string>
#include <iostream>
#include <typeinfo>
#include <bitset>
#include <algorithm> // for std::reverse
#include "fonts.h"
#include "../utils/data.h"

uint8_t SwapBigToLittleEndian(uint8_t big_endian)
{
    std::string string_be = std::bitset<8>(big_endian).to_string();
    std::reverse(string_be.begin(), string_be.end());
    std::bitset<8> bitset_le(string_be);
    uint8_t little_e = static_cast<uint8_t>(bitset_le.to_ulong());
    
    return little_e;
}

namespace sharp_mip
{
    void PrintBinaryArray(const uint8_t* array_to_print, size_t width, size_t heigth);
}

namespace sharp_mip
{
    bool vcom_bool_{false};

    uint8_t screen_cols = config::kWidth / 8;
    uint8_t* screen_state = new uint8_t[screen_cols * config::kHeight]{};

    // uint16_t* screen_state = new uint16_t[40 * 30]{};

    /**
     * @brief Updates screen buffer (array) with given text. The text is put in the screen buffer at given position.
     * 
     * @param x column, position at which the text starts. It is the number of columns (screen_width_in_pixels/8), NOT pixels.
     * @param y row, position at which the text starts.
     * @param new_string string which needs to be put in screen buffer on given position.
     */
    void WriteLine(uint16_t x, uint16_t y, const std::string& new_string)
    {
        printf("--sharp_mip::WriteLine : new_string = %s \n", new_string.c_str());

        // TODO x jest wyrazony w pixelach, natomiast screen_state uzywa byte (8 pixeli) jako jednostke bazowa
        // Uwzglednic to, gdy wartosc z kFont_12_16 jest przypisywana do screen_state
        
        uint8_t font_height_in_bits = 16;   // for kFont_12_16
        uint8_t font_width_in_bytes = 2;    // for kFont_12_16
        char first_char_in_fonts = '0';
        uint16_t char_counter{0};
        for(const auto& character : new_string)
        {
            printf("character = %c \n", character);
            for (uint8_t i = 0; i < font_height_in_bits; ++i)
            {
                for (uint8_t j = 0; j < font_width_in_bytes; j++)
                {
                    int pixel_index{(y + i) * screen_cols + x + char_counter*font_width_in_bytes};
                    int font_index{((character - first_char_in_fonts) * font_height_in_bits + i)*font_width_in_bytes};
                    
                    screen_state[pixel_index + j]= kFont_12_16[font_index + j];
                }
            }
            ++char_counter;
        }

        PrintBinaryArray(screen_state, screen_cols, config::kHeight);
    }

    /**
     * @brief Sends new pixel values to the screen. It updates all lines between line_start and line_end.
     * 
     * @param line_start number of the first row which should be updated.
     * @param line_end number of the last row which should be updated.
     */
    void RefreshScreen(uint8_t line_start, uint8_t line_end)
    {
        printf("--sharp_mip::RefreshScreen \n");

        gpio_put(config::kSPI_cs_pin, 1);

        int length_of_buffer = 1 + (line_end - line_start) * (1 + screen_cols + 1) + 1;
        uint8_t buf[length_of_buffer];
        int buf_iterator{0};
        buf[buf_iterator] = 0b10000000;    // command
        buf_iterator++;

        for (size_t i = line_start; i < line_end; i++)
        {
            uint8_t little_endian_line_address = SwapBigToLittleEndian(i);
            buf[buf_iterator] = little_endian_line_address;    //line address
            buf_iterator++;
            for (size_t j = 0; j < screen_cols; ++j)
            {
                buf[buf_iterator] = screen_state[i * screen_cols + j];
                buf_iterator++;
            }
            buf[buf_iterator] = 0b00000000;     //end line trailer
            buf_iterator++;
        }

        buf[buf_iterator] = 0b00000000;     //end transmission trailer
        spi_write_blocking(spi0, buf, length_of_buffer);
        gpio_put(config::kSPI_cs_pin, 0);
        sleep_ms(10);
    }

    /**
     * @brief Clears the screen.
     * 
     */
    void ClearScreen()
    {
        printf("---ClearScreen \n");

        for(int i = 0; i < (sharp_mip::screen_cols * config::kHeight); ++i) 
        {
            sharp_mip::screen_state[i] = 0b11111111;
        }

        gpio_put(config::kSPI_cs_pin, 1);
        uint8_t buf[2];
        buf[0] = 0b01100000;
        buf[1] = 0b00000000;
        spi_write_blocking(spi0, buf, 2);
        gpio_put(config::kSPI_cs_pin, 0);
        sleep_ms(10);

        gpio_put(config::kSPI_cs_pin, 1);
        buf[0] = 0b00100000;
        buf[1] = 0b00000000;
        spi_write_blocking(spi0, buf, 2);
        gpio_put(config::kSPI_cs_pin, 0);
        sleep_ms(10);
    }

    /**
     * @brief Toggles the state of VCOM. Sharp MIP requires to toggle VCOM at least once per second. 
     * 
     */
    void ToggleVCOM()
    {
        printf("--sharp_mip::ToggleVCOM \n");
        gpio_put(config::kSPI_cs_pin, 1);
        uint8_t buf[22];
        if(vcom_bool_)
        {
            buf[0] = 0b01000000;
            vcom_bool_ = false;
        }
        else
        {
            buf[0] = 0b00000000;
            vcom_bool_ = true;
        }
        buf[1] = 0b00000000;
        spi_write_blocking(spi0, buf, 2);
        gpio_put(config::kSPI_cs_pin, 0);
        sleep_ms(10);
    }

    /**
     * @brief Helper function to print array of pixels in the terminal. Used only during debugging.
     * 
     * @param array_to_print 
     * @param width 
     * @param heigth 
     */
    void PrintBinaryArray(const uint8_t* array_to_print, size_t width, size_t heigth)
    {
        printf("--sharp_mip::PrintBinaryArray\n");
        for (size_t i = 0; i < heigth; i++)     // rows
        {
            for (size_t j = 0; j < width; j++)  // cols
            {
                for (size_t k = 0; k < 8; k++) // word size
                {
                    uint8_t mask = 0b10000000;
                    mask = mask >> k;
                    if(array_to_print[(i*width) + j] & mask)
                    {
                        printf(".");
                    }
                    else
                    {
                        printf("+");
                    }
                }
            }
            printf("\n");
        }
    }
}

#endif  // WRITE_H