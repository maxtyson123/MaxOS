//
// Created by 98max on 10/10/2023.
//

#include <gui/widgets/button.h>

using namespace maxOS::common;
using namespace maxOS::gui;
using namespace maxOS::gui::widgets;
using namespace maxOS::drivers;
using namespace maxOS::drivers::peripherals;

/// ___ Button Event Handler ___

ButtonEventHandler::ButtonEventHandler() {

}

ButtonEventHandler::~ButtonEventHandler() {

}

/**
 * @details Handles the button events
 * @param event The event to handle
 */
Event<ButtonEvents>* ButtonEventHandler::onEvent(Event<ButtonEvents> *event) {

    // Check the event type
    switch (event -> type) {

        // Button pressed
        case BUTTON_PRESSED:
            onButtonPressed(((ButtonPressedEvent*)event) -> source);
            break;

        // Button released
        case BUTTON_RELEASED:
            onButtonReleased(((ButtonReleasedEvent*)event) -> source);
            break;

    }

    return event;

}

/**
 * @details Handles the button pressed event
 * @param source The source of the event
 */
void ButtonEventHandler::onButtonPressed(Button *source) {

}

/**
 * @details Handles the button released event
 * @param source The source of the event
 */
void ButtonEventHandler::onButtonReleased(Button *source) {

}



//// ___ Button ___

Button::Button(int32_t left, int32_t top, uint32_t width, uint32_t height, string text)
: Widget(left, top, width, height)
{
    // Set the default values
    this -> font = AmigaFont();
    this -> text = text;
    this -> backgroundColour = Colour(0xFF, 0xFF, 0xFF);
    this -> foregroundColour = Colour(0x00, 0x00, 0x00);
    this -> borderColour = Colour(0x57, 0x57, 0x57);
}

Button::~Button() {

}

/**
 * @details Draws the button
 * @param gc The graphics context to draw to
 * @param area The area to draw to
 */
void Button::draw(GraphicsContext *gc, Rectangle<int> &area) {

    // Default Draw Operation
    Widget::draw(gc, area);

    // Get the absolute position of the button
    Coordinates buttonCoordinates = absoluteCoordinates(Coordinates(0,0));
    Rectangle<int> buttonPosition = getPosition();

    // Get the x and y position of the button
    int32_t x = buttonCoordinates.first;
    int32_t y = buttonCoordinates.second;

    // Draw the background for the button
    gc -> fillRectangle(x + area.left, y + area.top, x + area.left + area.width, y + area.top + area.height, backgroundColour);

    // Draw the border  (TODO: Make a border class?? Window uses it too)

    // Top Border
    if(area.intersects(Rectangle<int>(0,0,buttonPosition.width,1))){

        // Start in the top left corner of the button and end in the top right corner
        gc ->drawLine(x + area.left, y, x + area.left + area.width - 1, y,borderColour);
    }

    // Left Border
    if(area.intersects(Rectangle<int>(0,0,1,buttonPosition.height))){

        // Start in the top left corner and end in the bottom left corner
        gc ->drawLine(x, y + area.top, x, y + area.top + area.height - 1,borderColour);
    }

    // Right Border
    if(area.intersects(Rectangle<int>(0,buttonPosition.height - 1,buttonPosition.width,1))){

        // Start in the top right corner and end in the bottom right corner
        gc ->drawLine(x + area.left, y + buttonPosition.height - 1, x + area.left + area.width - 1, y + buttonPosition.height - 1,borderColour);
    }

    // Bottom Border
    if(area.intersects(Rectangle<int>(buttonPosition.width - 1,0,1,buttonPosition.height))){

        // Start in the bottom left corner and end in the bottom right corner
        gc ->drawLine(x + buttonPosition.width - 1, y + area.top, x + buttonPosition.width - 1, y + area.top + area.height - 1,borderColour);
    }

    // Draw the text
    common::Rectangle<int> textArea(area.left - 1, area.top - 1, area.width, area.height);
    font.drawText(x + 1, y + 1, foregroundColour, backgroundColour, gc, text,textArea);

}

/**
 * @details Handles the mouse button pressed event
 * @param x The x position of the mouse
 * @param y The y position of the mouse
 * @param button The button that was pressed
 * @return The mouse event handler
 */
MouseEventHandler* Button::onMouseButtonPressed(uint32_t x, uint32_t y, uint8_t button) {
    // Raise the button pressed event
    raiseEvent(new ButtonPressedEvent(this));

    // Change the button colour
    backgroundColour = Colour(0x57, 0x57, 0x57);
    Widget::invalidate();

    // Pass the event on (that it was handled)
    return Widget::onMouseButtonPressed(x, y, button);
}

/**
 * @details Handles the mouse button released event
 * @param x The x position of the mouse
 * @param y The y position of the mouse
 * @param button The button that was released
 */
void Button::onMouseButtonReleased(uint32_t x, uint32_t y, uint8_t button) {

    // Raise the button released event
    raiseEvent(new ButtonReleasedEvent(this));

    // Change the button colour
    backgroundColour = Colour(0xFF, 0xFF, 0xFF);
    Widget::invalidate();

    // Pass the event on (that it was handled)
    Widget::onMouseButtonReleased(x, y, button);
}

/// ___ Event ___

ButtonReleasedEvent::ButtonReleasedEvent(Button *source)
: Event(ButtonEvents::BUTTON_RELEASED) {
    this -> source = source;
}

ButtonReleasedEvent::~ButtonReleasedEvent() {

}

ButtonPressedEvent::ButtonPressedEvent(Button *source)
: Event(ButtonEvents::BUTTON_PRESSED) {
    this -> source = source;
}

ButtonPressedEvent::~ButtonPressedEvent() {

}
