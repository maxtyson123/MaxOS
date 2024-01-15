//
// Created by 98max on 10/10/2023.
//

#include <gui/widgets/button.h>
#include <gui/font/amiga_font.h>

using namespace MaxOS::common;
using namespace MaxOS::gui;
using namespace MaxOS::gui::widgets;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::peripherals;

/// ___ Button Event Handler ___

ButtonEventHandler::ButtonEventHandler() {

}

ButtonEventHandler::~ButtonEventHandler() {

}

/**
 * @brief Handles the button events
 *
 * @param event The event to handle
 */
Event<ButtonEvents>* ButtonEventHandler::on_event(Event<ButtonEvents> *event) {

    // Check the event type
    switch (event -> type) {

        // Button pressed
        case BUTTON_PRESSED:
            on_button_pressed(((ButtonPressedEvent *)event)->source);
            break;

        // Button released
        case BUTTON_RELEASED:
            on_button_released(((ButtonReleasedEvent *)event)->source);
            break;

    }
    return event;
}

/**
 * @brief Handles the button pressed event
 *
 * @param source The source of the event
 */
void ButtonEventHandler::on_button_pressed(Button*) {

}

/**
 * @brief Handles the button released event
 *
 * @param source The source of the event
 */
void ButtonEventHandler::on_button_released(Button*) {

}



//// ___ Button ___

Button::Button(int32_t left, int32_t top, uint32_t width, uint32_t height, string text)
: Widget(left, top, width, height),
  background_colour(Colour(0xFF, 0xFF, 0xFF)),
  foreground_colour(Colour(0x00, 0x00, 0x00)),
  border_colour(Colour(0x57, 0x57, 0x57)),
  font((uint8_t*)AMIGA_FONT),
  text(text)
{

}

Button::~Button() {

}

/**
 * @brief Draws the button
 *
 * @param gc The graphics context to draw to
 * @param area The area to draw to
 */
void Button::draw(GraphicsContext *gc, Rectangle<int32_t> &area) {

    // Default Draw Operation
    Widget::draw(gc, area);

    // Get the absolute m_position of the button
    Coordinates buttonCoordinates = absolute_coordinates(Coordinates(0, 0));
    Rectangle<int32_t> buttonPosition = position();

    // Get the x and y m_position of the button
    int32_t x = buttonCoordinates.first;
    int32_t y = buttonCoordinates.second;

    // Draw the background for the button
    gc->fill_rectangle(x + area.left, y + area.top, x + area.left + area.width,
                       y + area.top + area.height, background_colour);

    // Draw the border  (TODO: Make a border class?? Window uses it too)

    // Top Border
    if(area.intersects(Rectangle<int32_t>(0,0,buttonPosition.width,1))){

        // Start in the top left corner of the button and end in the top right corner
        gc->draw_line(x + area.left, y, x + area.left + area.width - 1, y,
                      border_colour);
    }

    // Left Border
    if(area.intersects(Rectangle<int32_t>(0,0,1,buttonPosition.height))){

        // Start in the top left corner and end in the bottom left corner
        gc->draw_line(x, y + area.top, x, y + area.top + area.height - 1,
                      border_colour);
    }

    // Right Border
    if(area.intersects(Rectangle<int32_t>(0,buttonPosition.height - 1,buttonPosition.width,1))){

        // Start in the top right corner and end in the bottom right corner
        gc->draw_line(x + area.left, y + buttonPosition.height - 1,
                      x + area.left + area.width - 1,
                      y + buttonPosition.height - 1, border_colour);
    }

    // Bottom Border
    if(area.intersects(Rectangle<int32_t>(buttonPosition.width - 1,0,1,buttonPosition.height))){

        // Start in the bottom left corner and end in the bottom right corner
        gc->draw_line(x + buttonPosition.width - 1, y + area.top,
                      x + buttonPosition.width - 1,
                      y + area.top + area.height - 1, border_colour);
    }

    // Draw the text
    common::Rectangle<int32_t> textArea(area.left - 1, area.top - 1, area.width, area.height);
    font.draw_text(x + 1, y + 1, foreground_colour, background_colour, gc, text,
                   textArea);

}

/**
 * @brief Handles the mouse button pressed event
 *
 * @param x The x m_position of the mouse
 * @param y The y m_position of the mouse
 * @param button The button that was pressed
 * @return The mouse event handler
 */
MouseEventHandler* Button::on_mouse_button_pressed(uint32_t x, uint32_t y, uint8_t button) {

    // Raise the event
    raise_event(new ButtonPressedEvent(this));

    // Change the button colour
    background_colour = Colour(0x57, 0x57, 0x57);
    Widget::invalidate();

    // Pass the event on (that it was handled)
    return Widget::on_mouse_button_pressed(x, y, button);
}

/**
 * @brief Handles the mouse button released event
 *
 * @param x The x m_position of the mouse
 * @param y The y m_position of the mouse
 * @param button The button that was released
 */
void Button::on_mouse_button_released(uint32_t x, uint32_t y, uint8_t button) {

    // Raise the button released event
    raise_event(new ButtonReleasedEvent(this));

    // Change the button colour
    background_colour = Colour(0xFF, 0xFF, 0xFF);
    Widget::invalidate();

    // Pass the event on (that it was handled)
    Widget::on_mouse_button_released(x, y, button);
}

/// ___ Event ___

ButtonReleasedEvent::ButtonReleasedEvent(Button *source)
: Event(ButtonEvents::BUTTON_RELEASED),
  source(source)
{

}

ButtonReleasedEvent::~ButtonReleasedEvent() {

}

ButtonPressedEvent::ButtonPressedEvent(Button *source)
: Event(ButtonEvents::BUTTON_PRESSED),
  source(source)
{
}

ButtonPressedEvent::~ButtonPressedEvent() {

}