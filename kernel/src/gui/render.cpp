//
// Created by 98max on 10/16/2022.
//
#include <gui/render.h>

using namespace maxOS;
using namespace gui;
using namespace common;


Render::Render(common::int32_t w, common::int32_t h) {

}

Render::~Render() {

}

/**
 * @details This method is used to draw a pixel on the screen.
 *
 * @param gc The graphics context to draw the pixels.
 */
void Render::display(GraphicsContext* gc){
    for(int ix = 0; ix < 320; ix++){
        for(int iy = 0; iy < 200; iy++){
            gc->PutPixel(ix,iy, pixels[ix][iy].r, pixels[ix][iy].g, pixels[ix][iy].b);
        }
    }
}

/**
 * @details This method is used to draw a pixel on the into a pixels buffer.
 *
 * @param x The x coordinate of the pixel.
 * @param y The y coordinate of the pixel.
 * @param r The red value of the pixel.
 * @param g The green value of the pixel.
 * @param b The blue value of the pixel.
 */
void Render::PutPixel(int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b){
    if(x < 0 || x >= 320 || y < 0 || y >= 200) return; //Check if coords r legal

    pixels[x][y].r = r;
    pixels[x][y].g = g;
    pixels[x][y].b = b;
}
