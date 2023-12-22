//
// Created by 98max on 10/15/2022.
//

#ifndef MaxOS_COMMON_GRAPHICSCONTEX_H
#define MaxOS_COMMON_GRAPHICSCONTEX_H

#include <stdint.h>
#include <common/colour.h>

namespace maxOS {

    namespace common {

       /**
        * @class GraphicsContext
        * @brief Draws pixels to the screen, and handles drawing lines, rectangles and circles
        */
       class GraphicsContext {

        protected:
           bool mirror_y_axis { false };

           uint32_t m_width { 0 };
           uint32_t m_height { 0 };
           uint32_t m_color_depth { 0 };

           Colour m_colour_pallet[256];

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

           uint32_t colour_to_int(Colour);
           Colour int_to_colour(uint32_t);

           uint32_t get_width();
           uint32_t get_height();

           void put_pixel(int32_t x, int32_t y, Colour colour);
           void putPixel(int32_t x, int32_t y, int32_t colour);
           Colour get_pixel(int32_t x, int32_t y);
           void invert_pixel(int32_t x, int32_t y);

           void draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, Colour colour);
           void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t colour);

           void draw_rectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, Colour colour);
           void draw_rectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t colour);

           void fill_rectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, Colour colour);
           void fill_rectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t colour);

           void draw_circle(int32_t x0, int32_t y0, int32_t radius, Colour colour);
           void draw_circle(int32_t x0, int32_t y0, int32_t radius, uint32_t colour);

           void fill_circle(int32_t x0, int32_t y0, int32_t radius, Colour colour);
           void fillCircle(int32_t x0, int32_t y0, int32_t radius, uint32_t colour);

       };

    }
}

#endif //MaxOS_GRAPHICSCONTEX_H
