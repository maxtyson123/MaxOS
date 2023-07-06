//
// Created by 98max on 30/03/2023.
//

#include <common/graphicsContext.h>

using namespace maxOS::common;

GraphicsContext::GraphicsContext() {
    width = 0;
    height = 0;
    colorDepth = 0;
    mirrorYAxis = false;

    // VirtualBox VGA palette
    colourPallet[0x00] = Colour(0x00,0x00,0x00);    // Black
    colourPallet[0x01] = Colour(0x00,0x00,0xA8);    // Duke Blue
    colourPallet[0x02] = Colour(0x00,0xA8,0x00);    // Islamic Green
    colourPallet[0x03] = Colour(0x00,0xA8,0xA8);    // Persian Green
    colourPallet[0x04] = Colour(0xA8,0x00,0x00);    // Dark Candy Apple Red
    colourPallet[0x05] = Colour(0xA8,0x00,0xA8);    // Heliotrope Magenta

    colourPallet[0x06] = Colour(0xA8,0xA8,0x00);    // Light Gold
    colourPallet[0x07] = Colour(0xA8,0xA8,0xA8);    // Dark Gray (X11)
    colourPallet[0x08] = Colour(0x00,0x00,0x57);    // Cetacean Blue
    colourPallet[0x09] = Colour(0x00,0x00,0xFF);    // Blue
    colourPallet[0x0A] = Colour(0x00,0xA8,0x57);    // Green (Pigment)
    colourPallet[0x0B] = Colour(0x00,0xA8,0xFF);    // Vivid Cerulean
    colourPallet[0x0C] = Colour(0xA8,0x00,0x57);    // Jazzberry Jam
    colourPallet[0x0D] = Colour(0xA8,0x00,0x57);    // Jazzberry Jam
    colourPallet[0x0E] = Colour(0xA8,0xA8,0x57);    // Olive Green
    colourPallet[0x0F] = Colour(0xA8,0xA8,0xFF);    // Maximum Blue Purple

    colourPallet[0x10] = Colour(0x00,0x57,0x00);    // Dark Green (X11)
    colourPallet[0x11] = Colour(0x00,0x57,0xA8);    // Cobalt Blue
    colourPallet[0x12] = Colour(0x00,0xFF,0x00);    // Electric Green
    colourPallet[0x13] = Colour(0x00,0xFF,0xA8);    // Medium Spring Green
    colourPallet[0x14] = Colour(0xA8,0x57,0x00);    // Windsor Tan
    colourPallet[0x15] = Colour(0xA8,0x57,0xA8);    // Purpureus
    colourPallet[0x16] = Colour(0xA8,0xFF,0x00);    // Spring Bud
    colourPallet[0x17] = Colour(0xA8,0xFF,0xA8);    // Mint Green
    colourPallet[0x18] = Colour(0x00,0x57,0x57);    // Midnight Green (Eagle Green)
    colourPallet[0x19] = Colour(0x00,0x57,0xFF);    // Blue (RYB)
    colourPallet[0x1A] = Colour(0x00,0xFF,0x57);    // Malachite
    colourPallet[0x1B] = Colour(0x00,0xFF,0xFF);    // Aqua
    colourPallet[0x1C] = Colour(0xA8,0x57,0x57);    // Middle Red Purple
    colourPallet[0x1D] = Colour(0xA8,0x57,0xFF);    // Lavender Indigo
    colourPallet[0x1E] = Colour(0xA8,0xFF,0x57);    // Olive Green
    colourPallet[0x1F] = Colour(0xA8,0xFF,0xFF);    // Celeste

    colourPallet[0x20] = Colour(0x57,0x00,0x00);    // Blood Red
    colourPallet[0x21] = Colour(0x57,0x00,0xA8);    // Metallic Violet
    colourPallet[0x22] = Colour(0x57,0xA8,0x00);    // Kelly Green
    colourPallet[0x23] = Colour(0x57,0xA8,0xA8);    // Cadet Blue
    colourPallet[0x24] = Colour(0xFF,0x00,0x00);    // Red
    colourPallet[0x25] = Colour(0xFF,0x00,0xA8);    // Fashion Fuchsia
    colourPallet[0x26] = Colour(0xFF,0xA8,0x00);    // Chrome Yellow
    colourPallet[0x27] = Colour(0xFF,0xA8,0xA8);    // Light Salmon Pink
    colourPallet[0x28] = Colour(0x57,0x00,0x57);    // Imperial Purple
    colourPallet[0x29] = Colour(0x57,0x00,0xFF);    // Electric Indigo
    colourPallet[0x2A] = Colour(0x57,0xA8,0x57);    // Apple
    colourPallet[0x2B] = Colour(0x57,0xA8,0xFF);    // Blue Jeans
    colourPallet[0x2C] = Colour(0xFF,0x00,0x57);    // Folly
    colourPallet[0x2D] = Colour(0xFF,0x00,0xFF);    // Fuchsia
    colourPallet[0x2E] = Colour(0xFF,0xA8,0x57);    // Rajah
    colourPallet[0x2F] = Colour(0xFF,0xA8,0xFF);    // Rich Brilliant Lavender

    colourPallet[0x30] = Colour(0x57,0x57,0x00);    // Dark Bronze (Coin)
    colourPallet[0x31] = Colour(0x57,0x57,0xA8);    // Liberty
    colourPallet[0x32] = Colour(0x57,0xFF,0x00);    // Chlorophyll Green
    colourPallet[0x33] = Colour(0x57,0xFF,0xA8);    // Medium Aquamarine
    colourPallet[0x34] = Colour(0xFF,0x57,0x00);    // Orange (Pantone)
    colourPallet[0x35] = Colour(0xFF,0x57,0xA8);    // Brilliant Rose
    colourPallet[0x36] = Colour(0xFF,0xFF,0x00);    // Yellow
    colourPallet[0x37] = Colour(0xFF,0xFF,0xA8);    // Calamansi
    colourPallet[0x38] = Colour(0x57,0x57,0x57);    // Davy's Grey
    colourPallet[0x39] = Colour(0x57,0x57,0xFF);    // Very Light Blue
    colourPallet[0x3A] = Colour(0x57,0xFF,0x57);    // Screamin' Green
    colourPallet[0x3B] = Colour(0x57,0xFF,0xFF);    // Electric Blue
    colourPallet[0x3C] = Colour(0xFF,0x57,0x57);    // Sunset Orange
    colourPallet[0x3D] = Colour(0xFF,0x57,0xFF);    // Shocking Pink (Crayola)
    colourPallet[0x3E] = Colour(0xFF,0xFF,0x57);    // Shocking Pink (Crayola)
    colourPallet[0x3F] = Colour(0xFF,0xFF,0xFF);    // White


    // Set the rest of the palette to black
    for(uint8_t colorCode = 255; colorCode >= 0x40; --colorCode)
        colourPallet[colorCode] = Colour(0,0,0);


}

GraphicsContext::~GraphicsContext() {

}

/**
 * @details Renders a pixel to the screen, automatically uses the correct color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The colour of the pixel
 */
void GraphicsContext::renderPixel(uint32_t x, uint32_t y, uint32_t colour) {

    // Call the correct putPixel function based on the color depth
    switch (colorDepth) {
        case 8:
            renderPixel8Bit(x, y, colour);
            break;
        case 16:
            renderPixel16Bit(x, y, colour);
            break;
        case 24:
            renderPixel24Bit(x, y, colour);
            break;
        case 32:
            renderPixel32Bit(x, y, colour);
            break;
    }


}

/**
 * @details Renders a pixel to the screen using the 8 bit color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The 8Bit colour of the pixel
 */
void GraphicsContext::renderPixel8Bit(uint32_t x, uint32_t y, uint8_t colour) {

}

/**
 * @details Renders a pixel to the screen using the 16 bit color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The 16Bit colour of the pixel
 */
void GraphicsContext::renderPixel16Bit(uint32_t x, uint32_t y, uint16_t colour) {

}

/**
 * @details Renders a pixel to the screen using the 24 bit color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The 24Bit colour of the pixel
 */
void GraphicsContext::renderPixel24Bit(uint32_t x, uint32_t y, uint32_t colour) {

}

/**
 * @details Renders a pixel to the screen using the 32 bit color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The 32Bit colour of the pixel
 */
void GraphicsContext::renderPixel32Bit(uint32_t x, uint32_t y, uint32_t colour) {

}

/**
 * @details Gets the colour of a pixel on the screen, automatically uses the correct color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @return The colour of the pixel
 */
uint32_t GraphicsContext::getRenderedPixel(uint32_t x, uint32_t y) {
    // Call the correct getPixel function based on the color depth
    switch (colorDepth) {
        case 8:
            return getRenderedPixel8Bit(x, y);
        case 16:
            return getRenderedPixel16Bit(x, y);
        case 24:
            return getRenderedPixel24Bit(x, y);
        case 32:
            return getRenderedPixel32Bit(x, y);
    }
}

/**
 * @details Gets the colour of a pixel on the screen using the 8 bit color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @return The 8Bit colour of the pixel
 */
uint8_t GraphicsContext::getRenderedPixel8Bit(uint32_t x, uint32_t y) {
    return 0;
}

/**
 * @details Gets the colour of a pixel on the screen using the 16 bit color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @return The 16Bit colour of the pixel
 */
uint16_t GraphicsContext::getRenderedPixel16Bit(uint32_t x, uint32_t y) {
    return 0;
}

/**
 * @details Gets the colour of a pixel on the screen using the 24 bit color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @return The 24Bit colour of the pixel
 */
uint32_t GraphicsContext::getRenderedPixel24Bit(uint32_t x, uint32_t y) {
    return 0;
}

/**
 * @details Gets the colour of a pixel on the screen using the 32 bit color depth
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @return The 32Bit colour of the pixel
 */
uint32_t GraphicsContext::getRenderedPixel32Bit(uint32_t x, uint32_t y) {
    return 0;
}

/**
 * @details Converts a colour to an integer value based on the current color depth
 *
 * @param colour The colour class to convert
 * @return The integer value of the colour
 */
uint32_t GraphicsContext::colourToInt(Colour colour) {

    switch(colorDepth)
    {
        case 8:
        {
            uint32_t result = 0;
            int mindistance = 0xfffffff;
            for(uint32_t i = 0; i <= 255; ++i)
            {
                Colour* c = &colourPallet[i];
                int distance =
                        ((int)colour.red-(int)c->red)*((int)colour.red-(int)c->red)
                        +((int)colour.green-(int)c->green)*((int)colour.green-(int)c->green)
                        +((int)colour.blue-(int)c->blue)*((int)colour.blue-(int)c->blue);
                if(distance < mindistance)
                {
                    mindistance = distance;
                    result = i;
                }
            }
            return result;
        }
        case 16:
        {
            // 16-Bit colours RRRRRGGGGGGBBBBB
            return ((uint16_t)(colour.red   & 0xF8)) << 8
                   | ((uint16_t)(colour.green & 0xFC)) << 3
                   | ((uint16_t)(colour.blue  & 0xF8) >> 3);
        }
        case 24:
        {
            return (uint32_t)colour.red   << 16
                   | (uint32_t)colour.green << 8
                   | (uint32_t)colour.blue;
        }
        default:
        case 32:
        {
            return (uint32_t)colour.red   << 24
                   | (uint32_t)colour.green << 16
                   | (uint32_t)colour.blue  <<  8
                   | (uint32_t)colour.alpha;
        }
    }
}

/**
 * @details Converts an integer value to a colour based on the current color depth
 *
 * @param colour The integer value to convert
 * @return The colour class of the integer value
 */
Colour GraphicsContext::intToColour(uint32_t colour) {
    switch (colorDepth) {

        case 8:
        {
            return colourPallet[colour & 0xFF]; //Mask off the top 24 bits and return the colour from the pallet (e.g. 0x00FF0000 -> 0x000000FF -> colourPallet[0xFF])
        }

        case 16:
        {
            // 16-Bit Colour: 5 bits for red, 6 bits for green, 5 bits for blue (RRRRR,GGGGGG,BBBBB)
            Colour result;

            result.red = (colour & 0xF800) >> 8;                                 // Red,   mask off the top 5 bits and shift right 8 bits (0000000000000000RRRRR000)
            result.green = (colour & 0x07E0) >> 3;                               // Green, mask off the top 6 bits and shift right 3 bits (000000000000GGGGGG000000)
            result.blue = (colour & 0x001F) << 3;                                // Blue,  mask off the top 5 bits and shift left 3 bits (000000000000000000000BBBBB)

            return result;
        }

        case 24:
        {
            // 24-Bit Colour: 8 bits for red, 8 bits for green, 8 bits for blue (RRRRRRRR,GGGGGGGG,BBBBBBBB)
            Colour result;

            result.red = (colour & 0xFF0000) >> 16;                               // Red,   mask off the top 8 bits and shift right 16 bits (RRRRRRRR0000000000000000)
            result.green = (colour & 0x00FF00) >> 8;                              // Green, mask off the top 8 bits and shift right 8 bits (00000000GGGGGGGG00000000)
            result.blue = (colour & 0x0000FF);                                    // Blue,  mask off the top 8 bits (0000000000000000BBBBBBBB)

            return result;
        }

        default:
        case 32:
        {
            // 32-Bit Colour: 8 bits for red, 8 bits for green, 8 bits for blue, 8 bits for alpha (RRRRRRRR,GGGGGGGG,BBBBBBBB,AAAAAAAA)
            Colour result;

            result.red = (colour & 0xFF000000) >> 24;                             // Red,   mask off the top 8 bits and shift right 24 bits (RRRRRRRR000000000000000000000000)
            result.green = (colour & 0x00FF0000) >> 16;                        // Green, mask off the top 8 bits and shift right 16 bits (00000000GGGGGGGG0000000000000000)
            result.blue = (colour & 0x0000FF00) >> 8;                          // Blue,  mask off the top 8 bits and shift right 8 bits (0000000000000000BBBBBBBB00000000)
            result.alpha = (colour & 0x000000FF);                               // Alpha, mask off the top 8 bits (000000000000000000000000AAAAAAAA)

            return result;

        }
    }
}

/**
 * @details Gets the width of the screen
 *
 * @return The width of the screen
 */
uint32_t GraphicsContext::getWidth() {
    return width;
}

/**
 * @details Gets the height of the screen
 *
 * @return The height of the screen
 */
uint32_t GraphicsContext::getHeight() {
    return height;
}

/**
 * @details Renders a pixel to the screen
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The colour of the pixel
 */
void GraphicsContext::putPixel(int32_t x, int32_t y, Colour colour) {

    // Convert the colour to an integer and then print it
    renderPixel(x,y, colourToInt(colour));
}

/**
 * @details Renders a pixel to the screen
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The colour of the pixel
 */
void GraphicsContext::putPixel(int32_t x, int32_t y, int32_t colour) {

    // Check if the pixel is within the width of the screen
    if (0 > x || (uint32_t)x >= width) {
        return;
    }

    // Check if the pixel is within the height of the screen
    if (0 > y || (uint32_t) y >= height) {
        return;
    }

    // Render the pixel
    renderPixel(x, mirrorYAxis ? height-y-1 : y, colour);

}

/**
 * @details Gets the colour of a pixel
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @return The colour of the pixel
 */
Colour GraphicsContext::getPixel(int32_t x, int32_t y) {

    // Check if the pixel is within the width of the screen
    if (0 > x || (uint32_t)x >= width) {
        return Colour(0,0,0);
    }

    // Check if the pixel is within the height of the screen
    if (0 > y || (uint32_t) y >= height) {
        return Colour(0,0,0);
    }

    // Get the pixel and convert it to a colour
    uint32_t translatedColor = getRenderedPixel(x, mirrorYAxis ? height-y-1 : y);
    return intToColour(translatedColor);


}

/**
 * @details Inverts a pixel
 *
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 */
void GraphicsContext::invertPixel(int32_t x, int32_t y) {
    // Get the pixel
    Colour colour = getPixel(x,y);

    // Invert the pixel
    colour.red = 255 - colour.red;
    colour.green = 255 - colour.green;
    colour.blue = 255 - colour.blue;

    // Render the pixel
    putPixel(x,y,colour);
}


/**
 * @details Draws a line on the screen
 *
 * @param x0 The x coordinate of the first point
 * @param y0 The y coordinate of the first point
 * @param x1 The x coordinate of the final point
 * @param y1 The y coordinate of the final point
 * @param colour The colour of the line
 */
void GraphicsContext::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, Colour colour) {
    drawLine(x0,y0,x1,y1,colourToInt(colour));
}

/**
 * @details Draws a line on the screen
 *
 * @param x0 The x coordinate of the first point
 * @param y0 The y coordinate of the first point
 * @param x1 The x coordinate of the final point
 * @param y1 The y coordinate of the final point
 * @param colour The colour of the line
 */
void GraphicsContext::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t colour) {

    // Store the minimum and maximum y values by checking which one is larger/smaller
    bool y0IsSmaller = y0 < y1;
    int32_t yMin = y0IsSmaller ? y0 : y1;
    int32_t yMax = y0IsSmaller ? y1 : y0;

    // If x1 is smaller than x0, reverse the points to draw from left to right
    if(x1 < x0){
        drawLine(x1,y1,x0,y0,colour);
        return;
    }

    // If the line is vertical, draw it
    if(x1 == x0)
    {
        // Force the line to be within the screen
        if(yMin < 0) yMin = 0;
        if((uint32_t)yMax >= height) yMax = height - 1;

        // Mirror the Y axis as directly calling renderPixel will not do this
        if(mirrorYAxis)
        {
            int32_t temp = yMax;
            yMax = height - yMin - 1;
            yMin = height - temp - 1;
        }

        // Check that the line is within the screen
        if (0 > x0 || (uint32_t) x0 >= width) {
            return;
        }

        // Draw the line
        for(int32_t y = yMin; y <= yMax; ++y)
            renderPixel(x0, y, colour);

        return;
    }

    // If the line is horizontal, draw it
    if(y1 == y0)
    {
        // Ensure the line is within the screen
        if(x0 < 0) x0 = 0;
        if((uint32_t)x1 >= width) x1 = width-1;

        // Mirror the Y axis as directly calling renderPixel will not do this
        if(mirrorYAxis)
            y0 = height-y0-1;

        // Check that the line is within the screen
        if (0 > y0 || (uint32_t) y0 >= height)
            return;

        // Draw the line
        for(int32_t x = x0; x <= x1; ++x)
            renderPixel(x,y0,colour);
    }

    // If the line is not horizontal or vertical then it must be a diagonal line
    // Find the slope of the line
    float slope = ((float)(y1-y0))/(x1-x0);              // Change in y over change in x

    // If the line is more horizontal than vertical, increment x by 1 and increment y by the slope
    if(-1 <= slope && slope <= 1)
    {
        // Start at minimum y and increment y by the slope
        float y = y0;

        // Start at minimum x and increment x by 1
        for(int32_t x = x0; x <= x1; x++, y+=slope)
            renderPixel(x, (int32_t)y, colour);
    }

    // If the line is more vertical than horizontal, increment y by 1 and increment x by the inverse of the slope
    else
    {
        // Invert the slope
        slope = 1.0f/slope;

        // Start at minimum x and increment x by the inverse of the slope
        float x = x0;

        // Start at minimum y and increment y by 1
        for(int32_t y = yMin; y <= yMax; x+=slope, y++)
            renderPixel((int32_t)x, y, colour);
    }



}

/**
 * @details Draws a rectangle on the screen
 *
 * @param x0 The x coordinate of the top left corner
 * @param y0 The y coordinate of the top left corner
 * @param x1 The x coordinate of the bottom right corner
 * @param y1 The y coordinate of the bottom right corner
 * @param colour The colour of the rectangle
 */
void GraphicsContext::drawRectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, Colour colour) {
    drawRectangle(x0,y0,x1,y1,colourToInt(colour));

}

/**
 * @details Draws a rectangle on the screen
 *
 * @param x0 The x coordinate of the top left corner
 * @param y0 The y coordinate of the top left corner
 * @param x1 The x coordinate of the bottom right corner
 * @param y1 The y coordinate of the bottom right corner
 * @param colour The colour of the rectangle
 */
void GraphicsContext::drawRectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t colour) {

    // Ensure x and y 0 is smaller than x and y 1
    --y0;
    --x0;

    // Draw the rectangle
    drawLine(x0,y0,x1,y0,colour); // Top
    drawLine(x0,y1,x1,y1,colour); // Bottom
    drawLine(x0,y0,x0,y1,colour); // Left
    drawLine(x1,y0,x1,y1,colour); // Right

}

/**
 * @details Draws a rectangle on the screen, filled with a colour
 *
 * @param x0 The x coordinate of the top left corner
 * @param y0 The y coordinate of the top left corner
 * @param x1 The x coordinate of the bottom right corner
 * @param y1 The y coordinate of the bottom right corner
 * @param colour The colour of the rectangle
 */
void GraphicsContext::fillRectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, Colour colour) {
    fillRectangle(x0,y0,x1,y1,colourToInt(colour));
}

/**
 * @details Draws a rectangle on the screen, filled with a colour
 *
 * @param x0 The x coordinate of the top left corner
 * @param y0 The y coordinate of the top left corner
 * @param x1 The x coordinate of the bottom right corner
 * @param y1 The y coordinate of the bottom right corner
 * @param colour The colour of the rectangle
 */
void GraphicsContext::fillRectangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t colour) {

    // Draw from left to right
    if(y1 < y0){
        fillRectangle(x1,y1,x0,y0,colour);
        return;
    }

    // Make sure the rectangle is within the height of the screen
    if(y0 < 0) y0 = 0;
    if((uint32_t)y1 > height) y1 = height;

    // Make sure the rectangle is within the width of the screen
    bool x0IsSmaller = x0 < x1;
    int32_t xMin = x0IsSmaller ? x0 : x1;
    int32_t xMax = x0IsSmaller ? x1 : x0;

    if(xMin < 0) xMin = 0;
    if((uint32_t)xMax > width) xMax = width;

    // Mirror the Y axis as directly calling renderPixel will not do this
    if(mirrorYAxis)
    {
        uint32_t temp = y1;             // Store the maximum y value
        y1 = height - y0 - 1;           // Set the maximum y value to the minimum y value
        y0 = height - temp - 1;         // Set the minimum y value to the maximum y value
    }

    // Draw the rectangle
    for(int32_t y = y0; y < y1; ++y){
        for (int32_t x = xMin; x < xMax; ++x) {
            renderPixel(x, y, colour);
        }
    }

}

/**
 * @details Draws a circle on the screen
 *
 * @param x0 The x coordinate of the centre of the circle
 * @param y0 The y coordinate of the centre of the circle
 * @param radius The radius of the circle
 * @param colour The colour of the circle
 */
void GraphicsContext::drawCircle(int32_t x0, int32_t y0, int32_t radius, Colour colour){
    drawCircle(x0,y0,radius,colourToInt(colour));
}

/**
 * @details Draws a circle on the screen
 *
 * @param x0 The x coordinate of the centre of the circle
 * @param y0 The y coordinate of the centre of the circle
 * @param radius The radius of the circle
 * @param colour The colour of the circle
 */
void GraphicsContext::drawCircle(int32_t x0, int32_t y0, int32_t radius, uint32_t colour) {

    // Make sure the circle is with in the width and height of the screen
    if(x0 < 0) x0 = 0;
    if((uint32_t)x0 > width) x0 = width;
    if(y0 < 0) y0 = 0;
    if((uint32_t)y0 > height) y0 = height;

    // Mirror the Y axis as directly calling renderPixel will not do this
    if(mirrorYAxis)
        y0 = height-y0-1;

    // Draw the circle

    // Begin drawing at the left most point of the circle and draw a line to the right most point of the circle
    for(int32_t x = -radius; x <= radius; ++x){

        // Draw a line from the top most point of the circle to the bottom most point of the circle
        for(int32_t y = -radius; y <= radius; ++y){

            // If the point is within the circle, draw it but make sure it is only part of the outline
            if(x*x + y*y <= radius*radius && x*x + y*y >= (radius-1)*(radius-1))
                renderPixel(x0+x,y0+y,colour);
        }
    }


}

/**
 * @details Draws a circle on the screen, filled with a colour
 *
 * @param x0 The x coordinate of the centre of the circle
 * @param y0 The y coordinate of the centre of the circle
 * @param radius The radius of the circle
 * @param colour The colour of the circle
 */
void GraphicsContext::fillCircle(int32_t x0, int32_t y0, int32_t radius, Colour colour) {
    fillCircle(x0,y0,radius,colourToInt(colour));

}

/**
 * @details Draws a circle on the screen, filled with a colour
 *
 * @param x0 The x coordinate of the centre of the circle
 * @param y0 The y coordinate of the centre of the circle
 * @param radius The radius of the circle
 * @param colour The colour of the circle
 */
void GraphicsContext::fillCircle(int32_t x0, int32_t y0, int32_t radius, uint32_t colour) {

        // Make sure the circle is with in the width and height of the screen
        if(x0 < 0) x0 = 0;
        if((uint32_t)x0 > width) x0 = width;
        if(y0 < 0) y0 = 0;
        if((uint32_t)y0 > height) y0 = height;

        // Mirror the Y axis as directly calling renderPixel will not do this
        if(mirrorYAxis)
            y0 = height-y0-1;

        // Draw the circle

        // Begin drawing at the left most point of the circle and draw a line to the right most point of the circle
        for(int32_t x = -radius; x <= radius; ++x){

            // Draw a line from the top most point of the circle to the bottom most point of the circle
            for(int32_t y = -radius; y <= radius; ++y){

                // Only draw the pixel if it is within the circle
                if(x*x + y*y <= radius*radius)
                    renderPixel(x0+x,y0+y,colour);
            }
        }

}