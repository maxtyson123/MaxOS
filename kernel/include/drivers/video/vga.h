//
// Created by Tyson on 14/10/2022.
//

#ifndef MaxOS_DRIVERS_VGA_H
#define MaxOS_DRIVERS_VGA_H

#include <common/types.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/port.h>
#include <drivers/video/video.h>

namespace maxOS{

    namespace drivers{

        namespace video{

            class VideoGraphicsArray : public VideoDriver{
                protected:
                    common::uint8_t* FrameBufferSegment;
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

                    void WriteRegisters(common::uint8_t* registers);                //Send Initialization codes to corresponding port
                    common::uint8_t* GetFrameBufferSegment();                       //Get offset for segment wanted to use

                    bool internalSetMode(common::uint32_t width, common::uint32_t height, common::uint32_t colourDepth);
                    void renderPixel8Bit(common::uint32_t x, common::uint32_t y, common::uint8_t colour);
                    common::uint8_t getRenderedPixel8Bit(common::uint32_t x, common::uint32_t y);

                public:
                       VideoGraphicsArray();
                       ~VideoGraphicsArray();

                       bool supportsMode(common::uint32_t width, common::uint32_t height, common::uint32_t colourDepth);

                       inline void* operator new(common::uint32_t, VideoGraphicsArray* p)
                       {
                            return p;
                       }
               };
        }

    }
}

#endif //MaxOS_DRIVERS_VGA_H
