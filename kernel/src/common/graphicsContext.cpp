/**
 * @file graphicsContext.cpp
 * @brief Implements the GraphicsContext class defined in graphicsContext.h
 *
 * @date 30th March 2023
 * @author Max Tyson
 */

#include <common/graphicsContext.h>

using namespace MaxOS::common;


/**
 * @brief Constructs a GraphicsContext object and initializes the color palette
 */
GraphicsContext::GraphicsContext() {


	// VirtualBox VGA palette
	m_colour_pallet[0x00] = Colour(0x00, 0x00, 0x00);    // Black
	m_colour_pallet[0x01] = Colour(0x00, 0x00, 0xA8);    // Duke Blue
	m_colour_pallet[0x02] = Colour(0x00, 0xA8, 0x00);    // Islamic Green
	m_colour_pallet[0x03] = Colour(0x00, 0xA8, 0xA8);    // Persian Green
	m_colour_pallet[0x04] = Colour(0xA8, 0x00, 0x00);    // Dark Candy Apple Red
	m_colour_pallet[0x05] = Colour(0xA8, 0x00, 0xA8);    // Heliotrope Magenta

	m_colour_pallet[0x06] = Colour(0xA8, 0xA8, 0x00);    // Light Gold
	m_colour_pallet[0x07] = Colour(0xA8, 0xA8, 0xA8);    // Dark Gray (X11)
	m_colour_pallet[0x08] = Colour(0x00, 0x00, 0x57);    // Cetacean Blue
	m_colour_pallet[0x09] = Colour(0x00, 0x00, 0xFF);    // Blue
	m_colour_pallet[0x0A] = Colour(0x00, 0xA8, 0x57);    // Green (Pigment)
	m_colour_pallet[0x0B] = Colour(0x00, 0xA8, 0xFF);    // Vivid Cerulean
	m_colour_pallet[0x0C] = Colour(0xA8, 0x00, 0x57);    // Jazz berry Jam
	m_colour_pallet[0x0D] = Colour(0xA8, 0x00, 0x57);    // Jazz berry Jam
	m_colour_pallet[0x0E] = Colour(0xA8, 0xA8, 0x57);    // Olive Green
	m_colour_pallet[0x0F] = Colour(0xA8, 0xA8, 0xFF);    // Maximum Blue Purple

	m_colour_pallet[0x10] = Colour(0x00, 0x57, 0x00);    // Dark Green (X11)
	m_colour_pallet[0x11] = Colour(0x00, 0x57, 0xA8);    // Cobalt Blue
	m_colour_pallet[0x12] = Colour(0x00, 0xFF, 0x00);    // Electric Green
	m_colour_pallet[0x13] = Colour(0x00, 0xFF, 0xA8);    // Medium Spring Green
	m_colour_pallet[0x14] = Colour(0xA8, 0x57, 0x00);    // Windsor Tan
	m_colour_pallet[0x15] = Colour(0xA8, 0x57, 0xA8);    // Purpureus
	m_colour_pallet[0x16] = Colour(0xA8, 0xFF, 0x00);    // Spring Bud
	m_colour_pallet[0x17] = Colour(0xA8, 0xFF, 0xA8);    // Mint Green
	m_colour_pallet[0x18] = Colour(0x00, 0x57, 0x57);    // Midnight Green (Eagle Green)
	m_colour_pallet[0x19] = Colour(0x00, 0x57, 0xFF);    // Blue (RYB)
	m_colour_pallet[0x1A] = Colour(0x00, 0xFF, 0x57);    // Malachite
	m_colour_pallet[0x1B] = Colour(0x00, 0xFF, 0xFF);    // Aqua
	m_colour_pallet[0x1C] = Colour(0xA8, 0x57, 0x57);    // Middle Red Purple
	m_colour_pallet[0x1D] = Colour(0xA8, 0x57, 0xFF);    // Lavender Indigo
	m_colour_pallet[0x1E] = Colour(0xA8, 0xFF, 0x57);    // Olive Green
	m_colour_pallet[0x1F] = Colour(0xA8, 0xFF, 0xFF);    // Celeste

	m_colour_pallet[0x20] = Colour(0x57, 0x00, 0x00);    // Blood Red
	m_colour_pallet[0x21] = Colour(0x57, 0x00, 0xA8);    // Metallic Violet
	m_colour_pallet[0x22] = Colour(0x57, 0xA8, 0x00);    // Kelly Green
	m_colour_pallet[0x23] = Colour(0x57, 0xA8, 0xA8);    // Cadet Blue
	m_colour_pallet[0x24] = Colour(0xFF, 0x00, 0x00);    // Red
	m_colour_pallet[0x25] = Colour(0xFF, 0x00, 0xA8);    // Fashion Fuchsia
	m_colour_pallet[0x26] = Colour(0xFF, 0xA8, 0x00);    // Chrome Yellow
	m_colour_pallet[0x27] = Colour(0xFF, 0xA8, 0xA8);    // Light Salmon Pink
	m_colour_pallet[0x28] = Colour(0x57, 0x00, 0x57);    // Imperial Purple
	m_colour_pallet[0x29] = Colour(0x57, 0x00, 0xFF);    // Electric Indigo
	m_colour_pallet[0x2A] = Colour(0x57, 0xA8, 0x57);    // Apple
	m_colour_pallet[0x2B] = Colour(0x57, 0xA8, 0xFF);    // Blue Jeans
	m_colour_pallet[0x2C] = Colour(0xFF, 0x00, 0x57);    // Folly
	m_colour_pallet[0x2D] = Colour(0xFF, 0x00, 0xFF);    // Fuchsia
	m_colour_pallet[0x2E] = Colour(0xFF, 0xA8, 0x57);    // Rajah
	m_colour_pallet[0x2F] = Colour(0xFF, 0xA8, 0xFF);    // Rich Brilliant Lavender

	m_colour_pallet[0x30] = Colour(0x57, 0x57, 0x00);    // Dark Bronze (Coin)
	m_colour_pallet[0x31] = Colour(0x57, 0x57, 0xA8);    // Liberty
	m_colour_pallet[0x32] = Colour(0x57, 0xFF, 0x00);    // Chlorophyll Green
	m_colour_pallet[0x33] = Colour(0x57, 0xFF, 0xA8);    // Medium Aquamarine
	m_colour_pallet[0x34] = Colour(0xFF, 0x57, 0x00);    // Orange (Pantone)
	m_colour_pallet[0x35] = Colour(0xFF, 0x57, 0xA8);    // Brilliant Rose
	m_colour_pallet[0x36] = Colour(0xFF, 0xFF, 0x00);    // Yellow
	m_colour_pallet[0x37] = Colour(0xFF, 0xFF, 0xA8);    // Calamansi
	m_colour_pallet[0x38] = Colour(0x57, 0x57, 0x57);    // Davy's Grey
	m_colour_pallet[0x39] = Colour(0x57, 0x57, 0xFF);    // Very Light Blue
	m_colour_pallet[0x3A] = Colour(0x57, 0xFF, 0x57);    // Screamin' Green
	m_colour_pallet[0x3B] = Colour(0x57, 0xFF, 0xFF);    // Electric Blue
	m_colour_pallet[0x3C] = Colour(0xFF, 0x57, 0x57);    // Sunset Orange
	m_colour_pallet[0x3D] = Colour(0xFF, 0x57, 0xFF);    // Shocking Pink (Crayola)
	m_colour_pallet[0x3E] = Colour(0xFF, 0xFF, 0x57);    // Shocking Pink (Crayola)
	m_colour_pallet[0x3F] = Colour(0xFF, 0xFF, 0xFF);    // White


	// Set the rest of the palette to black
	for (uint8_t color_code = 255; color_code >= 0x40; --color_code)
		m_colour_pallet[color_code] = Colour(0, 0, 0);


}

GraphicsContext::~GraphicsContext() = default;

/**
 * @brief Renders a pixel to the screen based on the current color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The colour of the pixel
 */
void GraphicsContext::render_pixel(uint32_t x, uint32_t y, uint32_t colour) {

	// Call the correct put_pixel function based on the color depth
	switch (m_color_depth) {
		case 8:
			render_pixel_8_bit(x, y, colour);
			break;
		case 16:
			render_pixel_16_bit(x, y, colour);
			break;
		case 24:
			render_pixel_24_bit(x, y, colour);
			break;
		case 32:
			render_pixel_32_bit(x, y, colour);
			break;
	}


}

/**
 * @brief Renders a pixel to the screen using the 8 bit color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The 8Bit colour of the pixel
 */
void GraphicsContext::render_pixel_8_bit(uint32_t, uint32_t, uint8_t) {

}

/**
 * @brief Renders a pixel to the screen using the 16 bit color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The 16Bit colour of the pixel
 */
void GraphicsContext::render_pixel_16_bit(uint32_t, uint32_t, uint16_t) {

}

/**
 * @brief Renders a pixel to the screen using the 24 bit color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The 24Bit colour of the pixel
 */
void GraphicsContext::render_pixel_24_bit(uint32_t, uint32_t, uint32_t) {

}

/**
 * @brief Renders a pixel to the screen using the 32 bit color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The 32Bit colour of the pixel
 */
void GraphicsContext::render_pixel_32_bit(uint32_t, uint32_t, uint32_t) {

}

/**
 * @brief Gets the colour of a pixel on the screen, automatically uses the correct color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @return The colour of the pixel or white if the pixel is not supported
 */
uint32_t GraphicsContext::get_rendered_pixel(uint32_t x, uint32_t y) {
	// Call the correct get_pixel function based on the color depth
	switch (m_color_depth) {
		case 8:
			return get_rendered_pixel_8_bit(x, y);
		case 16:
			return get_rendered_pixel_16_bit(x, y);
		case 24:
			return get_rendered_pixel_24_bit(x, y);
		case 32:
			return get_rendered_pixel_32_bit(x, y);
	}

	return colour_to_int(Colour(0xFF, 0xFF, 0xFF));
}

/**
 * @brief Gets the colour of a pixel on the screen using the 8 bit color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @return The 8Bit colour of the pixel
 */
uint8_t GraphicsContext::get_rendered_pixel_8_bit(uint32_t, uint32_t) {
	return 0;
}

/**
 * @brief Gets the colour of a pixel on the screen using the 16 bit color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @return The 16Bit colour of the pixel
 */
uint16_t GraphicsContext::get_rendered_pixel_16_bit(uint32_t, uint32_t) {
	return 0;
}

/**
 * @brief Gets the colour of a pixel on the screen using the 24 bit color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @return The 24Bit colour of the pixel
 */
uint32_t GraphicsContext::get_rendered_pixel_24_bit(uint32_t, uint32_t) {
	return 0;
}

/**
 * @brief Gets the colour of a pixel on the screen using the 32 bit color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @return The 32Bit colour of the pixel
 */
uint32_t GraphicsContext::get_rendered_pixel_32_bit(uint32_t, uint32_t) {
	return 0;
}

/**
 * @brief Converts a colour to an integer value based on the current color depth
 *
 * @param colour The colour class to convert
 * @return The integer value of the colour
 */
uint32_t GraphicsContext::colour_to_int(const Colour &colour) {

	switch (m_color_depth) {
		case 8: {
			uint32_t result = 0;
			int mindistance = 0xfffffff;
			for (uint32_t i = 0; i <= 255; ++i) {
				Colour *c = &m_colour_pallet[i];
				int distance =
						((int) colour.red - (int) c->red) * ((int) colour.red - (int) c->red)
						+ ((int) colour.green - (int) c->green) * ((int) colour.green - (int) c->green)
						+ ((int) colour.blue - (int) c->blue) * ((int) colour.blue - (int) c->blue);
				if (distance < mindistance) {
					mindistance = distance;
					result = i;
				}
			}
			return result;
		}
		case 16: {
			// 16-Bit colours RRRRRGGGGGGBBBBB
			return ((uint16_t) (colour.red & 0xF8)) << 8
				   | ((uint16_t) (colour.green & 0xFC)) << 3
				   | ((uint16_t) (colour.blue & 0xF8) >> 3);
		}
		case 24: {
			return (uint32_t) colour.red << 16
				   | (uint32_t) colour.green << 8
				   | (uint32_t) colour.blue;
		}
		default:
		case 32: {
			uint32_t red_hex = ((uint32_t) colour.red & 0xFF) << 16;
			uint32_t green_hex = ((uint32_t) colour.green & 0xFF) << 8;
			uint32_t blue_hex = (uint32_t) colour.blue & 0xFF;
			uint32_t alpha_hex = ((uint32_t) colour.alpha & 0xFF) << 24;

			uint32_t hexValue = red_hex | green_hex | blue_hex | alpha_hex;


			return hexValue;
		}
	}
}

/**
 * @brief Converts an integer value to a colour based on the current color depth
 *
 * @param colour The integer value to convert
 * @return The colour class of the integer value
 */
Colour GraphicsContext::int_to_colour(uint32_t colour) {
	switch (m_color_depth) {

		case 8: {
			// Return the colour from the palette
			return m_colour_pallet[colour & 0xFF];
		}

		case 16: {
			// 16-Bit Colour: 5 bits for red, 6 bits for green, 5 bits for blue (RRRRR,GGGGGG,BBBBB)
			Colour result;

			result.red = (colour & 0xF800) >> 8;
			result.green = (colour & 0x07E0) >> 3;
			result.blue = (colour & 0x001F) << 3;

			return result;
		}

		case 24: {
			// 24-Bit Colour: 8 bits for red, 8 bits for green, 8 bits for blue (RRRRRRRR,GGGGGGGG,BBBBBBBB)
			Colour result;

			result.red = (colour & 0xFF0000) >> 16;
			result.green = (colour & 0x00FF00) >> 8;
			result.blue = (colour & 0x0000FF);

			return result;
		}

		default:
		case 32: {
			Colour result;

			uint32_t hex_value = colour;
			result.red = (hex_value >> 16) & 0xFF;
			result.green = (hex_value >> 8) & 0xFF;
			result.blue = hex_value & 0xFF;
			result.alpha = (hex_value >> 24) & 0xFF;

			return result;

		}
	}
}

/**
 * @brief Gets the width of the screen
 *
 * @return The width of the screen
 */
uint32_t GraphicsContext::width() const {
	return m_width;
}

/**
 * @brief Gets the height of the screen
 *
 * @return The height of the screen
 */
uint32_t GraphicsContext::height() const {
	return m_height;
}

/**
 * @brief Gets the current color depth (bits per pixel)
 * @return The color depth
 */
uint32_t GraphicsContext::color_depth() const {
	return m_color_depth;
}

/**
 * @brief Renders a pixel to the screen (automatically converts the colour to an integer)
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The colour of the pixel
 */
void GraphicsContext::put_pixel(int32_t x, int32_t y, const Colour &colour) {

	// Convert the colour to an integer and then print it
	putPixel(x, y, colour_to_int(colour));
}

/**
 * @brief Renders a pixel to the screen (automatically converts the colour to an integer), will not render the pixel if it is outside the screen
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The colour of the pixel
 */
void GraphicsContext::putPixel(int32_t x, int32_t y, uint32_t colour) {

	if (0 > x || (uint32_t) x >= m_width) {
		return;
	}

	// Check if the pixel is within the m_height of the screen
	if (0 > y || (uint32_t) y >= m_height) {
		return;
	}

	// Render the pixel
	render_pixel(x, mirror_y_axis ? m_height - y - 1 : y, colour);

}

/**
 * @brief Gets the colour of a pixel, or returns black if the pixel is outside the screen
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @return The colour of the pixel or black if the pixel is outside the screen
 */
Colour GraphicsContext::get_pixel(int32_t x, int32_t y) {

	// Check if the pixel is within the bounds of the screen
	if (0 > x || (uint32_t) x >= m_width || 0 > y || (uint32_t) y >= m_height)
		return {0, 0, 0};

	// Get the pixel and convert it to a colour
	uint32_t translated_color = get_rendered_pixel(x, mirror_y_axis ? m_height - y - 1 : y);
	return int_to_colour(translated_color);
}

/**
 * @brief Inverts a pixel
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 */
void GraphicsContext::invert_pixel(int32_t x, int32_t y) {

	// Get the pixel
	Colour colour = get_pixel(x, y);

	// Invert the pixel
	colour.red = 255 - colour.red;
	colour.green = 255 - colour.green;
	colour.blue = 255 - colour.blue;

	// Render the pixel
	put_pixel(x, y, colour);
}


/**
 * @brief Draws a line on the screen
 *
 * @param x0 The x coordinate of the m_first_memory_chunk point
 * @param y0 The y coordinate of the m_first_memory_chunk point
 * @param x1 The x coordinate of the final point
 * @param y1 The y coordinate of the final point
 * @param colour The colour of the line
 */
void GraphicsContext::draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Colour &colour) {
	drawLine(x0, y0, x1, y1, colour_to_int(colour));
}

/**
 * @brief Draws a line on the screen
 *
 * @param x0 The x coordinate of the m_first_memory_chunk point
 * @param y0 The y coordinate of the m_first_memory_chunk point
 * @param x1 The x coordinate of the final point
 * @param y1 The y coordinate of the final point
 * @param colour The colour of the line
 */
void GraphicsContext::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t colour) {

	// Store the minimum and maximum y values
	bool y_0_is_smaller = y0 < y1;
	int32_t y_min = y_0_is_smaller ? y0 : y1;
	int32_t y_max = y_0_is_smaller ? y1 : y0;

	//Reverse the points to draw from left to right
	if (x1 < x0) {
		drawLine(x1, y1, x0, y0, colour);
		return;
	}

	// Vertical line
	if (x1 == x0) {
		// Force the line to be within the screen
		if (y_min < 0) y_min = 0;
		if (y_max >= m_height)
			y_max = m_height - 1;

		// Mirror the Y axis as directly calling put_pixel will not do this
		if (mirror_y_axis) {
			int32_t temp = y_max;
			y_max = m_height - y_min - 1;
			y_min = m_height - temp - 1;
		}

		// Check that the line is within the screen
		if (0 > x0 || (uint32_t) x0 >= m_width) {
			return;
		}

		// Draw the line
		for (int32_t y = y_min; y <= y_max; ++y)
			putPixel(x0, y, colour);

		return;
	}

	// Horizontal line
	if (y1 == y0) {
		// Ensure the line is within the screen
		if (x0 < 0) x0 = 0;
		if (x1 >= m_width) x1 = m_width - 1;

		// Mirror the Y axis as directly calling put_pixel will not do this
		if (mirror_y_axis)
			y0 = m_height - y0 - 1;

		// Check that the line is within the screen
		if (0 > y0 || y0 >= m_height)
			return;

		// Draw the line
		for (int32_t x = x0; x <= x1; ++x)
			putPixel(x, y0, colour);
	}

	// If the line is not horizontal or vertical then it must be a diagonal line
	// Find the slope of the line
	float slope = ((float) (y1 - y0)) / (x1 - x0);

	// A slope that is more horizontal should be drawn by incrementing x
	if (-1 <= slope && slope <= 1) {
		float y = y0;
		for (int32_t x = x0; x <= x1; x++, y += slope)
			putPixel(x, (int32_t) y, colour);
	}

		// A slope that is more vertical should be drawn by incrementing y
	else {
		// Invert the slope
		slope = 1.0f / slope;

		float x = x0;
		for (int32_t y = y_min; y <= y_max; x += slope, y++)
			putPixel((int32_t) x, y, colour);
	}
}

/**
 * @brief Draws a rectangle on the screen
 *
 * @param x0 The x coordinate of the top left corner
 * @param y0 The y coordinate of the top left corner
 * @param x1 The x coordinate of the bottom right corner
 * @param y1 The y coordinate of the bottom right corner
 * @param colour The colour of the rectangle
 */
void GraphicsContext::draw_rectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Colour &colour) {
	draw_rectangle(x0, y0, x1, y1, colour_to_int(colour));

}

/**
 * @brief Draws a rectangle on the screen
 *
 * @param x0 The x coordinate of the top left corner
 * @param y0 The y coordinate of the top left corner
 * @param x1 The x coordinate of the bottom right corner
 * @param y1 The y coordinate of the bottom right corner
 * @param colour The colour of the rectangle
 */
void GraphicsContext::draw_rectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t colour) {

	// Ensure x and y 0 is smaller than x and y 1
	--y0;
	--x0;

	// Draw the rectangle
	drawLine(x0, y0, x1, y0, colour); // Top
	drawLine(x0, y1, x1, y1, colour); // Bottom
	drawLine(x0, y0, x0, y1, colour); // Left
	drawLine(x1, y0, x1, y1, colour); // Right

}

/**
 * @brief Draws a rectangle on the screen, filled with a colour
 *
 * @param x0 The x coordinate of the top left corner
 * @param y0 The y coordinate of the top left corner
 * @param x1 The x coordinate of the bottom right corner
 * @param y1 The y coordinate of the bottom right corner
 * @param colour The colour of the rectangle
 */
void GraphicsContext::fill_rectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, const Colour &colour) {
	fill_rectangle(x0, y0, x1, y1, colour_to_int(colour));
}

/**
 * @brief Draws a rectangle on the screen, filled with a colour
 *
 * @param x0 The x coordinate of the top left corner
 * @param y0 The y coordinate of the top left corner
 * @param x1 The x coordinate of the bottom right corner
 * @param y1 The y coordinate of the bottom right corner
 * @param colour The colour of the rectangle
 */
void GraphicsContext::fill_rectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t colour) {

	// Draw from left to right
	if (y1 < y0) {
		fill_rectangle(x1, y1, x0, y0, colour);
		return;
	}

	// Make sure the rectangle is within the height of the screen
	if (y0 < 0) y0 = 0;
	if (y1 > m_height) y1 = m_height;

	// Make sure the rectangle is within the width of the screen
	bool x_0_is_smaller = x0 < x1;
	int32_t x_min = x_0_is_smaller ? x0 : x1;
	int32_t x_max = x_0_is_smaller ? x1 : x0;

	if (x_min < 0) x_min = 0;
	if (x_max > m_width)
		x_max = m_width;

	// Mirror the Y axis as directly calling put_pixel will not do this
	if (mirror_y_axis) {
		int32_t temp = y1;
		y1 = m_height - y0 - 1;
		y0 = m_height - temp - 1;
	}

	// Draw the rectangle
	for (int32_t y = y0; y < y1; ++y) {
		for (int32_t x = x_min; x < x_max; ++x) {
			putPixel(x, y, colour);
		}
	}

}

/**
 * @brief Draws a circle on the screen
 *
 * @param x0 The x coordinate of the centre of the circle
 * @param y0 The y coordinate of the centre of the circle
 * @param radius The radius of the circle
 * @param colour The colour of the circle
 */
void GraphicsContext::draw_circle(int32_t x0, int32_t y0, int32_t radius, const Colour &colour) {
	draw_circle(x0, y0, radius, colour_to_int(colour));
}

/**
 * @brief Draws a circle on the screen
 *
 * @param x0 The x coordinate of the centre of the circle
 * @param y0 The y coordinate of the centre of the circle
 * @param radius The radius of the circle
 * @param colour The colour of the circle
 */
void GraphicsContext::draw_circle(int32_t x0, int32_t y0, int32_t radius, uint32_t colour) {

	// Make sure the circle is with in the width and height of the screen
	if (x0 < 0) x0 = 0;
	if (x0 > m_width) x0 = m_width;
	if (y0 < 0) y0 = 0;
	if (y0 > m_height) y0 = m_height;

	// Mirror the Y axis as directly calling put_pixel will not do this
	if (mirror_y_axis)
		y0 = m_height - y0 - 1;


	// Begin drawing at the left most point of the circle and draw a line to the right most point of the circle
	for (int32_t x = -radius; x <= radius; ++x) {

		// Draw a line from the top most point of the circle to the bottom most point of the circle
		for (int32_t y = -radius; y <= radius; ++y) {

			// If the point is within the circle, draw it but make sure it is only part of the outline
			if (x * x + y * y <= radius * radius && x * x + y * y >= (radius - 1) * (radius - 1))
				putPixel(x0 + x, y0 + y, colour);
		}
	}


}

/**
 * @brief Draws a circle on the screen, filled with a colour
 *
 * @param x0 The x coordinate of the centre of the circle
 * @param y0 The y coordinate of the centre of the circle
 * @param radius The radius of the circle
 * @param colour The colour of the circle
 */
void GraphicsContext::fill_circle(int32_t x0, int32_t y0, int32_t radius, const Colour &colour) {
	fillCircle(x0, y0, radius, colour_to_int(colour));

}

/**
 * @brief Draws a circle on the screen, filled with a colour
 *
 * @param x0 The x coordinate of the centre of the circle
 * @param y0 The y coordinate of the centre of the circle
 * @param radius The radius of the circle
 * @param colour The colour of the circle
 */
void GraphicsContext::fillCircle(int32_t x0, int32_t y0, int32_t radius, uint32_t colour) {

	// Make sure the circle is with in the width and height of the screen
	if (x0 < 0) x0 = 0;
	if (x0 > m_width) x0 = m_width;
	if (y0 < 0) y0 = 0;
	if (y0 > m_height) y0 = m_height;

	// Mirror the Y axis as directly calling put_pixel will not do this
	if (mirror_y_axis)
		y0 = m_height - y0 - 1;

	// Draw the circle

	// Begin drawing at the left most point of the circle and draw a line to the right most point of the circle
	for (int32_t x = -radius; x <= radius; ++x) {

		// Draw a line from the top most point of the circle to the bottom most point of the circle
		for (int32_t y = -radius; y <= radius; ++y) {

			// Only draw the pixel if it is within the circle
			if (x * x + y * y <= radius * radius)
				putPixel(x0 + x, y0 + y, colour);
		}
	}
}

/**
 * @brief Gets the address of the context's framebuffer to draw on
 *
 * @return The framebuffer address
 */
uint64_t *GraphicsContext::framebuffer_address() {
	return m_framebuffer_address;
}
