//
// Created by 98max on 11/10/2023.
//

#ifndef MAXOS_GUI_WIDGETS_INPUTBOX_H
#define MAXOS_GUI_WIDGETS_INPUTBOX_H

#include <common/types.h>
#include <gui/widget.h>
#include <gui/font.h>

namespace maxOS{

    namespace gui {

        namespace widgets {

            enum InputBoxEvents{
                INPUTBOX_TEXT_CHANGED
            };

            class InputBoxTextChangedEvent : public common::Event<InputBoxEvents>{
            public:
                InputBoxTextChangedEvent(common::string newText);
                ~InputBoxTextChangedEvent();

                common::string newText;
            };

            class InputBoxEventHandler : public common::EventHandler<InputBoxEvents>{
            public:
                InputBoxEventHandler();
                ~InputBoxEventHandler();

                virtual void onEvent(common::Event<InputBoxEvents>* event);

                virtual void onInputBoxTextChanged(common::string newText);
            };

            class InputBox : public Widget, public common::EventManager<InputBoxEvents>{

                protected:
                    char widgetText[256];       // Replace with a buffer in memory later

                public:
                    InputBox(common::int32_t left, common::int32_t top, common::uint32_t width, common::uint32_t height);
                    InputBox(common::int32_t left, common::int32_t top, common::uint32_t width, common::uint32_t height, common::string text);
                    ~InputBox();

                    void draw(common::GraphicsContext* gc, common::Rectangle<int>& area);

                    void onFocus();
                    void onFocusLost();

                    void onKeyDown(drivers::peripherals::KeyCode keyDownCode, drivers::peripherals::KeyboardState keyDownState);

                    void updateText(common::string newText);
                    common::string getText();

                    // InputBox Variables
                    common::Colour backgroundColour;
                    common::Colour foregroundColour;
                    common::Colour borderColour;
                    gui::AmigaFont font;
                    common::uint32_t cursorPosition;

            };
        }
    }
}

#endif //MAXOS_GUI_WIDGETS_INPUTBOX_H
