//
// Created by 98max on 10/10/2023.
//

#ifndef MAXOS_GUI_WIDGETS_BUTTON_H
#define MAXOS_GUI_WIDGETS_BUTTON_H

#include <stdint.h>
#include <common/eventHandler.h>
#include <gui/widget.h>
#include <gui/font.h>


namespace MaxOS {

    namespace gui {

        namespace widgets {

            //forward declaration
            class Button;

            enum class ButtonEvents{
                PRESSED,
                RELEASED
            };

            /**
             * @class ButtonPressedEvent
             * @brief Event that is triggered when a button is pressed
             */
            class ButtonPressedEvent : public common::Event<ButtonEvents>{
                public:
                    explicit ButtonPressedEvent(Button*);
                    ~ButtonPressedEvent();

                    Button* source;
            };

            /**
             * @class ButtonReleasedEvent
             * @brief Event that is triggered when a button is released
             */
            class ButtonReleasedEvent : public common::Event<ButtonEvents>{
                public:
                    explicit ButtonReleasedEvent(Button*);
                    ~ButtonReleasedEvent();

                    Button* source;
            };

            /**
             * @class ButtonEventHandler
             * @brief Handles button events
             */
            class ButtonEventHandler : public common::EventHandler<ButtonEvents>{
                public:
                    ButtonEventHandler();
                    ~ButtonEventHandler();

                    common::Event<ButtonEvents>* on_event(common::Event<ButtonEvents>*) override;

                    virtual void on_button_pressed(Button* source);
                    virtual void on_button_released(Button* source);
            };

            /**
             * @class Button
             * @brief A button widget, can be clicked
             */
            class Button : public Widget, public common::EventManager<ButtonEvents> {

                public:
                    Button(int32_t left, int32_t top, uint32_t width, uint32_t height, const string& text);
                    ~Button();

                    // Widget Stuff
                    void draw(common::GraphicsContext* gc, common::Rectangle<int32_t>& area) override;
                    drivers::peripherals::MouseEventHandler* on_mouse_button_pressed(uint32_t x, uint32_t y, uint8_t button) override;
                    void on_mouse_button_released(uint32_t x, uint32_t y, uint8_t button) override;

                    // Button Stuff
                    common::Colour background_colour;
                    common::Colour foreground_colour;
                    common::Colour border_colour;
                    gui::Font font;
                    string text;
            };
        }
    }

}

#endif //MAXOS_GUI_WIDGETS_BUTTON_H
