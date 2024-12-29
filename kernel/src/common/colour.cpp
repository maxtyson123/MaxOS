//
// Created by 98max on 30/03/2023.
//

#include <common/colour.h>
#include <common/kprint.h>

using namespace MaxOS::common;

Colour::Colour() {

}

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

Colour::~Colour() {

}
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
        parse_console_colour(Black);
        break;

      case 1:
        parse_console_colour(Red);
        break;

      case 2:
        parse_console_colour(Green);
        break;

      case 3:
        parse_console_colour(Yellow);
        break;

      case 4:
        parse_console_colour(Blue);
        break;

      case 5:
        parse_console_colour(Magenta);
        break;

      case 6:
        parse_console_colour(Cyan);
        break;

      case 7:
        parse_console_colour(White);
        break;

      default:
        break;
  }
}

/**
 * @brief Parses a console colour to a colour
 * @param colour The console colour
 */
void Colour::parse_console_colour(ConsoleColour colour) {
  switch (colour) {

    case Black:
      red = 0;
      green = 0;
      blue = 0;
      break;

    case Blue:
      red = 0;
      green = 0;
      blue = 170;
      break;

    case Green:
      red = 0;
      green = 170;
      blue = 0;
      break;

    case Cyan:
      red = 0;
      green = 170;
      blue = 170;
      break;

    case Red:
      red = 170;
      green = 0;
      blue = 0;
      break;

    case Magenta:
      red = 170;
      green = 0;
      blue = 170;
      break;

    case Brown:
      red = 170;
      green = 85;
      blue = 0;
      break;

    case LightGrey:
      red = 170;
      green = 170;
      blue = 170;
      break;

    case DarkGrey:
      red = 85;
      green = 85;
      blue = 85;
      break;

    case LightBlue:
      red = 85;
      green = 85;
      blue = 255;
      break;

    case LightGreen:
      red = 85;
      green = 255;
      blue = 85;
      break;

    case LightCyan:
      red = 85;
      green = 255;
      blue = 255;
      break;

    case LightRed:
      red = 255;
      green = 85;
      blue = 85;
      break;

    case LightMagenta:
      red = 255;
      green = 85;
      blue = 255;
      break;

    case Yellow:
      red = 255;
      green = 255;
      blue = 85;
      break;

    case White:
      red = 255;
      green = 255;
      blue = 255;
      break;
  }
}

/**
 * @brief Converts the colour to a console colour
 * @return The console colour
 */
ConsoleColour Colour::to_console_colour() const {


  if (red == 0 && green == 0 && blue == 0)
    return Black;

  if (red == 0 && green == 0 && blue == 170)
    return Blue;

  if (red == 0 && green == 170 && blue == 0)
    return Green;

  if (red == 0 && green == 170 && blue == 170)
    return Cyan;

  if (red == 170 && green == 0 && blue == 0)
    return Red;

  if (red == 170 && green == 0 && blue == 170)
    return Magenta;

  if (red == 170 && green == 85 && blue == 0)
    return Brown;

  if (red == 170 && green == 170 && blue == 170)
    return LightGrey;

  if (red == 85 && green == 85 && blue == 85)
    return DarkGrey;

  if (red == 85 && green == 85 && blue == 255)
    return LightBlue;

  if (red == 85 && green == 255 && blue == 85)
    return LightGreen;

  if (red == 85 && green == 255 && blue == 255)
    return LightCyan;

  if (red == 255 && green == 85 && blue == 85)
    return LightRed;

  if (red == 255 && green == 85 && blue == 255)
    return LightMagenta;

  if (red == 255 && green == 255 && blue == 85)
    return Yellow;

  if (red == 255 && green == 255 && blue == 255)
    return White;

  // Return a default value in case no match is found
  return Black;
}
