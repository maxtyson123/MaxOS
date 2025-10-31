//
// Created by 98max on 4/15/2023.
//

#include <gui/font.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::gui;

/**
 * @brief Construct a new Font object
 *
 * @param font_data The 8x8 font data to use
 */
Font::Font(const uint8_t* font_data) {

	// Store the font data
	for (int i = 0; i < 2048; ++i) {
		m_font8x8[i] = font_data[i];
	}
}

Font::~Font() = default;

/**
 * @brief write the entire text to the screen
 * @param x The x coordinate of the text
 * @param y The y coordinate of the text
 * @param foreground_colour The letter colour
 * @param background_colour The background colour
 * @param context The graphics context to draw the text on
 * @param text The text to draw
 */
void Font::draw_text(int32_t x, int32_t y, common::Colour foreground_colour,
					 common::Colour background_colour,
					 common::GraphicsContext* context, string text) {

	// Calculate the rectangle of the text
	int32_t top = 0;
	int32_t left = 0;
	int32_t width = get_text_width(text);
	int32_t height = get_text_height(text);

	// Create the rectangle
	Rectangle<int32_t> text_area(left, top, width, height);

	// Draw the text
	draw_text(x, y, foreground_colour, background_colour, context, text, text_area);
}


/**
 * @brief write the entire text to the screen
 *
 * @param x The x coordinate of the text
 * @param y The y coordinate of the text
 * @param foreground_colour The letter colour
 * @param background_colour The background colour
 * @param context The graphics context to draw the text on
 * @param text The text to draw
 * @param limitArea The area of the text to draw
 */
void Font::draw_text(int32_t x, int32_t y, common::Colour foreground_colour,
					 common::Colour background_colour,
					 common::GraphicsContext* context, string text,
					 common::Rectangle<int32_t> limitArea) {

	// Convert the colours
	uint32_t foreground = context->colour_to_int(foreground_colour);
	uint32_t background = context->colour_to_int(background_colour);

	// Ensure the area is within the actual area of the text
	if (limitArea.top < 0) {
		limitArea.height += limitArea.top;
		limitArea.top = 0;
	}

	if (limitArea.left < 0) {
		limitArea.width += limitArea.left;
		limitArea.left = 0;
	}

	// Clamp the height and width max
	if (limitArea.top + limitArea.height > (int) get_text_height(text))
		limitArea.height = get_text_height(text) - limitArea.top;

	if (limitArea.left + limitArea.width > (int) get_text_width(text))
		limitArea.width = get_text_width(text) - limitArea.left;

	// Calculate limits
	int32_t xLimit = limitArea.left + limitArea.width;
	int32_t yLimit = limitArea.top + limitArea.height;

	// Draw the text from top to bottom
	for (int yBitMapOffset = limitArea.top; yBitMapOffset < yLimit; yBitMapOffset++) {
		for (int xBitMapOffset = limitArea.left; xBitMapOffset < xLimit; ++xBitMapOffset) {

			// If the y is the middle then add a strikethrough
			if (is_strikethrough && yBitMapOffset == yLimit / 2) {

				// Draw the pixel
				context->putPixel(x + xBitMapOffset, y + yBitMapOffset, foreground);
				continue;
			}

			// If the y is the bottom then add an underline
			if (is_underlined && yBitMapOffset == yLimit - 1) {

				// Draw the pixel
				context->putPixel(x + xBitMapOffset, y + yBitMapOffset, foreground);
				continue;
			}

			//TODO: Bold, Italic when other fonts are working

			// Get the character
			uint8_t character = text[xBitMapOffset / 8];

			// Check if this pixel  is set or not
			bool set = m_font8x8[(uint16_t) character * 8 + yBitMapOffset] & (128 >> (xBitMapOffset % 8));

			// Draw the pixel
			context->putPixel(x + xBitMapOffset, y + yBitMapOffset, set ? foreground : background);

		}
	}
}

/**
 * @brief Get the height of the text
 *
 * @param text The text to get the height of
 * @return The height of the text
 */
int32_t Font::get_text_height(string text) {

	return 8;

}

/**
 * @brief Get the width of the text
 *
 * @param text The text to get the width of
 * @return The width of the text
 */
int32_t Font::get_text_width(string text) {

	return text.length() * 8;
}
