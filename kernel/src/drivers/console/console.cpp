//
// Created by 98max on 27/09/2023.
//

#include <drivers/console/console.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::drivers;
using namespace maxOS::drivers::console;

///____ Console ____

Console::Console() {

}

Console::~Console() {

}

/**
 * Get the width of the console in characters
 * @return The width of the console in characters
 */
common::uint16_t Console::getWidth() {
    return 0;
}

/**
 * Get the height of the console in characters
 * @return  The height of the console in characters
 */
common::uint16_t Console::getHeight() {
    return 0;
}

/**
 * Put a character on the console
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @param c The character to put on the console
 */
void Console::putChar(common::uint16_t x, common::uint16_t y, char c) {

}

/**
 * Set the foreground color of a character on the console
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @param foreground The foreground color to set
 */
void Console::setForegroundColor(common::uint16_t x, common::uint16_t y, ConsoleColour foreground) {

}

/**
 * Set the background color of a character on the console
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @param background The background color to set
 */
void Console::setBackgroundColor(common::uint16_t x, common::uint16_t y, ConsoleColour background) {

}

/**
 * Get the character at a given coordinate on the console
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @return The character at the given coordinate
 */
char Console::getChar(common::uint16_t x, common::uint16_t y) {
    return ' ';
}

/**
 * Get the background color of a character on the console
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @return The background color of the character
 */
ConsoleColour Console::getForegroundColor(common::uint16_t x, common::uint16_t y) {
    return Green;
}

/**
 * Get the background color of a character on the console
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @return The background color of the character
 */
ConsoleColour Console::getBackgroundColor(common::uint16_t x, common::uint16_t y) {
    return Green;
}

/**
 * Put a character on the console with a given foreground and background color
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @param c The character to put on the console
 * @param foreground The foreground color of the character
 * @param background The background color of the character
 */
void Console::putChar(common::uint16_t x, common::uint16_t y, char c, ConsoleColour foreground, ConsoleColour background) {

    // Set the colors of the character
    setForegroundColor(x, y, foreground);
    setBackgroundColor(x, y, background);

    // Put the character on the console
    putChar(x, y, c);

}

/**
 * Put a string on the console
 * @param x The x coordinate of the string
 * @param y The y coordinate of the string
 * @param s The string to put on the console
 * @param foreground The foreground color of the string
 * @param background The background color of the string
 */
void Console::putString(common::uint16_t x, common::uint16_t y, common::string s, ConsoleColour foreground, ConsoleColour background) {

    // For each character in the string
    for(const char* si = s; x < getWidth() && *si != '\0'; si++, x++) {

        // Put the character on the console
        putChar(x,y,*si,foreground,background);

    }

}

/**
 * Scroll the console up by 1 line
 */
void Console::scrollUp() {

    // Scroll the console up by 1 line
    scrollUp(0, 0, getWidth(), getHeight());

}

/**
 * Scroll an area of the console up by 1 line
 * @param left The left coordinate of the area to scroll
 * @param top The top coordinate of the area to scroll
 * @param width The width of the area to scroll
 * @param height The height of the area to scroll
 * @param foreground The foreground color of the new line
 * @param background The background color of the new line
 * @param fill The character to fill the new line with
 */
void Console::scrollUp(common::uint16_t left, common::uint16_t top, common::uint16_t width, common::uint16_t height, ConsoleColour foreground, ConsoleColour background, char fill) {

    // For each line in the area to scroll (except the last line)
    for(uint16_t y = top; y < top+height-1; y++){

        // For each character in the line
        for(uint16_t x = left; x < left+width; x++) {

            // Put the character from the line below
            putChar(x, y, getChar(x, y+1), getForegroundColor(x, y+1), getBackgroundColor(x, y+1));

        }
    }

    // For each character in the last line
    for(uint16_t x = left; x < left+width; x++) {

        // Put the fill character
        putChar(x, top+height-1, fill, foreground, background);

    }
}

/**
 * Clear the console
 */
void Console::clear() {

    // Clear the console
    clear(0, 0, getWidth(), getHeight());

}

/**
 * Clear an area of the console
 * @param left The left coordinate of the area to clear
 * @param top The top coordinate of the area to clear
 * @param width The width of the area to clear
 * @param height The height of the area to clear
 * @param foreground The foreground color of the area
 * @param background The background color of the area
 * @param fill The character to fill the area with
 */
void Console::clear(common::uint16_t left, common::uint16_t top, common::uint16_t width, common::uint16_t height, ConsoleColour foreground, ConsoleColour background, char fill) {

    // For each line in the area to clear
    for(uint16_t y = top; y < top+height; y++)
        for(uint16_t x = left; x < left+width; x++){
            // Put the fill character
            putChar(x,y,fill,foreground,background);
        }

}

/**
 * Invert the colors of a character on the console
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 */
void Console::invertColors(common::uint16_t x, common::uint16_t y) {

    // Get the colors of the character
    ConsoleColour foreground = getForegroundColor(x, y);
    ConsoleColour background = getBackgroundColor(x, y);

    // Set the colors of the character
    setForegroundColor(x, y, background);
    setBackgroundColor(x, y, foreground);
}


///____ Console Area ____///


ConsoleArea::ConsoleArea(Console *console, common::uint16_t left, common::uint16_t top, common::uint16_t width, common::uint16_t height)
: console(console), left(left), top(top), width(width), height(height)
{

}

ConsoleArea::ConsoleArea(Console *console, common::uint16_t left, common::uint16_t top, common::uint16_t width, common::uint16_t height, ConsoleColour foreground, ConsoleColour background)
        : console(console), left(left), top(top), width(width), height(height)
{

    // Loop through the area setting the colors
    for(uint16_t y = top; y < top+height; y++)
        for(uint16_t x = left; x < left+width; x++){
            console->setForegroundColor(x,y,foreground);
            console->setBackgroundColor(x,y,background);
        }

}


ConsoleArea::~ConsoleArea() {

}

/**
 * Return the width of the console area
 * @return The width of the console area
 */
common::uint16_t ConsoleArea::getWidth() {
    return width;
}

/**
 * Return the height of the console area
 * @return The height of the console area
 */
common::uint16_t ConsoleArea::getHeight() {
    return height;
}

/**
 * Place a character on the console area if the coordinates are within the area
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @param c The character to put on the console
 */
void ConsoleArea::putChar(common::uint16_t x, common::uint16_t y, char c) {

    // Make sure the coordinates are within the console area
    if(x >= width || y >= height)
        return;

    // Put the character on the console
    console->putChar(left+x, top+y, c);

}

/**
 * Change the foreground color of a character on the console area if the coordinates are within the area
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @param foreground The foreground color of the character
 */
void ConsoleArea::setForegroundColor(common::uint16_t x, common::uint16_t y, ConsoleColour foreground) {

        // Make sure the coordinates are within the console area
        if(x >= width || y >= height)
            return;

        // Set the foreground color of the character
        console -> setForegroundColor(left + x, top + y, foreground);

}

void ConsoleArea::setBackgroundColor(common::uint16_t x, common::uint16_t y, ConsoleColour background) {

    // Make sure the coordinates are within the console area
    if(x >= width || y >= height)
        return;

    // Set the background color of the character
    console -> setBackgroundColor(left + x, top + y, background);

}

/**
 * Return the character at the given coordinates if the coordinates are within the console area
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @return The character at the given coordinates, if the coordinates are within the console area otherwise " "
 */
char ConsoleArea::getChar(common::uint16_t x, common::uint16_t y) {

    // Make sure the coordinates are within the console area
    if(x >= width || y >= height)
        return ' ';

    // Return the character at the given coordinates
    return console->getChar(left+x, top+y);
}

/**
 * Return the foreground color of the character at the given coordinates if the coordinates are within the console area
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @return The foreground color of the character at the given coordinates, if the coordinates are within the console area otherwise ConsoleColour::LightGrey
 */
ConsoleColour ConsoleArea::getForegroundColor(common::uint16_t x, common::uint16_t y) {

    // Make sure the coordinates are within the console area
    if(x >= width || y >= height)
        return ConsoleColour::LightGrey;

    // Return the foreground color of the character at the given coordinates
    return console->getForegroundColor(left+x, top+y);

}

/**
 * Return the background color of the character at the given coordinates if the coordinates are within the console area
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @return The background color of the character at the given coordinates, if the coordinates are within the console area otherwise ConsoleColour::Black
 */
ConsoleColour ConsoleArea::getBackgroundColor(common::uint16_t x, common::uint16_t y) {

    // Make sure the coordinates are within the console area
    if(x >= width || y >= height)
        return ConsoleColour::Black;

    // Return the background color of the character at the given coordinates
    return console->getBackgroundColor(left+x, top+y);
}


///____ Console Stream ____///
ConsoleStream::ConsoleStream(Console *console) {

    // Set the x and y coordinates to 0
    cursorX = 0;
    cursorY = 0;

    // Set the console
    this->console = console;

}

ConsoleStream::~ConsoleStream() {

}

/**
 * Write a character to the console stream
 * @param c The character to write
 */
void ConsoleStream::writeChar(char c) {

    // If the character placement is more than the width of the console go on a new line
    if(cursorX >= console->getWidth()) {

        // Go to the start of the next line
        cursorX = 0;

        // Increment the y coordinate but if it is more than the height of the console scroll the console
        if(++cursorY >= console->getHeight()){

            // Scroll the console
            console->scrollUp();

            // Decrement the y coordinate
            cursorY = console->getHeight()-1;
        }
    }

    // Handle the character
    switch (c) {
        // New line
        case '\n':
            // Increment the y coordinate but if it is more than the height of the console scroll the console
            if(++cursorY >= console->getHeight()){

                // Scroll the console
                console->scrollUp();

                // Decrement the y coordinate
                cursorY = console->getHeight()-1;
            }

            // dont break here, we want to go to the next case because of the \r

        // Carriage return
        case '\r':
            // Go to the start of the next line
            cursorX = 0;
            break;

        // Null Terminator
        case '\0':
            break;

        default:
            // Put the character on the console
            console->putChar(cursorX, cursorY, c);

            // Increment the x coordinate
            cursorX++;
            break;

    }

}

/**
 * Set the position of the cursor
 * @param x The x coordinate of the cursor
 * @param y The y coordinate of the cursor
 */
void ConsoleStream::setCursor(common::uint16_t x, common::uint16_t y) {

    // Set the x and y coordinates
    cursorX = x;
    cursorY = y;
}

