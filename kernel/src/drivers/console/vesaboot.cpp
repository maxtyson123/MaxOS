/**
 * @file vesaboot.cpp
 * @brief Implementation of a VESA Boot Console for text output in graphics mode using VESA BIOS Extensions (VBE)
 *
 * @date 2nd October 2023
 * @author Max Tyson
 */

#include <drivers/console/vesaboot.h>
#include <gui/font/amiga_font.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::gui;
using namespace MaxOS::memory;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::console;
using namespace MaxOS::system;

/**
 * @brief Constructs a new VESA Boot Console object, initializing the console area for text output.
 * @param graphics_context The graphics context to use for rendering
 */
VESABootConsole::VESABootConsole(GraphicsContext *graphics_context)
: m_font((uint8_t *)AMIGA_FONT)
{

	// Set up
	Logger::INFO() << "Setting up VESA console\n";
	s_graphics_context = graphics_context;
	m_video_memory_meta = (uint16_t *) MemoryManager::kmalloc(width() * height() * sizeof(uint16_t));

	// Prepare the console
	VESABootConsole::clear();
	print_logo();
	m_console_area = new ConsoleArea(this, 0, 0, width() / 2 - 25, height(), ConsoleColour::DarkGrey, ConsoleColour::Black);
	cout = new ConsoleStream(m_console_area);

	// Only log to the screen when debugging
	#ifdef TARGET_DEBUG
		Logger::active_logger()->add_log_writer(cout);
		Logger::INFO() << "Console Stream set up \n";
	#endif
}

VESABootConsole::~VESABootConsole() = default;

/**
 * @brief Gets the width of the console
 *
 * @return The width of the console in characters
 */
uint16_t VESABootConsole::width() {
	return s_graphics_context->width() / FONT_WIDTH;       // 8 pixels per character
}

/**
 * @brief Gets the height of the console
 *
 * @return The height of the console in characters
 */
uint16_t VESABootConsole::height() {
	return s_graphics_context->height() / FONT_HEIGHT;
}

/**
 * @brief Places a character at the specified location
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @param c The character to place
 */
void VESABootConsole::put_character(uint16_t x, uint16_t y, char c) {

	// Parse any ansi codes
	if (c == '\033') {

		// Store the character
		ansi_code_length = 0;
		ansi_code[ansi_code_length++] = c;

		// Do not draw the escape character
		return;
	}

	if (ansi_code_length < 8) {

		// Add the character to the ANSI code
		ansi_code[ansi_code_length++] = c;

		// If the ANSI code is complete
		if (c == 'm') {
			ansi_code[ansi_code_length] = '\0';
			ansi_code_length = -1;

			if (strcmp("\033[0m", ansi_code) != 0) {
				m_foreground_color = ConsoleColour::Uninitialised;
				m_background_color = ConsoleColour::Uninitialised;
				return;
			}

			// Get the colour from the ANSI code
			const Colour colour(ansi_code);

			// Set the colour
			bool foreground = ansi_code[4] == '3';
			if (foreground)
				m_foreground_color = colour.to_console_colour();
			else
				m_background_color = colour.to_console_colour();

		}

		// Do not draw the escape character
		return;
	}

	// If the coordinates are out of bounds, return
	if (x >= width() || y >= height())
		return;

	// Calculate the offset
	int offset = (y * width() + x);

	// Set the character at the offset, by masking the character with the current character (last 8 bits)
	m_video_memory_meta[offset] = (m_video_memory_meta[offset] & 0xFF00) | (uint16_t) c;

	// Convert the char into a string
	char s[] = " ";
	s[0] = c;

	Colour foreground = m_foreground_color == ConsoleColour::Uninitialised ? get_foreground_color(x, y) : Colour(m_foreground_color);
	Colour background = m_background_color == ConsoleColour::Uninitialised ? get_background_color(x, y) : Colour(m_background_color);

	// Use the m_font to draw the character
	m_font.draw_text(x * 8, y * FONT_HEIGHT, foreground, background, s_graphics_context, s);
}

/**
 * @brief Sets the foreground color at the specified location
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @param foreground The foreground color
 */
void VESABootConsole::set_foreground_color(uint16_t x, uint16_t y, ConsoleColour foreground) {

	// If the coordinates are out of bounds, return
	if (x >= width() || y >= height())
		return;

	// Calculate the offset
	int offset = (y * width() + x);

	// Set the foreground color at the offset, by masking the foreground color with the current foreground color (bits 8-11)
	m_video_memory_meta[offset] = (m_video_memory_meta[offset] & 0xF0FF) | ((uint16_t) foreground << 8);
}

/**
 * @brief Sets the background color at the specified location
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @param background The background color
 */
void VESABootConsole::set_background_color(uint16_t x, uint16_t y, ConsoleColour background) {

	// If the coordinates are out of bounds, return
	if (x >= width() || y >= height())
		return;

	// Calculate the offset
	int offset = (y * width() + x);

	// Set the background color at the offset, by masking the background color with the current background color (bits 12-15)
	m_video_memory_meta[offset] = (m_video_memory_meta[offset] & 0x0FFF) | ((uint16_t) background << 12);
}

/**
 * @brief Gets the character at the specified location
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @return The character at the specified location or a space if the coordinates are out of bounds
 */
char VESABootConsole::get_character(uint16_t x, uint16_t y) {

	// If the coordinates are out of bounds, return
	if (x >= width() || y >= height())
		return ' ';

	// Calculate the offset
	int offset = (y * width() + x);

	// Return the character at the offset, by masking the character with the current character (last 8 bits)
	return (char) (m_video_memory_meta[offset] & 0x00FF);
}

/**
 * @brief  Gets the foreground color at the specified location
 *
 * @param x The x coordinate
 * @param y The y coordinate
 * @return The foreground color at the specified location or white if the coordinates are out of bounds
 */
ConsoleColour VESABootConsole::get_foreground_color(uint16_t x, uint16_t y) {

	if(CPU::panic_lock.is_locked())
		return ConsoleColour::White;

	// If the coordinates are out of bounds, return
	if (x >= width() || y >= height())
		return ConsoleColour::White;

	// Calculate the offset
	int offset = (y * width() + x);

	// Return the foreground color at the offset, by masking the foreground color with the current foreground color (bits 8-11)
	return (ConsoleColour) ((m_video_memory_meta[offset] & 0x0F00) >> 8);
}

/**
 * @brief Gets the background color at the specified location
 * @param x The x coordinate
 * @param y The y coordinate
 *
 * @return The background color at the specified location or black if the coordinates are out of bounds
 */
ConsoleColour VESABootConsole::get_background_color(uint16_t x, uint16_t y) {

	if(CPU::panic_lock.is_locked())
		return ConsoleColour::Red;

	// If the coordinates are out of bounds, return
	if (x >= width() || y >= height())
		return ConsoleColour::Black;

	// Calculate the offset
	int offset = (y * width() + x);

	// Return the background color at the offset, by masking the background color with the current background color (bits 12-15)
	return (ConsoleColour) ((m_video_memory_meta[offset] & 0xF000) >> 12);
}

/**
 * @brief Prints the logo to the center of the screen
 *
 * @param is_panic Whether to print the kernel panic logo or the normal logo
 */
void VESABootConsole::print_logo(bool is_panic) {

	// Load the logo
	const char *logo = is_panic ? header_data_kp : header_data;

	// Find the center of the screen
	uint32_t screen_width = s_graphics_context->width();
	uint32_t screen_height = s_graphics_context->height();
	uint32_t center_x = screen_width / 2;
	uint32_t center_y = screen_height / 2 - 80;

	// Fill the screen with the logo colour
	auto col = Colour(is_panic ? ConsoleColour::Red : ConsoleColour::Black);
	memset(s_graphics_context->framebuffer_address(), s_graphics_context->colour_to_int(col), screen_width * screen_height * (s_graphics_context->color_depth()/8));

	// Draw the logo
	for (uint32_t logoY = 0; logoY < LOGO_HEIGHT; ++logoY) {
		for (uint32_t logoX = 0; logoX < LOGO_WIDTH; ++logoX) {

			// Get the pixel from the logo
			uint8_t pixel[3] = {0};
			LOGO_HEADER_PIXEL(logo, pixel)

			// Draw the pixel
			s_graphics_context->put_pixel(center_x - LOGO_WIDTH / 2 + logoX,
										  center_y - LOGO_HEIGHT / 2 + logoY,
										  common::Colour(pixel[0], pixel[1], pixel[2]));
		}
	}

	update_progress_bar(0);
}


/**
 * @brief Scrolls the console up by 1 line
 *
 * @param left The left coordinate of the area to scroll
 * @param top The top coordinate of the area to scroll
 * @param width The width of the area to scroll
 * @param height The height of the area to scroll
 * @param foreground The foreground color of the new line
 * @param background The background color of the new line
 * @param fill The character to fill the new line with
 */
void VESABootConsole::scroll_up(uint16_t left, uint16_t top, uint16_t width,
								uint16_t height,
								common::ConsoleColour foreground,
								common::ConsoleColour background, char fill) {


	// Get the framebuffer info
	auto *framebuffer_address = (uint8_t *) s_graphics_context->framebuffer_address();
	uint64_t framebuffer_width = s_graphics_context->width();
	uint64_t framebuffer_bpp = s_graphics_context->color_depth(); // in bits per pixel
	uint64_t bytes_per_pixel = framebuffer_bpp / 8;
	uint64_t framebuffer_pitch = framebuffer_width * bytes_per_pixel;

	uint16_t line_height = FONT_HEIGHT;

	// Region conversions
	uint16_t region_pixel_y = top * line_height;
	uint16_t region_pixel_height = height * line_height;
	uint16_t region_pixel_left = left * FONT_WIDTH;
	uint16_t region_pixel_width = width * FONT_WIDTH;
	size_t row_bytes = region_pixel_width * bytes_per_pixel;

	// Decide the colour of the pixel
	ConsoleColour to_set_foreground = get_foreground_color(left, top + height - 1);
	ConsoleColour to_set_background = get_background_color(left, top + height - 1);
	Colour fill_colour = Colour(to_set_background);
	uint32_t fill_value = s_graphics_context->colour_to_int(to_set_background);

	// Scroll the region upward by one text line
	for (uint16_t row = 0; row < region_pixel_height - line_height; row++) {
		uint8_t *src  = framebuffer_address + (region_pixel_y + row + line_height) * framebuffer_pitch + region_pixel_left * bytes_per_pixel;
		uint8_t *dest = framebuffer_address + (region_pixel_y + row) * framebuffer_pitch + region_pixel_left * bytes_per_pixel;
		memmove(dest, src, row_bytes);
	}

	// Clear the last line of the region
	uint16_t clear_start_y = region_pixel_y + region_pixel_height - line_height;
	for (uint16_t row = 0; row < line_height; row++) {
		auto row_add = (uint32_t *) (framebuffer_address + (clear_start_y + row) * framebuffer_pitch + region_pixel_left * 4);
		for (uint16_t col = 0; col < region_pixel_width; col++) {
			row_add[col] = fill_value;
		}
	}

	//Update any per-pixel colour metadata
	uint16_t text_row = top + height - 1;
	for (uint16_t x = left; x < left + width; x++) {
		set_foreground_color(x, text_row, to_set_foreground);
		set_background_color(x, text_row, to_set_background);
	}
}

/**
 * @brief Cleans up the boot console
 */
void VESABootConsole::finish() {

	// Done
	Logger::HEADER() << "MaxOS Kernel Successfully Booted\n" << ANSI_COLOURS[ANSIColour::Reset];
	cout->set_cursor(0, 0);
	Logger::active_logger()->disable_log_writer(cout);
}

/**
 * @brief Updates the progress bar
 *
 * @param percentage The percentage to update the progress bar to (0-100)
 */
void VESABootConsole::update_progress_bar(uint8_t percentage) {

	// Check bounds
	if (percentage > 100)
		percentage = 100;

	// Must have a valid graphics context
	if (s_graphics_context == nullptr)
		return;

	uint8_t progress_height = 15;
	uint8_t progress_spacing = 20;
	uint8_t progress_width_cull = 40;

	// Find the center of the screen
	uint32_t right_x = (s_graphics_context->width() / 2) - LOGO_WIDTH / 2;
	uint32_t bottom_y = (s_graphics_context->height() / 2 - 80) - LOGO_HEIGHT / 2;

	// Find the bounds
	uint32_t start_x = progress_width_cull;
	uint32_t start_y = LOGO_HEIGHT + progress_spacing;
	uint32_t end_x = LOGO_WIDTH - progress_width_cull;
	uint32_t end_y = LOGO_HEIGHT + progress_height + progress_spacing;

	// Draw the progress bar
	for (uint32_t progress_y = start_y; progress_y < end_y; ++progress_y) {
		for (uint32_t progress_x = start_x; progress_x < end_x; ++progress_x) {

			// Check if drawing border
			bool is_border = (progress_y == start_y) || (progress_y == end_y - 1) ||
							 (progress_x == start_x) || (progress_x == end_x - 1);

			// Only draw the border if it is the first time drawing it
			is_border = is_border && percentage == 0;

			// If it is not within the percentage, skip it
			if (progress_x > LOGO_WIDTH * percentage / 100 && !is_border)
				continue;

			s_graphics_context->put_pixel(right_x + progress_x, bottom_y + progress_y, Colour(0xFF, 0xFF, 0xFF));

		}
	}
}