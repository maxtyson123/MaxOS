//
// Created by 98max on 10/16/2022.
//

#include <gui/widgets/text.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::gui;
using namespace maxOS::gui::widgets;

Text::Text(int32_t left, int32_t top, uint32_t width, uint32_t height, string text)
: Widget(left, top, width, height)
{
    // Set the default font
    this -> font = AmigaFont();

    // Set the text
    updateText(text);

    // Set the default colours
    foregroundColour = Colour(0,0,0);
    backgroundColour = Colour(255,255,255);
}

Text::~Text() {


}

/**
 * @details Draw the text on the screen
 *
 * @param gc The graphics context to draw on
 * @param area The area of the text to draw
 */
void Text::draw(GraphicsContext *gc, Rectangle<int>& area) {

    // Default Draw Operation
    Widget::draw(gc, area);

    // Get the absolute position of the text
    Coordinates textCoordinates = absoluteCoordinates(Coordinates(0,0));
    Rectangle<int> textPosition = getPosition();

    // Get the x and y position of the text
    int32_t x = textCoordinates.first;
    int32_t y = textCoordinates.second;

    // Draw the background for the text (TODO: Might not need to do this as the background is drawn by the default draw operation)
    gc -> fillRectangle(x+area.left, y+area.top, x+area.left+area.width, y+area.top+area.height, backgroundColour);

    // Draw the text
    this ->font.drawText(x, y, foregroundColour, backgroundColour, gc, widgetText, area);

}

/**
 * @details Update the text of the widget
 * @param newText The new text to display
 */
void Text::updateText(common::string newText) {


    // Copy the new text into the widget by looping through the characters
    for(uint32_t i = 0; i < 1000; i++)
    {
        // Set the character
        this -> widgetText[i] = newText[i];

        // Check if the end of the string has been reached
        if(newText[i] == '\0')
            break;
    }

    // New text has  been set so invalidate the widget
    invalidate();

}

