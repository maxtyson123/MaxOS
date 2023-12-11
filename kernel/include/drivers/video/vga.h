//
// Created by Tyson on 14/10/2022.
//

#ifndef MaxOS_DRIVERS_VGA_H
#define MaxOS_DRIVERS_VGA_H

#include <stdint.h>
#include <common/string.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/port.h>
#include <drivers/video/video.h>

namespace maxOS{

    namespace drivers{

        namespace video{

            class VideoGraphicsArray : public VideoDriver{
                protected:
                    uint8_t* FrameBufferSegment;
                    //-The CRT Controller (CRTC) is responsible for controlling the output of video data to the display monitor.
                    //-The graphics controller is responsible for managing the interface between CPU and video memory.
                    //-The sequencer manages the interface between the video data and RAMDAC.

                    hardwarecommunication::Port8Bit miscPort;                       //Miscellaneous
                    hardwarecommunication::Port8Bit crtcIndexPort;                  //cathode ray tube controller (index)
                    hardwarecommunication::Port8Bit crtcDataPort;                   //cathode ray tube controller (data)
                    hardwarecommunication::Port8Bit sequenceIndexPort;              //Sequence Index
                    hardwarecommunication::Port8Bit sequenceDataPort;               //Sequence Data
                    hardwarecommunication::Port8Bit graphicsControllerIndexPort;    //Graphics Controller Index
                    hardwarecommunication::Port8Bit graphicsControllerDataPort;     //Graphics Controller Data
                    hardwarecommunication::Port8Bit attributeControllerIndexPort;   //Attribute Controller Index
                    hardwarecommunication::Port8Bit attributeControllerReadPort;    //Attribute Controller Read
                    hardwarecommunication::Port8Bit attributeControllerWritePort;   //Attribute Controller Write
                    hardwarecommunication::Port8Bit attributeControllerResetPort;   //Attribute Controller Reset

                    void WriteRegisters(uint8_t* registers);                //Send Initialization codes to corresponding port
                    uint8_t* GetFrameBufferSegment();                       //Get offset for segment wanted to use

                    bool internalSetMode(uint32_t width, uint32_t height, uint32_t colourDepth);
                    void renderPixel8Bit(uint32_t x, uint32_t y, uint8_t colour);
                    uint8_t getRenderedPixel8Bit(uint32_t x, uint32_t y);

                public:
                       VideoGraphicsArray();
                       ~VideoGraphicsArray();

                        string getVendorName();
                        string getDeviceName();

                       bool supportsMode(uint32_t width, uint32_t height, uint32_t colourDepth);
               };
        }

    }
}

#endif //MaxOS_DRIVERS_VGA_H
