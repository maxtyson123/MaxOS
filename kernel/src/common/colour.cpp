//
// Created by 98max on 30/03/2023.
//

#include <common/colour.h>

using namespace maxOS::common;

Colour::Colour() {

    // Default to black
    this -> red = 0;
    this -> green = 0;
    this -> blue = 0;
    this -> alpha = 0;

}

Colour::Colour(uint8_t red, uint8_t green, uint8_t blue) {

    // Set the colours, no alpha
    this -> red = red;
    this -> green = green;
    this -> blue = blue;
    this -> alpha = 0;


}

Colour::Colour(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) {

    // Set the colours
    this -> red = red;
    this -> green = green;
    this -> blue = blue;
    this -> alpha = alpha;


}

Colour::~Colour() {

}
