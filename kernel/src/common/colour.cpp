//
// Created by 98max on 30/03/2023.
//

#include <common/colour.h>

using namespace MaxOS::common;

Colour::Colour() = default;

Colour::Colour(uint8_t red, uint8_t green, uint8_t blue)
: red(red),
  green(green),
  blue(blue)
{

}

Colour::Colour(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
: red(red),
  green(green),
  blue(blue),
  alpha(alpha)
{

}

Colour::~Colour() = default;

Colour::Colour(ConsoleColour colour) {
    parse_console_colour(colour);
}

Colour::Colour(MaxOS::string string) {

    if(string[0] == '#')
        parse_hex_string(string);
    else
        parse_ansi_string(string);

}

/**
 * @brief Parses a hex string, must be in the format #RRGGBB or #RRGGBBAA
 *
 * @param hex_string The hex string
 */
void Colour::parse_hex_string(string hex_string) {

    // Check if the string is a valid hex string
    if(hex_string.length() != 7 || hex_string.length() != 9)
        return;

    // Parse the red, green and blue values
    red = (hex_string[1] - '0') * 16 + (hex_string[2] - '0');
    green = (hex_string[3] - '0') * 16 + (hex_string[4] - '0');
    blue = (hex_string[5] - '0') * 16 + (hex_string[6] - '0');

    // Parse the alpha value
    alpha = 255;
    if(hex_string.length() == 9)
        alpha = (hex_string[7] - '0') * 16 + (hex_string[8] - '0');


}

/**
 * @brief Parses an ANSI string to a colour  in the format \033[--COLORm (30-37)
 *
 * @param ansi_string The ANSI string
 */
void Colour::parse_ansi_string(string ansi_string) {

  // Check if the string is a valid ANSI string
  if(ansi_string.length() != 7 || ansi_string[0] != '\033' || ansi_string[1] != '[' || ansi_string[6] != 'm')
    return;

  // Parse the colour
  uint8_t colour = ansi_string[5] - '0';

  // Set the colour
  switch (colour) {

      case 0:
        parse_console_colour(ConsoleColour::Black);
        break;

      case 1:
        parse_console_colour(ConsoleColour::Red);
        break;

      case 2:
        parse_console_colour(ConsoleColour::Green);
        break;

      case 3:
        parse_console_colour(ConsoleColour::Yellow);
        break;

      case 4:
        parse_console_colour(ConsoleColour::Blue);
        break;

      case 5:
        parse_console_colour(ConsoleColour::Magenta);
        break;

      case 6:
        parse_console_colour(ConsoleColour::Cyan);
        break;

      case 7:
        parse_console_colour(ConsoleColour::White);
        break;


      default:
        break;
  }
}

/**
 * @brief Parses a console colour to a colour
 *
 * @param colour The console colour
 */
void Colour::parse_console_colour(ConsoleColour colour) {
  switch (colour) {

  case ConsoleColour::Uninitialised:
  case ConsoleColour::Black:
      red = 0;
      green = 0;
      blue = 0;
      break;

    case ConsoleColour::Blue:
      red = 0;
      green = 128;
      blue = 253;
      break;

    case ConsoleColour::Green:
      red = 0;
      green = 170;
      blue = 0;
      break;

    case ConsoleColour::Cyan:
      red = 0;
      green = 170;
      blue = 170;
      break;

    case ConsoleColour::Red:
      red = 170;
      green = 0;
      blue = 0;
      break;

    case ConsoleColour::Magenta:
      red = 170;
      green = 0;
      blue = 170;
      break;

    case ConsoleColour::Brown:
      red = 170;
      green = 85;
      blue = 0;
      break;

    case ConsoleColour::LightGrey:
      red = 170;
      green = 170;
      blue = 170;
      break;

    case ConsoleColour::DarkGrey:
      red = 85;
      green = 85;
      blue = 85;
      break;

    case ConsoleColour::LightBlue:
      red = 85;
      green = 85;
      blue = 255;
      break;

    case ConsoleColour::LightGreen:
      red = 85;
      green = 255;
      blue = 85;
      break;

    case ConsoleColour::LightCyan:
      red = 85;
      green = 255;
      blue = 255;
      break;

    case ConsoleColour::LightRed:
      red = 255;
      green = 85;
      blue = 85;
      break;

    case ConsoleColour::LightMagenta:
      red = 255;
      green = 85;
      blue = 255;
      break;

    // Same as CLION yellow
    case ConsoleColour::Yellow:
      red = 0x96;
      green = 0x82;
      blue = 0x0E;
      break;

    case ConsoleColour::White:
      red = 255;
      green = 255;
      blue = 255;
      break;
  }
}

/**
 * @brief Converts the colour to a console colour
 *
 * @return The console colour
 */
ConsoleColour Colour::to_console_colour() const {


  if (red == 0 && green == 0 && blue == 0)
    return ConsoleColour::Black;

  if (red == 0 && green == 128 && blue == 253)
    return ConsoleColour::Blue;

  if (red == 0 && green == 170 && blue == 0)
    return ConsoleColour::Green;

  if (red == 0 && green == 170 && blue == 170)
    return ConsoleColour::Cyan;

  if (red == 170 && green == 0 && blue == 0)
    return ConsoleColour::Red;

  if (red == 170 && green == 0 && blue == 170)
    return ConsoleColour::Magenta;

  if (red == 170 && green == 85 && blue == 0)
    return ConsoleColour::Brown;

  if (red == 170 && green == 170 && blue == 170)
    return ConsoleColour::LightGrey;

  if (red == 85 && green == 85 && blue == 85)
    return ConsoleColour::DarkGrey;

  if (red == 85 && green == 85 && blue == 255)
    return ConsoleColour::LightBlue;

  if (red == 85 && green == 255 && blue == 85)
    return ConsoleColour::LightGreen;

  if (red == 85 && green == 255 && blue == 255)
    return ConsoleColour::LightCyan;

  if (red == 255 && green == 85 && blue == 85)
    return ConsoleColour::LightRed;

  if (red == 255 && green == 85 && blue == 255)
    return ConsoleColour::LightMagenta;

  if (red == 0x96 && green == 0x82 && blue == 0x0E)
    return ConsoleColour::Yellow;

  if (red == 255 && green == 255 && blue == 255)
    return ConsoleColour::White;

  // Return a default value in case no match is found
  return ConsoleColour::Black;
}
