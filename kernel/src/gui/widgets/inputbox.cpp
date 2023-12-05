//
// Created by 98max on 11/10/2023.
//

#include <gui/widgets/inputbox.h>

using namespace maxOS::common;
using namespace maxOS::gui;
using namespace maxOS::gui::widgets;
using namespace maxOS::drivers::peripherals;

/// ___ Event Handlers ___ ///

InputBoxEventHandler::InputBoxEventHandler() {

}

InputBoxEventHandler::~InputBoxEventHandler() {

}

Event<InputBoxEvents>* InputBoxEventHandler::onEvent(Event<InputBoxEvents> *event) {
    switch (event->type) {
        case INPUTBOX_TEXT_CHANGED:
            onInputBoxTextChanged(((InputBoxTextChangedEvent*)event)->newText);
            break;
    }

    return event;
}

void InputBoxEventHandler::onInputBoxTextChanged(string newText) {

}

/// ___ InputBox ___ ///

InputBox::InputBox(int32_t left, int32_t top, uint32_t width, uint32_t height)
: Widget(left, top, width, height)
{

    // Set the defaults
    this -> font = AmigaFont();
    cursorPosition = 0;
    backgroundColour = Colour(0xFF, 0xFF, 0xFF);
    foregroundColour = Colour(0x00, 0x00, 0x00);
    borderColour = Colour(0x57, 0x57, 0x57);

    // Clear the text buffer
    widgetText[0] = '\0';
}

InputBox::InputBox(int32_t left, int32_t top, uint32_t width, uint32_t height, string text)
: Widget(left, top, width, height)
{

    // Set the defaults
    this -> font = AmigaFont();
    cursorPosition = 0;
    backgroundColour = Colour(0xFF, 0xFF, 0xFF);
    foregroundColour = Colour(0x00, 0x00, 0x00);
    borderColour = Colour(0x57, 0x57, 0x57);

    // Clear the text buffer
    widgetText[0] = '\0';

    // Update the text
    updateText(text);

}

InputBox::~InputBox() {

}

void InputBox::draw(GraphicsContext *gc, Rectangle<int> &area) {

    // Default Draw
    Widget::draw(gc, area);

    // Get the absolute position of the input box
    Coordinates inputBoxCoordinates = absoluteCoordinates(Coordinates(0,0));
    Rectangle<int> inputBoxPosition = getPosition();

    // Get the x and y position of the input box
    int32_t x = inputBoxCoordinates.first;
    int32_t y = inputBoxCoordinates.second;

    // Draw the background for the input box
    gc -> fillRectangle(x + area.left, y + area.top, x + area.left + area.width, y + area.top + area.height, backgroundColour);

    // Draw the border  (TODO: Make this a function because it is used in multiple places)

    // Top Border
    if(area.intersects(Rectangle<int>(0,0,inputBoxPosition.width,1))){

        // Start in the top left corner of the button and end in the top right corner
        gc ->drawLine(x + area.left, y, x + area.left + area.width - 1, y,borderColour);
    }

    // Left Border
    if(area.intersects(Rectangle<int>(0,0,1,inputBoxPosition.height))){

        // Start in the top left corner and end in the bottom left corner
        gc ->drawLine(x, y + area.top, x, y + area.top + area.height - 1,borderColour);
    }

    // Right Border
    if(area.intersects(Rectangle<int>(0,inputBoxPosition.height - 1,inputBoxPosition.width,1))){

        // Start in the top right corner and end in the bottom right corner
        gc ->drawLine(x + area.left, y + inputBoxPosition.height - 1, x + area.left + area.width - 1, y + inputBoxPosition.height - 1,borderColour);
    }

    // Bottom Border
    if(area.intersects(Rectangle<int>(inputBoxPosition.width - 1,0,1,inputBoxPosition.height))){

        // Start in the bottom left corner and end in the bottom right corner
        gc ->drawLine(x + inputBoxPosition.width - 1, y + area.top, x + inputBoxPosition.width - 1, y + area.top + area.height - 1,borderColour);
    }

    // Draw the text
    common::Rectangle<int> textArea(area.left - 1, area.top - 1, area.width, area.height);
    font.drawText(x + 1, y + 1, foregroundColour, backgroundColour, gc, &widgetText[0],textArea);
}

void InputBox::onFocus() {

    // Make the border black on focus
    borderColour = Colour(0x00, 0x00, 0x00);
    invalidate();
}

void InputBox::onFocusLost() {

    // Reset the border colour
    borderColour = Colour(0x57, 0x57, 0x57);
    invalidate();
}

void InputBox::onKeyDown(KeyCode keyDownCode, KeyboardState keyDownState) {

    // Handle the key press
    switch(keyDownCode)
    {
        case KeyCode::backspace:
        {
            if(cursorPosition == 0)
                break;

            cursorPosition--;
            // no break - we move the cursor to the left and use the <Delete> code
        }
        case KeyCode::deleteKey:
        {
            // Move the text to the left
            for(uint32_t i = cursorPosition; widgetText[i] != '\0'; ++i)
                widgetText[i] = widgetText[i+1];
            break;
        }
        case KeyCode::leftArrow:
        {
            // If the cursor is not at the beginning of the text, move it to the left
            if(cursorPosition > 0)
                cursorPosition--;
            break;
        }
        case KeyCode::rightArrow:
        {

            // If the cursor is not at the end of the text, move it to the right
            if(widgetText[cursorPosition] != '\0')
                cursorPosition++;
            break;
        }
        default:
        {

            // If the key is a printable character, add it to the text
            if(31 < keyDownCode && keyDownCode < 127)
            {

                uint32_t length = cursorPosition;

                // Find the length of the text buffer
                while (widgetText[length] != '\0') {
                    ++length;
                }

                // Shift elements to the right
                while (length > cursorPosition) {
                    widgetText[length + 1] = widgetText[length];
                    --length;
                }

                // Insert the new character
                widgetText[cursorPosition + 1] = widgetText[cursorPosition];
                widgetText[cursorPosition] = (uint8_t)keyDownCode;
                cursorPosition++;
            }else{

                // Dont want to redraw the widget if nothing has changed
                return;
            }
            break;
        }
    }

    // Redraw the widget
    invalidate();

    // Fire the text changed event
    if(keyDownCode != KeyCode::leftArrow && keyDownCode != KeyCode::rightArrow)
        raiseEvent(new InputBoxTextChangedEvent(&widgetText[0]));

}

void InputBox::updateText(string newText) {

    // Rewrite the text, start at the beginning
    cursorPosition = 0;

    // Copy the new text into the widget text
    for(string c = (string)newText, *buffer = &widgetText[0]; *c != '\0'; ++c, buffer++)
    {

        // Update the cursor position and the buffer
        cursorPosition++;
        *buffer = *c;
    }

    // Write the null terminator
    widgetText[cursorPosition] = '\0';

    // Redraw the widget
    invalidate();

    // Fire the text changed event
    raiseEvent(new InputBoxTextChangedEvent(newText));

}

string InputBox::getText() {
    return &widgetText[0];
}

/// ___ Events ___ ///

InputBoxTextChangedEvent::InputBoxTextChangedEvent(string newText)
: Event(INPUTBOX_TEXT_CHANGED) {
    this->newText = newText;
}

InputBoxTextChangedEvent::~InputBoxTextChangedEvent() {

}
