//
// Created by 98max on 10/16/2022.
//

#include <gui/font/amiga_font.h>
#include <gui/widgets/text.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::gui;
using namespace MaxOS::gui::widgets;

Text::Text(int32_t left, int32_t top, uint32_t width, uint32_t height, const string& text)
: Widget(left, top, width, height),
  font((uint8_t*) AMIGA_FONT),
  foreground_colour(Colour(0, 0, 0)),
  background_colour(Colour(255, 255, 255))
{

	// Set the text
	update_text(text);
}

Text::~Text() = default;

/**
 * @brief Draw the text on the screen
 *
 * @param gc The graphics context to draw on
 * @param area The area of the text to draw
 */
void Text::draw(GraphicsContext* gc, Rectangle<int32_t>& area) {

	// Default Draw Operation
	Widget::draw(gc, area);

	// Get the absolute m_position of the text
	Coordinates textCoordinates = absolute_coordinates(Coordinates(0, 0));
	Rectangle<int32_t> textPosition = position();

	// Get the x and y m_position of the text
	int32_t x = textCoordinates.first;
	int32_t y = textCoordinates.second;

	// Draw the background (as the text might not fill the entire area)
	gc->fill_rectangle(x + area.left, y + area.top, x + area.left + area.width, y + area.top + area.height, background_colour);

	// Draw the text
	this->font.draw_text(x, y, foreground_colour, background_colour, gc, m_widget_text, area);
}

/**
 * @brief Update the text of the widget
 * @param new_text The new text to display
 */
void Text::update_text(const string& new_text) {

	// Set the text
	m_widget_text.copy(new_text);
	invalidate();

}
