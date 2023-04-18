//
// Created by 98max on 4/15/2023.
//

#include <gui/font.h>

using namespace maxOS;
using namespace maxOS::gui;
using namespace maxOS::common;


Font::Font() {

    fontSize = 8;   //TODO: Use this

    isBold = false;
    isItalic = false;
    isUnderlined = false;
    isStrikethrough = false;

    foregroundColour = Colour(0,0,0);
    backgroundColour = Colour(255,255,255);

}

Font::~Font() {

}

/**
 * @details Write the entire text to the screen
 * @param x The x coordinate of the text
 * @param y The y coordinate of the text
 * @param context The graphics context to draw the text on
 * @param text The text to draw
 */
void Font::drawText(int32_t x, int32_t y, GraphicsContext *context, string text) {

    // Calculate the rectangle of the text
    int32_t top = 0;
    int32_t left = 0;
    uint32_t width = getTextWidth(text);
    uint32_t height = getTextHeight(text);

    // Create the rectangle
    Rectangle<int> textArea(left, top, width, height);

    // Draw the text
    drawText(x, y, context, text, textArea);
}


/**
 * @details Write the entire text to the screen
 * @param x The x coordinate of the text
 * @param y The y coordinate of the text
 * @param context The graphics context to draw the text on
 * @param text The text to draw
 * @param limitArea The area of the text to draw
 */
void Font::drawText(int32_t x, int32_t y, GraphicsContext *context, string text,  Rectangle<int> limitArea) {

    // Convert the colours
    uint32_t foreground = context->colourToInt(foregroundColour);
    uint32_t background = context->colourToInt(backgroundColour);

    // Ensure the area is within the actual area of the text
    if (limitArea.top < 0) {

        // Move the area down to fake the desired top and set the top to 0
        limitArea.height += limitArea.top;
        limitArea.top = 0;
    }

    if (limitArea.left < 0) {

        // Move the area right to fake the desired left and set the left to 0
        limitArea.width += limitArea.left;
        limitArea.left = 0;
    }

    if (limitArea.top + limitArea.height > getTextHeight(text)) {

        // Set the height to the maximum height
        limitArea.height = getTextHeight(text) - limitArea.top;
    }

    if (limitArea.left + limitArea.width > getTextWidth(text)) {

        // Set the width to the maximum width
        limitArea.width = getTextWidth(text) - limitArea.left;
    }

    // Calculate limits
    int32_t xLimit = limitArea.left + limitArea.width;
    int32_t yLimit = limitArea.top + limitArea.height;


    // Draw the text from top to bottom
    for (int yBitMapOffset = limitArea.top; yBitMapOffset <yLimit; yBitMapOffset++){

        for (int xBitMapOffset = limitArea.left; xBitMapOffset < xLimit; ++xBitMapOffset) {

            // If the y is the middle then add a strikethrough
            if (isStrikethrough && yBitMapOffset == yLimit / 2) {

                // Draw the pixel
                context -> putPixel(x + xBitMapOffset, y + yBitMapOffset, foreground);
                continue;
            }

            // If the y is the bottom then add an underline
            if (isUnderlined && yBitMapOffset == yLimit - 1) {

                // Draw the pixel
                context -> putPixel(x + xBitMapOffset, y + yBitMapOffset, foreground);
                continue;
            }

            //TODO: Bold, Italic

            // Get the character
            uint8_t character = text[xBitMapOffset/8];       // Divide by 8 as each character is 8 pixels wide.

           // Check if this pixel  is set or not
           bool set = font8x8[(uint16_t)character * 8 + yBitMapOffset] & (128 >> (xBitMapOffset % 8));

           // Draw the pixel
           context -> putPixel(x + xBitMapOffset, y + yBitMapOffset, set ? foreground : background);

        }
    }
}

/**
 * @details Get the height of the text
 * @param text The text to get the height of
 * @return The height of the text
 */
common::uint32_t Font::getTextHeight(common::string text) {

    return 8;

}

/**
 * @details Get the width of the text
 * @param text The text to get the width of
 * @return The width of the text
 */
common::uint32_t Font::getTextWidth(common::string text) {
    uint32_t width = 0;

    // Loop through each character and add the width
    for (char* character = (char *)text;  *character != '\0'; character++) {
        width += 8;
    }

    return width;
}

AmigaFont::AmigaFont() {


}

AmigaFont::~AmigaFont() {


}