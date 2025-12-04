/**
 * @file vesa.h
 * @brief Defines a Video Electronics Standards Association (VESA) video driver for handling pixel rendering using VESA standards and a framebuffer
 *
 * @date 19th July 2023
 * @author Max Tyson
 */

#ifndef MAXOS_VIDEO_VESA_H
#define MAXOS_VIDEO_VESA_H

#include <cstdint>
#include <string.h>
#include <drivers/video/video.h>
#include <memory/memorymanagement.h>
#include <system/multiboot.h>
#include <memory/memoryIO.h>


namespace MaxOS::drivers::video {

	/**
	 * @class VideoElectronicsStandardsAssociation
	 * @brief Driver for the VESA video controller, handles the rendering of pixels to the screen using VESA
	 */
	class VideoElectronicsStandardsAssociation : public VideoDriver {

		private:
			bool internal_set_mode(uint32_t width, uint32_t height, uint32_t) final;

			void render_pixel_32_bit(uint32_t x, uint32_t y, uint32_t colour) final;
			uint32_t get_rendered_pixel_32_bit(uint32_t x, uint32_t y) final;

			// Memory
			size_t m_framebuffer_size;

			// Info
			multiboot_tag_framebuffer* m_framebuffer_info;
			uint8_t m_bpp;
			uint16_t m_pitch;

		public:
			explicit VideoElectronicsStandardsAssociation(multiboot_tag_framebuffer* framebuffer_info);
			~VideoElectronicsStandardsAssociation();

			bool supports_mode(uint32_t width, uint32_t height, uint32_t) final;

			string vendor_name() final;
			string device_name() final;

	};

}


#endif //MAXOS_VIDEO_VESA_H
