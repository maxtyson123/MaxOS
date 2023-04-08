//
// Created by 98max on 10/15/2022.
//

#ifndef MaxOS_COMMON_GRAPHICSCONTEX_H
#define MaxOS_COMMON_GRAPHICSCONTEX_H

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

           virtual void renderPixel(uint32_t x, uint32_t y, uint32_t colour);
           virtual void renderPixel8Bit(uint32_t x, uint32_t y, uint8_t colour);
           virtual void renderPixel16Bit(uint32_t x, uint32_t y, uint16_t colour);
           virtual void renderPixel24Bit(uint32_t x, uint32_t y, uint32_t colour);
           virtual void renderPixel32Bit(uint32_t x, uint32_t y, uint32_t colour);

           virtual uint32_t getRenderedPixel(uint32_t x, uint32_t y);
           virtual uint8_t getRenderedPixel8Bit(uint32_t x, uint32_t y);
           virtual uint16_t getRenderedPixel16Bit(uint32_t x, uint32_t y);
           virtual uint32_t getRenderedPixel24Bit(uint32_t x, uint32_t y);
           virtual uint32_t getRenderedPixel32Bit(uint32_t x, uint32_t y);

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

           void drawCircle(int32_t x0, int32_t y0, int32_t radius, Colour colour);
           void drawCircle(int32_t x0, int32_t y0, int32_t radius, uint32_t colour);

           void fillCircle(int32_t x0, int32_t y0, int32_t radius, Colour colour);
           void fillCircle(int32_t x0, int32_t y0, int32_t radius, uint32_t colour);

       };

    }
}

#endif //MaxOS_GRAPHICSCONTEX_H
