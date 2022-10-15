//
// Created by Tyson on 14/10/2022.
//

#include <drivers/vga.h>

using namespace maxOS::common;
using namespace maxOS::drivers;
using namespace maxOS::hardwarecommunication;


VideoGraphicsArray::VideoGraphicsArray()
:miscPort(0x3C2),
 crtcIndexPort(0x3D4),
 crtcDataPort(0x3D5),
 sequenceIndexPort(0x3C4),
 sequenceDataPort(0x3C5),
 graphicsControllerIndexPort(0x3CE),
 graphicsControllerDataPort(0x3CF),
 attributeControllerIndexPort(0x3C0),
 attributeControllerReadPort(0x3C1),
 attributeControllerWritePort(0x3C0),
 attributeControllerResetPort(0x3DA)
{

}

VideoGraphicsArray::~VideoGraphicsArray() {

}


void VideoGraphicsArray::WriteRegisters(uint8_t* registers)
{
    //MISC (1 val, hardcoded)
    miscPort.Write(*(registers++));                                     //Get pointer to register, write first to misc, and increase pointer

    //SEQ (5 vals, hardcoded)
    for (uint8_t i = 0; i < 5; i++ ) {
        sequenceIndexPort.Write(i);                                     //Tell where the data is to be written
        sequenceDataPort.Write(*(registers++));                         //Get pointer to register, first to sequence, and increase pointer
    }

    //Unlock CRTC controller (registers 0-7 of port 0x3D4 are write protected by the protect bit (bit 7 of index 0x11) so we must clear it to write to the registers .)
    crtcIndexPort.Write(0x03);
    crtcDataPort.Write(crtcDataPort.Read() | 0x80);                    //Set 0x03 (third's) first bit to 1
    crtcIndexPort.Write(0x11);
    crtcDataPort.Write(crtcDataPort.Read() | ~0x80);                   //Set 0x11 (eleventh's) first bit to 0

    //Make sure that the unlock isn't overwritten
    registers[0x03] = registers[0x03] | 0x80;                               //In the register 0x03 also set first 1
    registers[0x11] = registers[0x11] | ~0x80;                              //In the register 0x11 also set first 1

    //CRTC (25 vals, hardcoded)
    for (uint8_t i = 0; i < 25; i++ ) {
        crtcIndexPort.Write(i);                                         //Tell where the data is to be written
        crtcDataPort.Write(*(registers++));                             //Get pointer to register, write to cathode thingy, and increase pointer
    }

    //GC (9 vals, hardcoded)
    for(uint8_t i = 0; i < 9; i++)
    {
        ///HRS of bug hunting to figure out that I wrote index port twice
        graphicsControllerIndexPort.Write(i);                   //Tell where the data is to be writtens
        graphicsControllerDataPort.Write(*(registers++));       //Get pointer to register, write to graphics controller, and increase pointer
    }

    //AC (21 vals, hardcoded)
    for(uint8_t i = 0; i < 21; i++)
    {
        attributeControllerResetPort.Read();                        //Reset the Controller
        attributeControllerIndexPort.Write(i);                  //Tell where the data is to be written
        attributeControllerWritePort.Write(*(registers++));      //Get pointer to register, write to attribute controller, and increase pointer
    }

    //Re-Lock CRTC
    attributeControllerResetPort.Read();
    attributeControllerIndexPort.Write(0x20);

}

bool VideoGraphicsArray::SupportsMode(uint32_t width, uint32_t height, uint32_t colourdepth)
{
    return width == 320 && height == 200 && colourdepth == 8;
}


bool VideoGraphicsArray::SetMode(uint32_t width, uint32_t height, uint32_t colourdepth)
{
    if(!SupportsMode(width, height, colourdepth))
        return false;

    //Values from osdev / modes.c
    unsigned char g_320x200x256[] =
            {
                    /* MISC */
                    0x63,
                    /* SEQ */
                    0x03, 0x01, 0x0F, 0x00, 0x0E,
                    /* CRTC */
                    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
                    0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                    0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
                    0xFF,
                    /* GC */
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
                    0xFF,
                    /* AC */
                    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                    0x41, 0x00, 0x0F, 0x00, 0x00
            };

    WriteRegisters(g_320x200x256);
    return true;
}

uint8_t* VideoGraphicsArray::GetFrameBufferSegment()
{
    //Read data from index number 6
    graphicsControllerIndexPort.Write(0x06);
    uint8_t segmentNumber = graphicsControllerDataPort.Read() & (3<<2); //Shift by 2 as only intrested in bits 3 & 4 (& 3 so all the other bits are removed)
    switch(segmentNumber)
    {
        default:
        case 0<<2: return (uint8_t*)0x00000;
        case 1<<2: return (uint8_t*)0xA0000;
        case 2<<2: return (uint8_t*)0xB0000;
        case 3<<2: return (uint8_t*)0xB8000;
    }
}



void VideoGraphicsArray::PutPixel(uint32_t x, uint32_t y,  uint8_t colourIndex)
{
    uint8_t* pixelAddress = GetFrameBufferSegment() + 320*y + x;  //Get where to put the pixel in memory and x y pos
    *pixelAddress = colourIndex;
}

uint8_t VideoGraphicsArray::GetColourIndex(uint8_t r, uint8_t g, uint8_t b)
{
    if(r == 0x00, g == 0x00, b == 0xA8)
        return 0x01;
    return 0x00;
}

void VideoGraphicsArray::PutPixel(uint32_t x, uint32_t y,  uint8_t r, uint8_t g, uint8_t b)
{
    PutPixel(x,y, GetColourIndex(r,g,b));
}

void
VideoGraphicsArray::FillRectangle(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t r, uint8_t g, uint8_t b) {

    for (int32_t Y = y; Y < y+h; Y++) {
        for (int32_t X = x; X < x+w; X++) {
            PutPixel(X,Y,r,g,b);
        }
    }
}
