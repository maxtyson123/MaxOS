//
// Created by 98max on 11/10/2023.
//

#ifndef MAXOS_GUI_WIDGETS_INPUTBOX_H
#define MAXOS_GUI_WIDGETS_INPUTBOX_H

#include <stdint.h>
#include <gui/widget.h>
#include <gui/font.h>
#include <common/string.h>

namespace maxOS{

    namespace gui {

        namespace widgets {

            enum InputBoxEvents{
                INPUTBOX_TEXT_CHANGED
            };

            class InputBoxTextChangedEvent : public common::Event<InputBoxEvents>{
            public:
                InputBoxTextChangedEvent(string newText);
                ~InputBoxTextChangedEvent();

                string newText;
            };

            class InputBoxEventHandler : public common::EventHandler<InputBoxEvents>{
            public:
                InputBoxEventHandler();
                ~InputBoxEventHandler();

                virtual common::Event<InputBoxEvents>* onEvent(common::Event<InputBoxEvents>* event);

                virtual void onInputBoxTextChanged(string newText);
            };

            class InputBox : public Widget, public common::EventManager<InputBoxEvents>{

                protected:
                    char widgetText[256];       // Replace with a buffer in memory later

                public:
                    InputBox(int32_t left, int32_t top, uint32_t width, uint32_t height);
                    InputBox(int32_t left, int32_t top, uint32_t width, uint32_t height, string text);
                    ~InputBox();

                    void draw(common::GraphicsContext* gc, common::Rectangle<int32_t>& area);

                    void onFocus();
                    void onFocusLost();

                    void onKeyDown(drivers::peripherals::KeyCode keyDownCode, drivers::peripherals::KeyboardState keyDownState);

                    void updateText(string newText);
                    string getText();

                    // InputBox Variables
                    common::Colour backgroundColour;
                    common::Colour foregroundColour;
                    common::Colour borderColour;
                    gui::AmigaFont font;
                    uint32_t cursorPosition;

            };
        }
    }
}

#endif //MAXOS_GUI_WIDGETS_INPUTBOX_H
