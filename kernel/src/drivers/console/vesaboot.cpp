//
// Created by 98max on 10/2/2023.
//

#include <drivers/console/vesaboot.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::gui;
using namespace maxOS::drivers;
using namespace maxOS::drivers::console;

VESABootConsole::VESABootConsole(common::GraphicsContext *graphicsContext)
: Driver(),
  Console()
{

    // Set the graphics context
    this->graphicsContext = graphicsContext;

    // Set the font
    this->font = AmigaFont();
}

VESABootConsole::~VESABootConsole()
{

}

uint16_t VESABootConsole::getWidth()
{
    return 128;      // 1024 pixels / 8 pixels per character
}

uint16_t VESABootConsole::getHeight()
{
    return 85;       // 768 pixels / 8 pixels per character plus 1 pixel for spacing
}
/**
 * Places a character at the specified location
 * @param x The x coordinate
 * @param y The y coordinate
 * @param c The character to place
 */
void VESABootConsole::putChar(common::uint16_t x, common::uint16_t y, char c) {

    // If the coordinates are out of bounds, return
    if(x >= getWidth() || y >= getHeight())
        return;

    // Calculate the offset
    int offset = (y*getWidth() + x);

    // Set the character at the offset, by masking the character with the current character (last 8 bits)
    videoMemory[offset] = (videoMemory[offset] & 0xFF00) | (uint16_t)c;

    // Convert the char into a string
    string s = " ";
    s[0] = c;

    Colour foreground = consoleColourToVESA(getForegroundColor(x,y));
    Colour background = consoleColourToVESA(getBackgroundColor(x,y));

    // Use the font to draw the character
    font.drawText(x*8, y*9, foreground, background, graphicsContext, s);


}

/**
 * Sets the foreground color at the specified location
 * @param x The x coordinate
 * @param y The y coordinate
 * @param foreground The foreground color
 */
void VESABootConsole::setForegroundColor(common::uint16_t x, common::uint16_t y, ConsoleColour foreground) {

    // If the coordinates are out of bounds, return
    if(x >= getWidth() || y >= getHeight())
        return;

    // Calculate the offset
    int offset = (y*getWidth() + x);

    // Set the foreground color at the offset, by masking the foreground color with the current foreground color (bits 8-11)
    videoMemory[offset] = (videoMemory[offset] & 0xF0FF) | ((uint16_t)foreground << 8);
}

/**
 * Sets the background color at the specified location
 * @param x The x coordinate
 * @param y The y coordinate
 * @param background The background color
 */
void VESABootConsole::setBackgroundColor(common::uint16_t x, common::uint16_t y, ConsoleColour background) {

    // If the coordinates are out of bounds, return
    if(x >= getWidth() || y >= getHeight())
        return;

    // Calculate the offset
    int offset = (y*getWidth() + x);

    // Set the background color at the offset, by masking the background color with the current background color (bits 12-15)
    videoMemory[offset] = (videoMemory[offset] & 0x0FFF) | ((uint16_t)background << 12);

}

/**
 * Gets the character at the specified location
 * @param x The x coordinate
 * @param y The y coordinate
 * @return The character at the specified location
 */
char VESABootConsole::getChar(common::uint16_t x, common::uint16_t y) {

    // If the coordinates are out of bounds, return
    if(x >= getWidth() || y >= getHeight())
        return ' ';

    // Calculate the offset
    int offset = (y*getWidth() + x);

    // Return the character at the offset, by masking the character with the current character (last 8 bits)
    return (char)(videoMemory[offset] & 0x00FF);
}

/**
 * Gets the foreground color at the specified location
 * @param x The x coordinate
 * @param y The y coordinate
 * @return The foreground color at the specified location
 */
ConsoleColour VESABootConsole::getForegroundColor(common::uint16_t x, common::uint16_t y) {

    // If the coordinates are out of bounds, return
    if(x >= getWidth() || y >= getHeight())
        return ConsoleColour::White;

    // Calculate the offset
    int offset = (y*getWidth() + x);

    // Return the foreground color at the offset, by masking the foreground color with the current foreground color (bits 8-11)
    return (ConsoleColour)((videoMemory[offset] & 0x0F00) >> 8);
}

/**
 * Gets the background color at the specified location
 * @param x The x coordinate
 * @param y The y coordinate
 * @return The background color at the specified location
 */
ConsoleColour VESABootConsole::getBackgroundColor(common::uint16_t x, common::uint16_t y) {

    // If the coordinates are out of bounds, return
    if(x >= getWidth() || y >= getHeight())
        return ConsoleColour::Black;

    // Calculate the offset
    int offset = (y*getWidth() + x);

    // Return the background color at the offset, by masking the background color with the current background color (bits 12-15)
    return (ConsoleColour)((videoMemory[offset] & 0xF000) >> 12);
}





common::Colour VESABootConsole::consoleColourToVESA(ConsoleColour colour) {
    switch (colour) {

        case Black:
            return Colour(0, 0, 0);

        case Blue:
            return Colour(0, 0, 255);

        case Green:
            return Colour(0, 255, 0);

        case Cyan:
            return Colour(0, 170, 170);

        case Red:
            return Colour(170, 0, 0);

        case Magenta:
            return Colour(170, 0, 170);

        case Brown:
            return Colour(170, 85, 0);

        case LightGrey:
            return Colour(170, 170, 170);

        case DarkGrey:
            return Colour(85, 85, 85);

        case LightBlue:
            return Colour(85, 85, 255);

        case LightGreen:
            return Colour(85, 255, 85);

        case LightCyan:
            return Colour(85, 255, 255);

        case LightRed:
            return Colour(255, 85, 85);

        case LightMagenta:
            return Colour(255, 85, 255);

        case Yellow:
            return Colour(255, 255, 85);

        case White:
            return Colour(255, 255, 255);
    }
}


