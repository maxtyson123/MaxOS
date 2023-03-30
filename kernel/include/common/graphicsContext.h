//
// Created by 98max on 10/15/2022.
//

#ifndef MaxOS_COMMON_GRAPHICSCONTEX_H
#define MaxOS_COMMON_GRAPHICSCONTEX_H

#include <drivers/vga.h>
#include <common/colour.h>

namespace maxOS {

    namespace common {

       class GraphicsContext {

        protected:
           bool mirrorYAxis;

           uint32_t width;
           uint32_t height;
           uint32_t colorDepth;

           Colour colourPallet[256];

           virtual void putPixel(uint32_t x, uint32_t y, uint32_t colour);
           virtual void putPixel8Bit(uint32_t x, uint32_t y, uint32_t colour);
           virtual void putPixel16Bit(uint32_t x, uint32_t y, uint32_t colour);
           virtual void putPixel24Bit(uint32_t x, uint32_t y, uint32_t colour);
           virtual void putPixel32Bit(uint32_t x, uint32_t y, uint32_t colour);

           virtual uint32_t getPixel(uint32_t x, uint32_t y);
           virtual uint32_t getPixel8Bit(uint32_t x, uint32_t y);
           virtual uint32_t getPixel16Bit(uint32_t x, uint32_t y);
           virtual uint32_t getPixel24Bit(uint32_t x, uint32_t y);
           virtual uint32_t getPixel32Bit(uint32_t x, uint32_t y);

        public:
           GraphicsContext();
           ~GraphicsContext();

           uint32_t colourToInt(Colour colour);
           Colour intToColour(uint32_t colour);

           uint32_t getWidth();
           uint32_t getHeight();

           void putPixel(int32_t x, int32_t y, Colour colour);
           void putPixel(int32_t x, int32_t y, int32_t colour);
           Colour getPixel(int32_t x, int32_t y);
           void invertPixel(int32_t x, int32_t y);

           void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, Colour colour);
           void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t colour);

           void drawRectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, Colour colour);
           void drawRectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t colour);

           void fillRectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, Colour colour);
           void fillRectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t colour);

       };

    }
}

#endif //MaxOS_GRAPHICSCONTEX_H
