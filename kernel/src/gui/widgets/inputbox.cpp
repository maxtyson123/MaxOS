//
// Created by 98max on 11/10/2023.
//

#include <gui/widgets/inputbox.h>
#include <gui/font/amiga_font.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::gui;
using namespace MaxOS::gui::widgets;
using namespace MaxOS::drivers::peripherals;

InputBoxEventHandler::InputBoxEventHandler() = default;

InputBoxEventHandler::~InputBoxEventHandler() = default;

/**
 * @brief Delegates an event to the event handler for that event type
 *
 * @param event The event triggered
 * @return The event triggered potentially modified by the handler.
 */
Event<InputBoxEvents>* InputBoxEventHandler::on_event(Event<InputBoxEvents>* event) {

	switch (event->type) {
		case InputBoxEvents::TEXT_CHANGED:
			on_input_box_text_changed(((InputBoxTextChangedEvent*) event)->new_text);
			break;
	}

	return event;
}

/**
 * @brief Event triggered when the text in the input box is changed
 */
void InputBoxEventHandler::on_input_box_text_changed(string) {

}

InputBox::InputBox(int32_t left, int32_t top, uint32_t width, uint32_t height)
: Widget(left, top, width, height),
  background_colour(Colour(0xFF, 0xFF, 0xFF)),
  foreground_colour(Colour(0x00, 0x00, 0x00)),
  border_colour(Colour(0x57, 0x57, 0x57)),
  font((uint8_t*) AMIGA_FONT)
{

}

InputBox::InputBox(int32_t left, int32_t top, uint32_t width, uint32_t height, const string& text)
: Widget(left, top, width, height),
  background_colour(Colour(0xFF, 0xFF, 0xFF)),
  foreground_colour(Colour(0x00, 0x00, 0x00)),
  border_colour(Colour(0x57, 0x57, 0x57)),
  font((uint8_t*) AMIGA_FONT)
{

	// Update the text
	update_text(text);

}

InputBox::~InputBox() = default;

/**
 * @brief Draw an area of the input box onto a graphics context
 *
 * @param gc The context to draw onto
 * @param area What part of the input box to draw
 */
void InputBox::draw(GraphicsContext* gc, Rectangle<int32_t>& area) {

	// Default Draw
	Widget::draw(gc, area);

	// Get the absolute m_position of the input box
	Coordinates inputBoxCoordinates = absolute_coordinates(Coordinates(0, 0));
	Rectangle<int32_t> inputBoxPosition = position();

	// Get the x and y m_position of the input box
	int32_t x = inputBoxCoordinates.first;
	int32_t y = inputBoxCoordinates.second;

	// Draw the background for the input box
	gc->fill_rectangle(x + area.left, y + area.top, x + area.left + area.width,
					   y + area.top + area.height, background_colour);

	// Draw the border  (TODO: Make this a function because it is used in multiple places)

	// Top Border
	if (area.intersects(Rectangle<int32_t>(0, 0, inputBoxPosition.width, 1))) {

		// Start in the top left corner of the button and end in the top right corner
		gc->draw_line(x + area.left, y, x + area.left + area.width - 1, y,
					  border_colour);
	}

	// Left Border
	if (area.intersects(Rectangle<int32_t>(0, 0, 1, inputBoxPosition.height))) {

		// Start in the top left corner and end in the bottom left corner
		gc->draw_line(x, y + area.top, x, y + area.top + area.height - 1,
					  border_colour);
	}

	// Right Border
	if (area.intersects(Rectangle<int32_t>(0, inputBoxPosition.height - 1, inputBoxPosition.width, 1))) {

		// Start in the top right corner and end in the bottom right corner
		gc->draw_line(x + area.left, y + inputBoxPosition.height - 1,
					  x + area.left + area.width - 1,
					  y + inputBoxPosition.height - 1, border_colour);
	}

	// Bottom Border
	if (area.intersects(Rectangle<int32_t>(inputBoxPosition.width - 1, 0, 1, inputBoxPosition.height))) {

		// Start in the bottom left corner and end in the bottom right corner
		gc->draw_line(x + inputBoxPosition.width - 1, y + area.top,
					  x + inputBoxPosition.width - 1,
					  y + area.top + area.height - 1, border_colour);
	}

	// Draw the text
	common::Rectangle<int32_t> textArea(area.left - 1, area.top - 1, area.width, area.height);
	font.draw_text(x + 1, y + 1, foreground_colour, background_colour, gc, m_widget_text, textArea);
}

/**
 * @brief Update the border when the input box is focussed
 */
void InputBox::on_focus() {

	// Make the border black on focus
	border_colour = Colour(0x00, 0x00, 0x00);
	invalidate();
}

/**
 * @brief Reset to the original input box style when it is no longer focussed
 */
void InputBox::on_focus_lost() {

	// Reset the border colour
	border_colour = Colour(0x57, 0x57, 0x57);
	invalidate();
}

/**
 * @brief Handles a keypress event by updating the rendered text in the input box
 *
 * @param key_down_code The key being pressed
 * @param key_down_state The state of the key being pressed
 */
void InputBox::on_key_down(KeyCode key_down_code, KeyboardState key_down_state) {

	// Handle the key press
	switch (key_down_code) {
		case KeyCode::backspace: {
			if (cursor_position == 0)
				break;

			cursor_position--;
			// no break - we move the cursor to the left and use the <Delete> code
			[[fallthrough]];
		}
		case KeyCode::deleteKey: {
			// Move the text to the left
			for (int i = cursor_position; i < m_widget_text.length(); ++i)
				m_widget_text[i] = m_widget_text[i + 1];

			// Put a null character at the end of the string
			m_widget_text[m_widget_text.length() - 1] = '\0';

			break;
		}
		case KeyCode::leftArrow: {
			// If the cursor is not at the beginning of the text, move it to the left
			if (cursor_position > 0)
				cursor_position--;
			break;
		}
		case KeyCode::rightArrow: {

			// If the cursor is not at the end of the text, move it to the right
			if (m_widget_text[cursor_position] != '\0')
				cursor_position++;
			break;
		}
		default: {

			// If the key is a printable character, add it to the text
			if (31 < (int) key_down_code && (int) key_down_code < 127) {
				uint32_t length = cursor_position;

				// Find the length of the text buffer
				while (m_widget_text[length] != '\0') {
					++length;
				}

				// Check if we need to make space for the new character
				if (length >= (uint32_t) m_widget_text.length()) {
					m_widget_text += " ";
				}

				// Shift elements to the right
				while (length > cursor_position) {
					m_widget_text[length + 1] = m_widget_text[length];
					--length;
				}

				// Insert the new character
				m_widget_text[cursor_position + 1] = m_widget_text[cursor_position];
				m_widget_text[cursor_position] = (uint8_t) key_down_code;
				cursor_position++;
			} else {

				// Don't want to redraw the widget if nothing has changed
				return;
			}
			break;
		}
	}

	// Redraw the widget
	invalidate();

	// Fire the text changed event
	if (key_down_code != KeyCode::leftArrow && key_down_code != KeyCode::rightArrow)
		raise_event(new InputBoxTextChangedEvent(m_widget_text));

}

/**
 * @brief Update the text in the input box
 *
 * @param new_text The new text to display
 */
void InputBox::update_text(const string& new_text) {

	m_widget_text.copy(new_text);
	cursor_position = m_widget_text.length();

	// Redraw the widget
	invalidate();

	// Fire the text changed event
	raise_event(new InputBoxTextChangedEvent(new_text));
}

/**
 * @brief Get the text currently in the input box
 *
 * @return The text in the input box
 */
string InputBox::text() {

	return m_widget_text;
}


InputBoxTextChangedEvent::InputBoxTextChangedEvent(const string& new_text)
: Event(InputBoxEvents::TEXT_CHANGED),
  new_text(new_text)
{

}

InputBoxTextChangedEvent::~InputBoxTextChangedEvent() = default;