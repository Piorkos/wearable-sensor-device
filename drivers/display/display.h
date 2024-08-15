#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdlib.h>
#include <iostream>
#include <string>

class Display
{
public:
    Display(uint8_t width, uint8_t height);
    virtual ~Display();
    virtual void DrawLineOfText(uint16_t x, uint16_t y, const std::string& new_string) = 0;
    virtual void RefreshScreen(uint8_t line_start, uint8_t line_end) = 0;
    virtual void ClearScreen() = 0;

protected:
    const uint8_t kScreenWidth_;
    const uint8_t kScreenHeight_;
};

#endif // DISPLAY_H
