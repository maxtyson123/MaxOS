//
// Created by 98max on 10/2/2023.
//

#include <drivers/console/vesaboot.h>
#include <gui/font/amiga_font.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::gui;
using namespace MaxOS::memory;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::console;

VESABootConsole::VESABootConsole(GraphicsContext *graphics_context)
: Driver(),
  Console(),
  m_graphics_context(graphics_context),
  m_font((uint8_t*)AMIGA_FONT)
{

    // Malloc the video memory
    m_video_memory = (uint16_t*)MemoryManager::s_active_memory_manager->malloc(width() * height() * sizeof(uint16_t));
}

VESABootConsole::~VESABootConsole()
{

}

/**
 * @brief Gets the width of the console
 *
 * @return The width of the console in characters
 */
uint16_t VESABootConsole::width()
{
    return m_graphics_context->get_width() / 8;       // 8 pixels per character
}

/**
 * @brief Gets the height of the console
 *
 * @return The height of the console in characters
 */
uint16_t VESABootConsole::height()
{
    return m_graphics_context->get_height() / 9;      // 9 pixels per character
}
/**
 * @brief Places a character at the specified location
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @param c The character to place
 */
void VESABootConsole::put_character(uint16_t x, uint16_t y, char c) {


    // Parse any ansi codes
    if (c == '\033') {

      // Store the character
      ansi_code_length = 0;
      ansi_code[ansi_code_length++] = c;

      // Do not draw the escape character
      return;

    } else if (ansi_code_length != -1 && ansi_code_length < 8) {

      // Add the character to the ANSI code
      ansi_code[ansi_code_length++] = c;

      // If the ANSI code is complete
      if (c == 'm') {
        ansi_code[ansi_code_length] = '\0';
        ansi_code_length = -1;

        if(strcmp("\033[0m", ansi_code)) {
          m_foreground_color = ConsoleColour::Unititialised;
          m_background_color = ConsoleColour::Unititialised;
          return;
        }

        // Get the colour from the ANSI code
        Colour* colour = new Colour(ansi_code);

        // Set the colour
        bool foreground = ansi_code[4] == '3';
        if (foreground)
          m_foreground_color = colour->to_console_colour();
        else
          m_background_color = colour->to_console_colour();

        // Delete the colour
        delete colour;

      }

      // Do not draw the escape character
      return;
    }

    // If the coordinates are out of bounds, return
    if(x >= width() || y >= height())
        return;

    // Calculate the offset
    int offset = (y* width() + x);

    // Set the character at the offset, by masking the character with the current character (last 8 bits)
    m_video_memory[offset] = (m_video_memory[offset] & 0xFF00) | (uint16_t)c;

    // Convert the char into a string
    char s[] = " ";
    s[0] = c;


    Colour foreground = m_foreground_color == ConsoleColour::Unititialised ? get_foreground_color(x, y) : Colour(m_foreground_color);
    Colour background = m_background_color == ConsoleColour::Unititialised ? get_background_color(x, y) : Colour(m_background_color);

    // Use the m_font to draw the character
    m_font.draw_text(x * 8, y * 9, foreground, background, m_graphics_context, s);

}

/**
 * @brief Sets the foreground color at the specified location
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @param foreground The foreground color
 */
void VESABootConsole::set_foreground_color(uint16_t x, uint16_t y, ConsoleColour foreground) {

    // If the coordinates are out of bounds, return
    if(x >= width() || y >= height())
        return;

    // Calculate the offset
    int offset = (y* width() + x);

    // Set the foreground color at the offset, by masking the foreground color with the current foreground color (bits 8-11)
    m_video_memory[offset] = (m_video_memory[offset] & 0xF0FF) | ((uint16_t)foreground << 8);
}

/**
 * @brief Sets the background color at the specified location
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @param background The background color
 */
void VESABootConsole::set_background_color(uint16_t x, uint16_t y, ConsoleColour background) {

    // If the coordinates are out of bounds, return
    if(x >= width() || y >= height())
        return;

    // Calculate the offset
    int offset = (y* width() + x);

    // Set the background color at the offset, by masking the background color with the current background color (bits 12-15)
    m_video_memory[offset] = (m_video_memory[offset] & 0x0FFF) | ((uint16_t)background << 12);

}

/**
 * @brief Gets the character at the specified location
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @return The character at the specified location or a space if the coordinates are out of bounds
 */
char VESABootConsole::get_character(uint16_t x, uint16_t y) {

    // If the coordinates are out of bounds, return
    if(x >= width() || y >= height())
        return ' ';

    // Calculate the offset
    int offset = (y* width() + x);

    // Return the character at the offset, by masking the character with the current character (last 8 bits)
    return (char)(m_video_memory[offset] & 0x00FF);
}

/**
 * @brief  Gets the foreground color at the specified location
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @return The foreground color at the specified location or white if the coordinates are out of bounds
 */
ConsoleColour VESABootConsole::get_foreground_color(uint16_t x, uint16_t y) {

    // If the coordinates are out of bounds, return
    if(x >= width() || y >= height())
        return ConsoleColour::White;

    // Calculate the offset
    int offset = (y* width() + x);

    // Return the foreground color at the offset, by masking the foreground color with the current foreground color (bits 8-11)
    return (ConsoleColour)((m_video_memory[offset] & 0x0F00) >> 8);
}

/**
 * @brief Gets the background color at the specified location
 * @param x The x coordinate
 * @param y The y coordinate
 *
 * @return The background color at the specified location or black if the coordinates are out of bounds
 */
ConsoleColour VESABootConsole::get_background_color(uint16_t x, uint16_t y) {

    // If the coordinates are out of bounds, return
    if(x >= width() || y >= height())
        return ConsoleColour::Black;

    // Calculate the offset
    int offset = (y* width() + x);

    // Return the background color at the offset, by masking the background color with the current background color (bits 12-15)
    return (ConsoleColour)((m_video_memory[offset] & 0xF000) >> 12);
}

/**
 * @brief Prints the logo to the center of the screen
 */
void VESABootConsole::print_logo() {

      // Load the logo
      const char* logo = header_data;

      // Find the center of the screen
      uint32_t center_x = m_graphics_context->get_width()/2;
      uint32_t center_y = m_graphics_context->get_height()/2 + 20;

      // Draw the logo
      for (uint32_t logoY = 0; logoY < logo_height; ++logoY) {
        for (uint32_t logoX = 0; logoX < logo_width; ++logoX) {

          // Store the pixel in the logo
          uint8_t pixel[3] = {0};

          // Get the pixel from the logo
          LOGO_HEADER_PIXEL(logo, pixel);

          // Draw the pixel
          m_graphics_context->put_pixel(center_x - logo_width / 2 + logoX,
                                        center_y - logo_height / 2 + logoY,
                                    common::Colour(pixel[0], pixel[1], pixel[2]));
        }
      }
}
