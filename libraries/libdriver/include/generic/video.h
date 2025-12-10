/**
 * @file video.h
 * @brief Defines a generic Video class for handling video modes and graphics context operations.
 *
 * @date 30th March 2023
 * @author Max Tyson
 */

#ifndef LIBDRIVER_GENERIC_VIDEO_H
#define LIBDRIVER_GENERIC_VIDEO_H

#include <stdint.h>
#include <driver.h>

namespace LibDriver::generic {

	/**
	 * @class Video
	 * @brief Driver for the video controller, handles the setting of the video mode
	 */
	class Video : public Driver {

		protected:
			virtual bool internal_set_mode(uint32_t width, uint32_t height, uint32_t color_depth);

			uint32_t m_width;
			uint32_t m_height;
			uint32_t m_color_depth;

		public:
			Video();
			~Video();

			virtual bool supports_mode(uint32_t width, uint32_t height, uint32_t color_depth);
			bool set_mode(uint32_t width, uint32_t height, uint32_t color_depth);
	};
}

#endif //LIBDRIVER_GENERIC_VIDEO_H
