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

            /**
             * @class InputBoxTextChangedEvent
             * @brief Event that is triggered when the text in an input box is changed
             */
            class InputBoxTextChangedEvent : public common::Event<InputBoxEvents>{
              public:
                  InputBoxTextChangedEvent(string);
                  ~InputBoxTextChangedEvent();

                  string new_text;
            };

            /**
             * @class InputBoxEventHandler
             * @brief Handles input box events
             */
            class InputBoxEventHandler : public common::EventHandler<InputBoxEvents>{
            public:
                InputBoxEventHandler();
                ~InputBoxEventHandler();

                virtual common::Event<InputBoxEvents>* on_event(common::Event<InputBoxEvents>* event) override;

                virtual void on_input_box_text_changed(string);
            };

            /**
             * @class InputBox
             * @brief A box that can be used to input text
             */
            class InputBox : public Widget, public common::EventManager<InputBoxEvents>{

                protected:
                    string m_widget_text = "                                                                          ";

                public:
                    InputBox(int32_t left, int32_t top, uint32_t width, uint32_t height);
                    InputBox(int32_t left, int32_t top, uint32_t width, uint32_t height, string text);
                    ~InputBox();

                    void draw(common::GraphicsContext* gc, common::Rectangle<int32_t>& area) override;

                    void on_focus() override;
                    void on_focus_lost() override;

                    void on_key_down(drivers::peripherals::KeyCode keyDownCode, drivers::peripherals::KeyboardState keyDownState) override;

                    void update_text(string);
                    string get_text();

                    // InputBox Variables
                    common::Colour background_colour;
                    common::Colour foreground_colour;
                    common::Colour border_colour;
                    gui::AmigaFont font;
                    uint32_t cursor_position { 0 };

            };
        }
    }
}

#endif //MAXOS_GUI_WIDGETS_INPUTBOX_H
