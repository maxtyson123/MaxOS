//
// Created by 98max on 10/16/2022.
//
#include "gui/render.h"

using namespace maxOS;
using namespace gui;
using namespace common;


Render::Render(common::int32_t w, common::int32_t h) {

}

Render::~Render() {

}

void Render::display(common::GraphicsContext *gc) {
    for(int X = 0; X < 320; X++){
        for(int T = 0; T < 200; T++){
            gc->PutPixel(X,T, pixels[X][T].r, pixels[X][T].g, pixels[X][T].b);
        }
    }
}

void Render::PutPixel(common::int32_t x, common::int32_t y, common::uint8_t r, common::uint8_t g, common::uint8_t b) {
    VideoGraphicsArray::PutPixel(x, y, r, g, b);
}
