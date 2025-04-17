//
// Created by 98max on 10/2/2023.
//

#include <drivers/console/vesaboot.h>
#include <gui/font/amiga_font.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::gui;
using namespace MaxOS::memory;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::console;
using namespace MaxOS::system;

VESABootConsole::VESABootConsole(GraphicsContext *graphics_context)
: m_font((uint8_t*)AMIGA_FONT)
{

    // Set up
    Logger::INFO() << "Setting up VESA console\n";
    s_graphics_context = graphics_context;
    m_video_memory_meta = (uint16_t*)MemoryManager::kmalloc(width() * height() * sizeof(uint16_t));

    // Prepare the console
    VESABootConsole::clear();
    print_logo();

    // Connect to the loggers
    m_console_area = new ConsoleArea(this, 0, 0, width() / 2 - 25, height(), ConsoleColour::DarkGrey, ConsoleColour::Black);
    cout = new ConsoleStream(m_console_area);

    #ifdef TARGET_DEBUG
        Logger::active_logger() -> add_log_writer(cout);
        Logger::INFO() << "Console Stream set up \n";
    #endif

    update_progress_bar(0);
}

VESABootConsole::~VESABootConsole() = default;

/**
 * @brief Gets the width of the console
 *
 * @return The width of the console in characters
 */
uint16_t VESABootConsole::width()
{
    return s_graphics_context->width() / 8;       // 8 pixels per character
}

/**
 * @brief Gets the height of the console
 *
 * @return The height of the console in characters
 */
uint16_t VESABootConsole::height()
{
    return s_graphics_context->height() / Font::font_height;
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

    } else if (ansi_code_length < 8) {

      // Add the character to the ANSI code
      ansi_code[ansi_code_length++] = c;

      // If the ANSI code is complete
      if (c == 'm') {
        ansi_code[ansi_code_length] = '\0';
        ansi_code_length = -1;

        if(strcmp("\033[0m", ansi_code) != 0) {
          m_foreground_color = ConsoleColour::Uninitialised;
          m_background_color = ConsoleColour::Uninitialised;
          return;
        }

        // Get the colour from the ANSI code
        auto* colour = new Colour(ansi_code);

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
    m_video_memory_meta[offset] = (m_video_memory_meta[offset] & 0xFF00) | (uint16_t)c;

    // Convert the char into a string
    char s[] = " ";
    s[0] = c;

    Colour foreground = m_foreground_color == ConsoleColour::Uninitialised ? get_foreground_color(x, y) : Colour(m_foreground_color);
    Colour background = m_background_color == ConsoleColour::Uninitialised ? get_background_color(x, y) : Colour(m_background_color);

    // Use the m_font to draw the character
    m_font.draw_text(x * 8, y * Font::font_height, foreground, background, s_graphics_context, s);

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
    m_video_memory_meta[offset] = (m_video_memory_meta[offset] & 0xF0FF) | ((uint16_t)foreground << 8);
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
    m_video_memory_meta[offset] = (m_video_memory_meta[offset] & 0x0FFF) | ((uint16_t)background << 12);

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
    return (char)(m_video_memory_meta[offset] & 0x00FF);
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
    return (ConsoleColour)((m_video_memory_meta[offset] & 0x0F00) >> 8);
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
    return (ConsoleColour)((m_video_memory_meta[offset] & 0xF000) >> 12);
}

/**
 * @brief Prints the logo to the center of the screen
 */
void VESABootConsole::print_logo() {

      // Load the logo
      const char* logo = header_data;

      // Find the center of the screen
      uint32_t center_x = s_graphics_context->width()/2;
      uint32_t center_y = s_graphics_context->height()/2 - 80;

      // Draw the logo
      for (uint32_t logoY = 0; logoY < logo_height; ++logoY) {
        for (uint32_t logoX = 0; logoX < logo_width; ++logoX) {

          // Store the pixel in the logo
          uint8_t pixel[3] = {0};

          // Get the pixel from the logo
          LOGO_HEADER_PIXEL(logo, pixel)

          // Draw the pixel
          s_graphics_context->put_pixel(center_x - logo_width / 2 + logoX,
                                        center_y - logo_height / 2 + logoY,
                                    common::Colour(pixel[0], pixel[1], pixel[2]));
        }
      }
}


/**
 * @brief Scrolls the console up by 1 line
 *
 * @param left The left coordinate of the area to scroll
 * @param top The top coordinate of the area to scroll
 * @param width The width of the area to scroll
 * @param height The height of the area to scroll
 * @param foreground The foreground color of the new line
 * @param background The background color of the new line
 * @param fill The character to fill the new line with
 */
void VESABootConsole::scroll_up(uint16_t left, uint16_t top, uint16_t width,
                                uint16_t height,
                                common::ConsoleColour foreground,
                                common::ConsoleColour background, char) {


  // Get the framebuffer info
  auto* framebuffer_address = (uint8_t*)s_graphics_context->framebuffer_address();
  uint64_t framebuffer_width   = s_graphics_context->width();
  uint64_t framebuffer_height  = s_graphics_context->height();
  uint64_t framebuffer_bpp     = s_graphics_context->color_depth(); // in bits per pixel
  uint64_t bytes_per_pixel     = framebuffer_bpp / 8;
  uint64_t framebuffer_pitch   = framebuffer_width * bytes_per_pixel;

  // Calculate the number of pixels per line
  uint16_t line_height = Font::font_height;

  // Calculate the number of pixels in the region
  uint16_t region_pixel_y = top * line_height;
  uint16_t region_pixel_height  = height * line_height;
  uint16_t region_pixel_left    = left * Font::font_width;
  uint16_t region_pixel_width   = width * Font::font_width;
  size_t row_bytes              = region_pixel_width * bytes_per_pixel;

  // Decide the colour of the pixel
  ConsoleColour to_set_foreground = CPU::is_panicking ? ConsoleColour::White : get_foreground_color(left, top + height - 1);
  ConsoleColour to_set_background = CPU::is_panicking ? ConsoleColour::Red : get_background_color(left, top + height - 1);
  Colour fill_colour = Colour(to_set_background);
  uint32_t fill_value = s_graphics_context->colour_to_int(to_set_background);

  // Scroll the region upward by one text line
  for (uint16_t row = 0; row < region_pixel_height - line_height; row++) {
     uint8_t* src   = framebuffer_address + (region_pixel_y + row + line_height) * framebuffer_pitch + region_pixel_left * bytes_per_pixel;
     uint8_t* dest  = framebuffer_address + (region_pixel_y + row) * framebuffer_pitch + region_pixel_left * bytes_per_pixel;
     memmove(dest, src, row_bytes);
  }

  // Clear the last line of the region
  uint16_t clear_start_y = region_pixel_y + region_pixel_height - line_height;
  for (uint16_t row = 0; row < line_height; row++) {
      auto row_add = (uint32_t*)(framebuffer_address + (clear_start_y + row) * framebuffer_pitch + region_pixel_left * 4);
      for (uint16_t col = 0; col < region_pixel_width; col++) {
          row_add[col] = fill_value;
      }
  }

  //Update any per-pixel colour metadata
  uint16_t text_row = top + height - 1;
  for (uint16_t x = left; x < left + width; x++) {
      set_foreground_color(x, text_row, to_set_foreground);
      set_background_color(x, text_row, to_set_background);
  }
}

/**
 * @brief Print the panic logo in the bottom right corner of the screen
 */
void VESABootConsole::print_logo_kernel_panic() {

  // Load the logo
  const char* logo = header_data_kp;

  // Find the bottom right of the screen
  uint32_t right_x = s_graphics_context->width() - kp_width - 10;
  uint32_t bottom_y = s_graphics_context->height() - kp_height - 10;

  // Draw the logo
  for (uint32_t logoY = 0; logoY < kp_height; ++logoY) {
    for (uint32_t logoX = 0; logoX < kp_width; ++logoX) {

      // Store the pixel in the logo
      uint8_t pixel[3] = {0};

      // Get the pixel from the logo
      LOGO_HEADER_PIXEL(logo, pixel)

      // Draw the pixel
      s_graphics_context->put_pixel(right_x + logoX, bottom_y + logoY, common::Colour(pixel[0], pixel[1], pixel[2]));
    }
  }

}

/**
 * @brief Cleans up the boot console
 */
void VESABootConsole::finish() {

  // Done
  Logger::INFO() << "MaxOS Kernel Successfully Booted\n";

  // Move COUT to the bottom of the screen
  cout->set_cursor(width(), height());

  // Disable the logger
  Logger::active_logger()->disable_log_writer(cout);

}

/**
 * @brief Updates the progress bar
 *
 * @param percentage The percentage to update the progress bar to (0-100)
 */
void VESABootConsole::update_progress_bar(uint8_t percentage) {

    // Must be within bounds
    if(percentage > 100)
        percentage = 100;

    // Must have a valid graphics context
    if(s_graphics_context == nullptr)
        return;

    uint8_t progress_height = 15;
    uint8_t progress_spacing = 20;
    uint8_t progress_width_cull = 40;

    // Find the center of the screen
    uint32_t right_x = (s_graphics_context->width()/2) - logo_width / 2;
    uint32_t bottom_y = (s_graphics_context->height()/2 - 80) - logo_height / 2;

    // Find the bounds
    uint32_t start_x = progress_width_cull;
    uint32_t start_y = logo_height + progress_spacing;
    uint32_t end_x = logo_width - progress_width_cull;
    uint32_t end_y = logo_height + progress_height + progress_spacing;

    // Draw the progress bar
    for (uint32_t progress_y = start_y; progress_y < end_y; ++progress_y) {
        for (uint32_t progress_x = start_x; progress_x < end_x; ++progress_x) {

            // Check if drawing border
            bool is_border = (progress_y == start_y) || (progress_y == end_y - 1) ||
                             (progress_x == start_x) || (progress_x == end_x - 1);

            // Only draw the border if it is the first time drawing it
            is_border = is_border && percentage == 0;

            // If it is not within the percentage, skip it
            if (progress_x > logo_width * percentage / 100 && !is_border)
                continue;


            // Draw the pixel
            s_graphics_context->put_pixel(right_x + progress_x, bottom_y + progress_y, Colour(0xFF, 0xFF, 0xFF));

        }
    }

}
