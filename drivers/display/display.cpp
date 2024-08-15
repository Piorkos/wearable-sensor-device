#include "display.h"

Display::Display(uint8_t width, uint8_t height)
: kScreenWidth_{width}, kScreenHeight_{height}
{
    std::cout << "Display Contrustor" << "\n";
}

Display::~Display() = default;