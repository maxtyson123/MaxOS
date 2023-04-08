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
    colourPallet[0]  = Colour(0x00, 0x00, 0x00);    // Black
    colourPallet[1]  = Colour(0x00, 0x00, 0xA8);    // Duke Blue
    colourPallet[2]  = Colour(0x00, 0xA8, 0x00);    // Islamic Green
    colourPallet[3]  = Colour(0x00, 0xA8, 0xA8);    // Persian Green
    colourPallet[4]  = Colour(0xA8, 0x00, 0x00);    // Dark Candy Apple Red
    colourPallet[5]  = Colour(0xA8, 0x00, 0xA8);    // Heliotrope Magenta
    colourPallet[6]  = Colour(0xA8, 0xA8, 0x00);    // Light Gold
    colourPallet[7]  = Colour(0xA8, 0xA8, 0xA8);    // Dark Gray (X11)
    colourPallet[8]  = Colour(0x00, 0x00, 0x57);    // Cetacean Blue
    colourPallet[9]  = Colour(0x00, 0x00, 0x0FF);   // Blue
    colourPallet[10] = Colour(0x00, 0xA8, 0x57);    // Green (Pigment)
    colourPallet[11] = Colour(0x00, 0xA8, 0xFF);    // Vivid Cerulean
    colourPallet[12] = Colour(0xA8, 0x00, 0x57);    // Jazz berry Jam
    colourPallet[13] = Colour(0xA8, 0x00, 0x57);    // Jazz berry Jam
    colourPallet[14] = Colour(0xA8, 0xA8, 0x57);    // Olive Green
    colourPallet[15] = Colour(0xA8, 0xA8, 0xFF);    // Maximum Blue Purple
    colourPallet[16] = Colour(0x00,0x57,0x00);      // Dark Green (X11)
    colourPallet[17] = Colour(0x00,0x57,0xA8);      // Cobalt Blue
    colourPallet[18] = Colour(0x00,0xFF,0x00);      // Electric Green
    colourPallet[19] = Colour(0x00,0xFF,0xA8);      // Medium Spring Green
    colourPallet[20] = Colour(0xA8,0x57,0x00);      // Windsor Tan
    colourPallet[21] = Colour(0xA8,0x57,0xA8);      // Purpureus
    colourPallet[22] = Colour(0xA8,0xFF,0x00);      // Spring Bud
    colourPallet[23] = Colour(0xA8,0xFF,0xA8);      // Mint Green
    colourPallet[24] = Colour(0x00,0x57,0x57);      // Midnight Green (Eagle Green)
    colourPallet[25] = Colour(0x00,0x57,0xFF);      // Blue (RYB)
    colourPallet[26] = Colour(0x00,0xFF,0x57);      // Malachite
    colourPallet[27] = Colour(0x00,0xFF,0xFF);      // Aqua
    colourPallet[28] = Colour(0xA8,0x57,0x57);      // Middle Red Purple
    colourPallet[29] = Colour(0xA8,0x57,0xFF);      // Lavender Indigo
    colourPallet[30] = Colour(0xA8,0xFF,0x57);      // Inchworm
    colourPallet[31] = Colour(0xA8,0xFF,0xFF);      // Celeste


    // Set the rest of the palette to white
    for(uint8_t colorCode = 255; colorCode >= 0x40; --colorCode)
        colourPallet[colorCode] = Colour(0,0,0);


}

GraphicsContext::~GraphicsContext() {

}

/**
 * Renders a pixel to the screen, automatically uses the correct color depth
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
 * Renders a pixel to the screen using the 8 bit color depth
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The 8Bit colour of the pixel
 */
void GraphicsContext::renderPixel8Bit(uint32_t x, uint32_t y, uint8_t colour) {

}

/**
 * Renders a pixel to the screen using the 16 bit color depth
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The 16Bit colour of the pixel
 */
void GraphicsContext::renderPixel16Bit(uint32_t x, uint32_t y, uint16_t colour) {

}

/**
 * Renders a pixel to the screen using the 24 bit color depth
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The 24Bit colour of the pixel
 */
void GraphicsContext::renderPixel24Bit(uint32_t x, uint32_t y, uint32_t colour) {

}

/**
 * Renders a pixel to the screen using the 32 bit color depth
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The 32Bit colour of the pixel
 */
void GraphicsContext::renderPixel32Bit(uint32_t x, uint32_t y, uint32_t colour) {

}

/**
 * Gets the colour of a pixel on the screen, automatically uses the correct color depth
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
 * Gets the colour of a pixel on the screen using the 8 bit color depth
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @return The 8Bit colour of the pixel
 */
uint8_t GraphicsContext::getRenderedPixel8Bit(uint32_t x, uint32_t y) {
    return 0;
}

/**
 * Gets the colour of a pixel on the screen using the 16 bit color depth
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @return The 16Bit colour of the pixel
 */
uint16_t GraphicsContext::getRenderedPixel16Bit(uint32_t x, uint32_t y) {
    return 0;
}

/**
 * Gets the colour of a pixel on the screen using the 24 bit color depth
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @return The 24Bit colour of the pixel
 */
uint32_t GraphicsContext::getRenderedPixel24Bit(uint32_t x, uint32_t y) {
    return 0;
}

/**
 * Gets the colour of a pixel on the screen using the 32 bit color depth
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @return The 32Bit colour of the pixel
 */
uint32_t GraphicsContext::getRenderedPixel32Bit(uint32_t x, uint32_t y) {
    return 0;
}

/**
 * Converts a colour to an integer value based on the current color depth
 * @param colour The colour class to convert
 * @return The integer value of the colour
 */
uint32_t GraphicsContext::colourToInt(Colour colour) {

    switch (colorDepth) {
        case 8:
        {
            uint32_t result = 0;
            int bestDistance = 0xFFFFFFF;      // Set to max int

            // Loop through the colour pallet and find the closest match
            for (uint8_t i = 0; i < 255; ++i) {
                Colour* palletColour = &colourPallet[i];

                // Calculate the distance between the two colours
                int distance = ((int)colour.red - (int)palletColour->red) * ((int)colour.red - (int)palletColour->red) +
                                ((int)colour.green - (int)palletColour->green) * ((int)colour.green - (int)palletColour->green) +
                                ((int)colour.blue - (int)palletColour->blue) * ((int)colour.blue - (int)palletColour->blue);

                // If the distance is less than the best distance, set the best distance to the current distance
                if (distance < bestDistance) {
                    bestDistance = distance;
                    result = i;
                }

                return  result;
            }

        }

        case 16:
        {
           // 16-Bit Colour: 5 bits for red, 6 bits for green, 5 bits for blue (RRRRR,GGGGGG,BBBBB)
           uint32_t result = 0;

           result = (
                     (uint16_t)(colour.red & 0xF8) << 8)                    // Red,   mask off the top 3 bits and shift left 8 (RRRRR00000000000)
                   | ((uint16_t)(colour.green & 0xFC) << 3)                 // Green, mask off the top 2 bits and shift left 3 bits (000000GGGGGG000)
                   | ((uint16_t)(colour.blue & 0xF8) >> 3);                 // Blue,  mask off the top 3 bits and shift right 3 bits (000000000000BBBBB)

            return result;

        }

        case 24:
        {
            // 24-Bit Colour: 8 bits for red, 8 bits for green, 8 bits for blue (RRRRRRRR,GGGGGGGG,BBBBBBBB)
            uint32_t result = 0;

            result = (
                      (uint32_t)colour.red << 16)                              // Red,   shift left 16 bits (RRRRRRRR0000000000000000)
                    | ((uint32_t)colour.green << 8)                            // Green, shift left 8 bits (00000000GGGGGGGG00000000)
                    | ((uint32_t)colour.blue);                                 // Blue,  no shift (0000000000000000BBBBBBBB)

            return result;
        }

        default:
        case 32:
        {
            // 32-Bit Colour: 8 bits for red, 8 bits for green, 8 bits for blue, 8 bits for alpha (RRRRRRRR,GGGGGGGG,BBBBBBBB,AAAAAAAA)

            return (uint32_t)colour.red << 24
                 | (uint32_t)colour.green << 16
                 | (uint32_t)colour.blue << 8
                 | (uint32_t)colour.alpha;
        }
    }
}

/**
 * Converts an integer value to a colour based on the current color depth
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
 * Gets the width of the screen
 * @return The width of the screen
 */
uint32_t GraphicsContext::getWidth() {
    return width;
}

/**
 * Gets the height of the screen
 * @return The height of the screen
 */
uint32_t GraphicsContext::getHeight() {
    return height;
}

/**
 * Renders a pixel to the screen
 * @param x The x coordinate of the pixel
 * @param y The y coordinate of the pixel
 * @param colour The colour of the pixel
 */
void GraphicsContext::putPixel(int32_t x, int32_t y, Colour colour) {

    // Convert the colour to an integer and then print it
    renderPixel(x,y, colourToInt(colour));
}

/**
 * Renders a pixel to the screen
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
 * Gets the colour of a pixel
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
 * Inverts a pixel
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
 * Draws a line on the screen
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
 * Draws a line on the screen
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
            int32_t temp = yMax;            // Store the maximum y value
            yMax = height - yMin - 1;       // Set the maximum y value to the minimum y value
            yMin = height - temp - 1;       // Set the minimum y value to the maximum y value
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
 * Draws a rectangle on the screen
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
 * Draws a rectangle on the screen
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
 * Draws a rectangle on the screen, filled with a colour
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
 * Draws a rectangle on the screen, filled with a colour
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
    for(int32_t y = y0; y <= y1; ++y){
        for (int32_t x = xMin; x <= xMax; ++x) {
            renderPixel(x, y, colour);
        }
    }

}

/**
 * Draws a circle on the screen
 * @param x0 The x coordinate of the centre of the circle
 * @param y0 The y coordinate of the centre of the circle
 * @param radius The radius of the circle
 * @param colour The colour of the circle
 */
void GraphicsContext::drawCircle(int32_t x0, int32_t y0, int32_t radius, Colour colour){
    drawCircle(x0,y0,radius,colourToInt(colour));
}

/**
 * Draws a circle on the screen
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

            // Only draw the pixel if it is within the circle
            if(x*x + y*y <= radius*radius)
                renderPixel(x0+x,y0+y,colour);
        }
    }


}

/**
 * Draws a circle on the screen, filled with a colour
 * @param x0 The x coordinate of the centre of the circle
 * @param y0 The y coordinate of the centre of the circle
 * @param radius The radius of the circle
 * @param colour The colour of the circle
 */
void GraphicsContext::fillCircle(int32_t x0, int32_t y0, int32_t radius, Colour colour) {
    fillCircle(x0,y0,radius,colourToInt(colour));

}

/**
 * Draws a circle on the screen, filled with a colour
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