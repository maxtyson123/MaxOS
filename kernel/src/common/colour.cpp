//
// Created by 98max on 30/03/2023.
//

#include <common/colour.h>

using namespace maxOS::common;

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
