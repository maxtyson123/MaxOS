/**
 * @file console.cpp
 * @brief Implementation of a basic console driver interface
 *
 * @date 27th September 2023
 * @author Max Tyson
 */

#include <drivers/console/console.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::console;

Console::Console() = default;

Console::~Console() = default;

/**
 * @brief Get the width of the console in characters
 *
 * @return The width of the console in characters
 */
uint16_t Console::width() {
    return 0;
}

/**
 * @brief Get the height of the console in characters
 *
 * @return  The height of the console in characters
 */
uint16_t Console::height() {
    return 0;
}

/**
 * @brief Put a character on the console
 *
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @param c The character to put on the console
 */
void Console::put_character(uint16_t, uint16_t, char) {
}

/**
 * @brief Set the foreground color of a character on the console
 *
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @param foreground The foreground color to set
 */
void Console::set_foreground_color(uint16_t, uint16_t, ConsoleColour) {

}

/**
 * @brief Set the background color of a character on the console
 *
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @param background The background color to set
 */
void Console::set_background_color(uint16_t, uint16_t, ConsoleColour) {

}

/**
 * @brief  Get the character at a given coordinate on the console
 *
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @return The character at the given coordinate
 */
char Console::get_character(uint16_t, uint16_t) {
    return ' ';
}

/**
 * @brief Get the background color of a character on the console
 *
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @return The background color of the character
 */
ConsoleColour Console::get_foreground_color(uint16_t, uint16_t) {
    return ConsoleColour::Green;
}

/**
 * @brief Get the background color of a character on the console
 *
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @return The background color of the character
 */
ConsoleColour Console::get_background_color(uint16_t, uint16_t) {
    return ConsoleColour::Green;
}

/**
 * @brief Put a character on the console with a given foreground and background color
 *
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @param c The character to put on the console
 * @param foreground The foreground color of the character
 * @param background The background color of the character
 */
void Console::put_character(uint16_t x, uint16_t y, char c, ConsoleColour foreground, ConsoleColour background) {

    // Set the colors of the character
    set_foreground_color(x, y, foreground);
    set_background_color(x, y, background);

    // Put the character on the console
    put_character(x, y, c);

}

/**
 * @brief Put a string on the console
 *
 * @param x The x coordinate of the string
 * @param y The y coordinate of the string
 * @param string The string to put on the console
 * @param foreground The foreground color of the string
 * @param background The background color of the string
 */
void Console::put_string(uint16_t x, uint16_t y, string string, ConsoleColour foreground, ConsoleColour background) {

    // Print each character on the screen
    for(size_t i = 0; i < string.length(); i++)
        put_character(x + i, y, string[i], foreground, background);
}

/**
 * @brief Scroll the entire console up by 1 line
 */
void Console::scroll_up() {

    scroll_up(0, 0, width(), height());
}

/**
 * @brief Scroll an area of the console up by 1 line
 *
 * @param left The left coordinate of the area to scroll
 * @param top The top coordinate of the area to scroll
 * @param width The m_width of the area to scroll
 * @param height The m_height of the area to scroll
 * @param foreground The foreground color of the new line
 * @param background The background color of the new line
 * @param fill The character to fill the new line with
 */
void Console::scroll_up(uint16_t left, uint16_t top, uint16_t width, uint16_t height, ConsoleColour foreground, ConsoleColour background, char fill) {

    // Shift everything but the last line by getting what is below it
    for(uint16_t y = top; y < top+height-1; y++)
        for(uint32_t x = left; x < left+width; x++)
            put_character(x, y, get_character(x, y + 1), get_foreground_color(x, y + 1), get_background_color(x, y + 1));

    // Fill the last line
    for(uint16_t x = left; x < left+width; x++)
        put_character(x, top + height - 1, fill, foreground, background);

}

/**
 * Clear the entire console
 */
void Console::clear() {

    clear(0, 0, width(), height());

}

/**
 * @brief Clear an area of the console
 *
 * @param left The left coordinate of the area to clear
 * @param top The top coordinate of the area to clear
 * @param width The m_width of the area to clear
 * @param height The m_height of the area to clear
 * @param foreground The foreground color of the area
 * @param background The background color of the area
 * @param fill The character to fill the area with
 */
void Console::clear(uint16_t left, uint16_t top, uint16_t width, uint16_t height, ConsoleColour foreground, ConsoleColour background, char fill) {

    // Check bounds
    if ( left > this -> width() || top > this -> height() || width > this -> width() || height > this -> height() )
        return;

    // Fill the area with the character
    for(uint16_t y = top; y < top + height; y++)
        for(uint16_t x = left; x < left + width; x++)
            put_character(x, y, fill, foreground, background);

}

/**
 * @brief Invert the colors of a character on the console
 *
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 */
void Console::invert_colors(uint16_t x, uint16_t y) {

    // Get the colors of the character
    ConsoleColour foreground = get_foreground_color(x, y);
    ConsoleColour background = get_background_color(x, y);

    // Set the colors of the character
    set_foreground_color(x, y, background);
    set_background_color(x, y, foreground);
}

/**
 * @brief Construct a new Console Area object
 *
 * @param console The console to create the area on
 * @param left How far from the left the area starts
 * @param top How far from the top the area starts
 * @param width The width of the area
 * @param height The height of the area
 */
ConsoleArea::ConsoleArea(Console *console, uint16_t left, uint16_t top, uint16_t width, uint16_t height)
: m_console(console),
  m_left(left),
  m_top(top),
  m_width(width),
  m_height(height)
{

}

/**
 * @brief Construct a new Console Area object with specified foreground and background colors
 *
 * @param console The console to create the area on
 * @param left How far from the left the area starts
 * @param top How far from the top the area starts
 * @param width The width of the area
 * @param height The height of the area
 * @param foreground The foreground color of the area
 * @param background The background color of the area
 */
ConsoleArea::ConsoleArea(Console *console, uint16_t left, uint16_t top, uint16_t width, uint16_t height, ConsoleColour foreground, ConsoleColour background)
: m_console(console),
  m_left(left),
  m_top(top),
  m_width(width),
  m_height(height)
{

    // Store the colours of the console
    for(uint16_t y = top; y < top+height; y++)
        for(uint16_t x = left; x < left+width; x++){
          console->set_foreground_color(x, y, foreground);
          console->set_background_color(x, y, background);
        }

}

ConsoleArea::~ConsoleArea() = default;

/**
 * @brief Return the width of the console area
 *
 * @return The width of the console area
 */
uint16_t ConsoleArea::width() {
    return m_width;
}

/**
 * @brief Return the height of the console area
 *
 * @return The height of the console area
 */
uint16_t ConsoleArea::height() {
    return m_height;
}

/**
 * @brief Place a character on the console area if the coordinates are within the area
 *
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @param c The character to put on the console
 */
void ConsoleArea::put_character(uint16_t x, uint16_t y, char c) {

    // Check bounds
    if(x >= m_width || y >= m_height)
        return;

    // Write to the console
    m_console->put_character(m_left + x, m_top + y, c);
}


/**
 * @brief Change the foreground color of a character on the console area if the coordinates are within the area
 *
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @param foreground The foreground color of the character
 */
void ConsoleArea::set_foreground_color(uint16_t x, uint16_t y, ConsoleColour foreground) {

    // Make sure the coordinates are within the console area
    if(x >= m_width || y >= m_height)
        return;

    // Set the foreground color of the character
    m_console->set_foreground_color(m_left + x, m_top + y, foreground);

}

/**
 * @brief Change the background color of a character on the console area if the coordinates are within the area
 *
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @param background The background color of the character
 */
void ConsoleArea::set_background_color(uint16_t x, uint16_t y, ConsoleColour background) {

    // Make sure the coordinates are within the console area
    if(x >= m_width || y >= m_height)
        return;

    // Set the background color of the character
    m_console->set_background_color(m_left + x, m_top + y, background);

}

/**
 * @brief Return the character at the given coordinates if the coordinates are within the console area
 *
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @return The character at the given coordinates, if the coordinates are within the console area otherwise " "
 */
char ConsoleArea::get_character(uint16_t x, uint16_t y) {

    // Make sure the coordinates are within the console area
    if(x >= m_width || y >= m_height)
        return ' ';

    // Return the character at the given coordinates
    return m_console->get_character(m_left + x, m_top + y);
}

/**
 * @brief Return the foreground color of the character at the given coordinates if the coordinates are within the console area
 *
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @return The foreground color of the character at the given coordinates, if the coordinates are within the console area otherwise ConsoleColour::LightGrey
 */
ConsoleColour ConsoleArea::get_foreground_color(uint16_t x, uint16_t y) {

    // Make sure the coordinates are within the console area
    if(x >= m_width || y >= m_height)
        return ConsoleColour::LightGrey;

    // Return the foreground color of the character at the given coordinates
    return m_console->get_foreground_color(m_left + x, m_top + y);

}

/**
 * @brief Return the background color of the character at the given coordinates if the coordinates are within the console area
 *
 * @param x The x coordinate of the character
 * @param y The y coordinate of the character
 * @return The background color of the character at the given coordinates, if the coordinates are within the console area otherwise ConsoleColour::Black
 */
ConsoleColour ConsoleArea::get_background_color(uint16_t x, uint16_t y) {

    // Make sure the coordinates are within the console area
    if(x >= m_width || y >= m_height)
        return ConsoleColour::Black;

    // Return the background color of the character at the given coordinates
    return m_console->get_background_color(m_left + x, m_top + y);
}

/**
 * @brief Scroll the console area up by 1 line
 */
void ConsoleArea::scroll_up() {

  m_console->scroll_up(m_left, m_top, m_width, m_height);
}

/**
 * @brief Scroll an area of the console up by 1 line
 *
 * @param left The left coordinate of the area to scroll
 * @param top The top coordinate of the area to scroll
 * @param width The m_width of the area to scroll
 * @param height The m_height of the area to scroll
 * @param foreground The foreground color of the new line
 * @param background The background color of the new line
 * @param fill The character to fill the new line with
 */
void ConsoleArea::scroll_up(uint16_t left, uint16_t top, uint16_t width,
                            uint16_t height, common::ConsoleColour foreground,
                            common::ConsoleColour background, char fill) {

  m_console->scroll_up(m_left + left, m_top + top, width, height, foreground, background, fill);

}

/**
 * @brief Construct a new Console Stream object
 * @param console The console to create the stream on
 */
ConsoleStream::ConsoleStream(Console *console)
: m_console(console)
{

}

ConsoleStream::~ConsoleStream() = default;

/**
 * @brief write a character to the console stream
 *
 * @param c The character to write
 */
void ConsoleStream::write_char(char c) {

    uint16_t spaces = 0;

    // If the character placement is more than the width of the console go on a new line
    if(m_cursor_x >= m_console->width() && c != '\n'){
	    line_feed();
        write("             ");
    }

    // Check if the character is an ANSI escape code
    if(c == '\033') is_ansi = true;

    // Handle the character
    switch (c) {
        // New line
        case '\n':
            // Go to the next line, if it is more then what the console can fit then scroll
            if(++m_cursor_y >= m_console->height()){
                m_console->scroll_up();

                // Ensure there is space at the bottom to write
                m_cursor_y = m_console->height()-1;
            }

            // don't break here, we want to go to the next case because of the \r
            [[fallthrough]];

        // Carriage return
        case '\r':
            m_cursor_x = 0;
            break;

        // End of string
        case '\0':
            break;

        // Backspace
        case '\b':
            m_cursor_x--;
            break;

        // Tab
        case '\t':

            // Pad with spaces until a tab is reached
            spaces = 8 - (m_cursor_x % 8);
            for(uint16_t i = 0; i < spaces; i++)
                write_char(' ');
            break;

        default:
            // Put the character on the console
            m_console->put_character(m_cursor_x, m_cursor_y, c);

            // Increment the x coordinate (ANSI aren't rendered)
            if(!is_ansi)
              m_cursor_x++;

            break;

    }

    // Check if the ANSI code is complete
    if(c == 'm') is_ansi = false;

}

/**
 * @brief Set the m_position of the cursor
 *
 * @param x The x coordinate of the cursor
 * @param y The y coordinate of the cursor
 */
void ConsoleStream::set_cursor(uint16_t x, uint16_t y) {

    // Set the x and y coordinates
    m_cursor_x = x;
    m_cursor_y = y;
}
