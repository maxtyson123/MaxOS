//
// Created by 98max on 11/10/2023.
//

#ifndef MAXOS_GUI_WIDGETS_INPUTBOX_H
#define MAXOS_GUI_WIDGETS_INPUTBOX_H

#include <stdint.h>
#include <gui/widget.h>
#include <gui/font.h>
#include <common/string.h>

namespace MaxOS{

    namespace gui {

        namespace widgets {

			/**
			 * @enum InputBoxEvents
			 * @brief The events that an input box can trigger
			 */
            enum class InputBoxEvents{
                TEXT_CHANGED
            };

            /**
             * @class InputBoxTextChangedEvent
             * @brief Event that is triggered when the text in an input box is changed
             */
            class InputBoxTextChangedEvent : public common::Event<InputBoxEvents>{
              public:
                  InputBoxTextChangedEvent(const string&);
                  ~InputBoxTextChangedEvent();

                  string new_text;  ///< The new text in the input box
            };

            /**
             * @class InputBoxEventHandler
             * @brief Handles input box events
             */
            class InputBoxEventHandler : public common::EventHandler<InputBoxEvents>{
            public:
                InputBoxEventHandler();
                ~InputBoxEventHandler();

                common::Event<InputBoxEvents>* on_event(common::Event<InputBoxEvents>* event) override;

                virtual void on_input_box_text_changed(string);
            };

            /**
             * @class InputBox
             * @brief A box that can be used to input text
             */
            class InputBox : public Widget, public common::EventManager<InputBoxEvents>{

                protected:
                    string m_widget_text = "                                                                          ";    ///< The text in the input box

                public:
                    InputBox(int32_t left, int32_t top, uint32_t width, uint32_t height);
                    InputBox(int32_t left, int32_t top, uint32_t width, uint32_t height, const string& text);
                    ~InputBox();

                    void draw(common::GraphicsContext* gc, common::Rectangle<int32_t>& area) override;

                    void on_focus() override;
                    void on_focus_lost() override;

                    void on_key_down(drivers::peripherals::KeyCode key_down_code, drivers::peripherals::KeyboardState key_down_state) override;

                    void update_text(const string&);
                    string text();

                    // InputBox Variables
                    common::Colour background_colour;           ///< The background colour of the input box
                    common::Colour foreground_colour;           ///< The colour of the text in the input box
                    common::Colour border_colour;               ///< The colour of the bar around the input box
                    gui::Font font;                             ///< The font to use for the input box text
                    int32_t cursor_position { 0 };              ///< How many characters into the text the cursor is

            };
        }
    }
}

#endif //MAXOS_GUI_WIDGETS_INPUTBOX_H
