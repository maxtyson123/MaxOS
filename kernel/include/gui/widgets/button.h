//
// Created by 98max on 10/10/2023.
//

#ifndef MAXOS_GUI_WIDGETS_BUTTON_H
#define MAXOS_GUI_WIDGETS_BUTTON_H

#include <stdint.h>
#include <common/eventHandler.h>
#include <gui/widget.h>
#include <gui/font.h>


namespace maxOS {

    namespace gui {

        namespace widgets {

            //forward declaration
            class Button;

            enum ButtonEvents{
                BUTTON_PRESSED,
                BUTTON_RELEASED
            };

            class ButtonPressedEvent : public common::Event<ButtonEvents>{
            public:
                ButtonPressedEvent(Button* source);
                ~ButtonPressedEvent();

                Button* source;
            };

            class ButtonReleasedEvent : public common::Event<ButtonEvents>{
            public:
                ButtonReleasedEvent(Button* source);
                ~ButtonReleasedEvent();

                Button* source;
            };

            class ButtonEventHandler : public common::EventHandler<ButtonEvents>{
            public:
                ButtonEventHandler();
                ~ButtonEventHandler();

                virtual common::Event<ButtonEvents>* onEvent(common::Event<ButtonEvents>* event);

                virtual void onButtonPressed(Button* source);
                virtual void onButtonReleased(Button* source);
            };

            class Button : public Widget, public common::EventManager<ButtonEvents> {

                public:
                    Button(int32_t left, int32_t top, uint32_t width, uint32_t height, string text);
                    ~Button();

                    // Widget Stuff
                    void draw(common::GraphicsContext* gc, common::Rectangle<int>& area);
                    drivers::peripherals::MouseEventHandler* onMouseButtonPressed(uint32_t x, uint32_t y, uint8_t button);
                    void onMouseButtonReleased(uint32_t x, uint32_t y, uint8_t button);

                    // Button Stuff
                    common::Colour backgroundColour;
                    common::Colour foregroundColour;
                    common::Colour borderColour;
                    gui::AmigaFont font;
                    string text;

            };
        }
    }

}

#endif //MAXOS_GUI_WIDGETS_BUTTON_H
