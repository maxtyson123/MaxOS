/**
 * @file graphicsContext.h
 * @brief Defines a GraphicsContext class for drawing pixels, lines, rectangles and circles to the screen
 *
 * @date 15th October 2022
 * @author Max Tyson
 *
 * @todo Move to drivers/video?
 */

#ifndef MaxOS_COMMON_GRAPHICSCONTEX_H
#define MaxOS_COMMON_GRAPHICSCONTEX_H

#include <cstdint>
#include <colour.h>


namespace MaxOS::common {

	/**
	 * @class GraphicsContext
	 * @brief Draws pixels to the screen, and handles drawing lines, rectangles and circles
	 */
	class GraphicsContext {

		protected:
			bool mirror_y_axis {
					false };    ///< Should the y axis be mirrored (0,0 is top left if false, bottom left if true)

			uint32_t m_width { 0 };           ///<  The width of the screen in pixels
			uint32_t m_height { 0 };          ///<  The height of the screen in pixels
			uint32_t m_color_depth { 0 };     ///<  The color depth of the screen in bits per pixel

			Colour m_colour_pallet[256];     ///<  The colour pallet for 8 bit color depth @todo make const

			uint64_t* m_framebuffer_address { nullptr }; ///< The address of the framebuffer

			virtual void render_pixel(uint32_t x, uint32_t y, uint32_t colour);
			virtual void render_pixel_8_bit(uint32_t x, uint32_t y, uint8_t colour);
			virtual void render_pixel_16_bit(uint32_t x, uint32_t y, uint16_t colour);
			virtual void render_pixel_24_bit(uint32_t x, uint32_t y, uint32_t colour);
			virtual void render_pixel_32_bit(uint32_t x, uint32_t y, uint32_t colour);

			virtual uint32_t get_rendered_pixel(uint32_t x, uint32_t y);
			virtual uint8_t get_rendered_pixel_8_bit(uint32_t x, uint32_t y);
			virtual uint16_t get_rendered_pixel_16_bit(uint32_t x, uint32_t y);
			virtual uint32_t get_rendered_pixel_24_bit(uint32_t x, uint32_t y);
			virtual uint32_t get_rendered_pixel_32_bit(uint32_t x, uint32_t y);

		public:
			GraphicsContext();
			~GraphicsContext();

			uint32_t colour_to_int(const Colour&);
			Colour int_to_colour(uint32_t);

			// Convert uint32_t to uint64s?
			[[nodiscard]] uint32_t width() const;
			[[nodiscard]] uint32_t height() const;
			[[nodiscard]] uint32_t color_depth() const;

			uint64_t* framebuffer_address();

			void put_pixel(uint32_t x, uint32_t y, const Colour& colour);
			void put_pixel(uint32_t x, uint32_t y, uint32_t colour);
			Colour get_pixel(uint32_t x, uint32_t y);
			void invert_pixel(uint32_t x, uint32_t y);

			void draw_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, const Colour& colour);
			void draw_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t colour);

			void draw_rectangle(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, const Colour& colour);
			void draw_rectangle(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t colour);

			void fill_rectangle(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, const Colour& colour);
			void fill_rectangle(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t colour);

			void draw_circle(uint32_t x0, uint32_t y0, uint32_t radius, const Colour& colour);
			void draw_circle(uint32_t x0, uint32_t y0, uint32_t radius, uint32_t colour);

			void fill_circle(uint32_t x0, uint32_t y0, uint32_t radius, const Colour& colour);
			void fill_circle(uint32_t x0, uint32_t y0, uint32_t radius, uint32_t colour);

	};

}


#endif //MaxOS_COMMON_GRAPHICSCONTEX_H
