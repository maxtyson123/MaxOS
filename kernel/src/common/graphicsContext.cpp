//
// Created by 98max on 30/03/2023.
//

# include <common/graphicsContext.h>

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


void GraphicsContext::putPixel(uint32_t x, uint32_t y, uint32_t colour) {

    // Call the correct putPixel function based on the color depth
    switch (colorDepth) {
        case 8:
            putPixel8Bit(x, y, colour);
            break;
        case 16:
            putPixel16Bit(x, y, colour);
            break;
        case 24:
            putPixel24Bit(x, y, colour);
            break;
        case 32:
            putPixel32Bit(x, y, colour);
            break;
    }


}

void GraphicsContext::putPixel8Bit(uint32_t x, uint32_t y, uint32_t colour) {

}

void GraphicsContext::putPixel16Bit(uint32_t x, uint32_t y, uint32_t colour) {

}

void GraphicsContext::putPixel24Bit(uint32_t x, uint32_t y, uint32_t colour) {

}

void GraphicsContext::putPixel32Bit(uint32_t x, uint32_t y, uint32_t colour) {

}

uint32_t GraphicsContext::getPixel(uint32_t x, uint32_t y) {
    // Call the correct getPixel function based on the color depth
    switch (colorDepth) {
        case 8:
            return getPixel8Bit(x, y);
        case 16:
            return getPixel16Bit(x, y);
        case 24:
            return getPixel24Bit(x, y);
        case 32:
            return getPixel32Bit(x, y);
    }
}

uint32_t GraphicsContext::getPixel8Bit(uint32_t x, uint32_t y) {
    return 0;
}

uint32_t GraphicsContext::getPixel16Bit(uint32_t x, uint32_t y) {
    return 0;
}

uint32_t GraphicsContext::getPixel24Bit(uint32_t x, uint32_t y) {
    return 0;
}

uint32_t GraphicsContext::getPixel32Bit(uint32_t x, uint32_t y) {
    return 0;
}

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

uint32_t GraphicsContext::getWidth() {
    return width;
}

uint32_t GraphicsContext::getHeight() {
    return height;
}

void GraphicsContext::putPixel(int32_t x, int32_t y, Colour colour) {

}

