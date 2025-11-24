/**
 * @file font.cpp
 * @brief Implementation of a Font class for rendering text in a graphics context
 *
 * @date 15th April 2023
 * @author Max Tyson
 *
 * @todo Replace with a proper font format
 */

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
	for(int i = 0; i < 2048; ++i) {
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
 * @param limit_area The area of the text to draw
 */
void Font::draw_text(int32_t x, int32_t y, common::Colour foreground_colour,
                     common::Colour background_colour,
                     common::GraphicsContext* context, string text,
                     common::Rectangle<int32_t> limit_area) {

	// Convert the colours
	uint32_t foreground = context->colour_to_int(foreground_colour);
	uint32_t background = context->colour_to_int(background_colour);

	// Ensure the area is within the actual area of the text
	if(limit_area.top < 0) {
		limit_area.height += limit_area.top;
		limit_area.top = 0;
	}

	if(limit_area.left < 0) {
		limit_area.width += limit_area.left;
		limit_area.left = 0;
	}

	// Clamp the height and width max
	if(limit_area.top + limit_area.height > (int) get_text_height(text))
		limit_area.height = get_text_height(text) - limit_area.top;

	if(limit_area.left + limit_area.width > (int) get_text_width(text))
		limit_area.width = get_text_width(text) - limit_area.left;

	// Calculate limits
	int32_t x_limit = limit_area.left + limit_area.width;
	int32_t y_limit = limit_area.top + limit_area.height;

	// Draw the text from top to bottom
	for(int y_bit_map_offset = limit_area.top; y_bit_map_offset < y_limit; y_bit_map_offset++) {
		for(int x_bit_map_offset = limit_area.left; x_bit_map_offset < x_limit; ++x_bit_map_offset) {

			// If the y is the middle then add a strikethrough
			if(is_strikethrough && y_bit_map_offset == y_limit / 2) {

				// Draw the pixel
				context->put_pixel(x + x_bit_map_offset, y + y_bit_map_offset, foreground);
				continue;
			}

			// If the y is the bottom then add an underline
			if(is_underlined && y_bit_map_offset == y_limit - 1) {

				// Draw the pixel
				context->put_pixel(x + x_bit_map_offset, y + y_bit_map_offset, foreground);
				continue;
			}

			// Get the character
			uint8_t character = text[x_bit_map_offset / 8];

			// Check if this pixel  is set or not
			bool set = m_font8x8[(uint16_t) character * 8 + y_bit_map_offset] & (128 >> (x_bit_map_offset % 8));

			// Draw the pixel
			context->put_pixel(x + x_bit_map_offset, y + y_bit_map_offset, set ? foreground : background);

		}
	}
}

/**
 * @brief Get the height of the text
 *
 * @param text The text to get the height of
 * @return The height of the text
 */
size_t Font::get_text_height(string text) {

	return 8;

}

/**
 * @brief Get the width of the text
 *
 * @param text The text to get the width of
 * @return The width of the text
 */
size_t Font::get_text_width(string text) {

	return text.length() * 8;
}
