/**
 * @file vga.cpp
 * @brief Implementation of a Video Graphics Array (VGA) driver
 *
 * @date 14th October 2022
 * @author Max Tyson
 *
 * @see https://files.osdev.org/mirrors/geezer/osd/graphics/modes.c - helpful for VGA mode setting
 */

#include <drivers/video/vga.h>

using namespace MaxOS;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::video;
using namespace MaxOS::hardwarecommunication;

VideoGraphicsArray::VideoGraphicsArray()
: m_misc_port(0x3C2),
  m_crtc_index_port(0x3D4),
  crtc_data_port(0x3D5),
  m_sequence_index_port(0x3C4),
  m_sequence_data_port(0x3C5),
  m_graphics_controller_index_port(0x3CE),
  m_graphics_controller_data_port(0x3CF),
  m_attribute_controller_index_port(0x3C0),
  m_attribute_controller_read_port(0x3C1),
  m_attribute_controller_write_port(0x3C0),
  m_attribute_controller_reset_port(0x3DA)
{
}

VideoGraphicsArray::~VideoGraphicsArray() = default;

/**
 * @brief This function is used to write to the VGA registers.
 *
 * @param registers  The VGA registers to write to.
 */
void VideoGraphicsArray::write_registers(uint8_t *registers) {

	// Move to the next register
	m_misc_port.write(*(registers++));

	// Set the sequencer registers
	for (uint8_t i = 0; i < 5; i++) {
		m_sequence_index_port.write(i);
		m_sequence_data_port.write(*(registers++));
	}

	// Clear protection bit to enable writing to CR0-7
	m_crtc_index_port.write(0x03);
	crtc_data_port.write(crtc_data_port.read() | 0x80);
	m_crtc_index_port.write(0x11);
	crtc_data_port.write(crtc_data_port.read() | ~0x80);

	// Ensure protection bit is set
	registers[0x03] = registers[0x03] | 0x80;
	registers[0x11] = registers[0x11] & ~0x80;

	// Write the CRTC registers
	for (uint8_t i = 0; i < 25; i++) {
		m_crtc_index_port.write(i);
		crtc_data_port.write(*(registers++));
	}

	// Write the graphics controller registers
	for (uint8_t i = 0; i < 9; i++) {
		m_graphics_controller_index_port.write(i);
		m_graphics_controller_data_port.write(*(registers++));
	}

	// Write the attribute controller registers
	for (uint8_t i = 0; i < 21; i++) {
		m_attribute_controller_reset_port.read();
		m_attribute_controller_index_port.write(i);
		m_attribute_controller_write_port.write(*(registers++));
	}

	// Re-Lock CRTC and unblank display
	m_attribute_controller_reset_port.read();
	m_attribute_controller_index_port.write(0x20);

}

/**
 * @brief Checks if the specified resolution is supported.
 *
 * @return True if the specified resolution is supported, otherwise false.
 */
bool VideoGraphicsArray::supports_mode(uint32_t width, uint32_t height, uint32_t colour_depth) {
	return width == 320 && height == 200 && colour_depth == 8;
}

/**
 * @brief Set the resolution of the screen.
 *
 * @param width The width of the resolution.
 * @param height The height of the resolution.
 * @param colour_depth The colour depth of the resolution.
 *
 * @return True if the card was able to set the resolution, otherwise false.
 */
bool VideoGraphicsArray::internal_set_mode(uint32_t width, uint32_t height, uint32_t colour_depth) {

	//Values from osdev / modes.c
	unsigned char g_320x200x256[] =
			{
					// MISC
					0x63,
					// SEQ
					0x03, 0x01, 0x0F, 0x00, 0x0E,
					// CRTC
					0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
					0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
					0xFF,
					// GC
					0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
					0xFF,
					// AC
					0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
					0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
					0x41, 0x00, 0x0F, 0x00, 0x00
			};

	write_registers(g_320x200x256);
	return true;
}

/**
 * @brief This function is used to get the framebuffer address.
 *
 * @return The framebuffer address.
 */
uint8_t *VideoGraphicsArray::get_frame_buffer_segment() {

	// Optimise so that don't have to read and write to the port every time
	return (uint8_t *) 0xA0000;

	//read data from index number 6
	m_graphics_controller_index_port.write(0x06);
	uint8_t segmentNumber =
			m_graphics_controller_data_port.read() &
			(3 << 2); //Shift by 2 as only interested in bits 3 & 4 (& 3 so all the other bits are removed)
	switch (segmentNumber) {
		default:
		case 0 << 2:
			return (uint8_t *) nullptr;
		case 1 << 2:
			return (uint8_t *) 0xA0000;
		case 2 << 2:
			return (uint8_t *) 0xB0000;
		case 3 << 2:
			return (uint8_t *) 0xB8000;
	}
}

/**
 * @brief Puts a 8 bit pixel on the screen.
 *
 * @param x The x coordinate of the pixel.
 * @param y The y coordinate of the pixel.
 * @param colour The colour of the pixel.
 */
void VideoGraphicsArray::render_pixel_8_bit(uint32_t x, uint32_t y, uint8_t colour) {

	uint8_t *pixel_address = get_frame_buffer_segment() + 320 * y + x;
	*pixel_address = colour;
}

/**
 * @brief Gets a 8 bit pixel from the screen.
 *
 * @param x The x coordinate of the pixel.
 * @param y The y coordinate of the pixel.
 * @return The colour of the pixel.
 */
uint8_t VideoGraphicsArray::get_rendered_pixel_8_bit(uint32_t x, uint32_t y) {

	uint8_t *pixel_address = get_frame_buffer_segment() + 320 * y + x;
	return *pixel_address;
}

/**
 * @brief Gets the name of the vendor.
 *
 * @return The name of the vendor.
 */
string VideoGraphicsArray::vendor_name() {

	// VGA was made by IBM
	return "IBM";
}

/**
 * @brief Gets the name of the device.
 *
 * @return The name of the device.
 */
string VideoGraphicsArray::device_name() {
	return "VGA compatible graphics card";
}