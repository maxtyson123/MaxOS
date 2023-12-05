//
// Created by 98max on 30/03/2023.
//

#ifndef MAXOS_DRIVERS_VIDEO_VIDEO_H
#define MAXOS_DRIVERS_VIDEO_VIDEO_H

#include <stdint.h>
#include <drivers/driver.h>
#include <common/graphicsContext.h>

namespace maxOS
{
    namespace drivers
    {
        namespace video
        {
            class VideoDriver : public Driver, public common::GraphicsContext {

            protected:
                virtual bool internalSetMode(uint32_t width, uint32_t height, uint32_t colorDepth);

            public:
                VideoDriver();
                ~VideoDriver();

                virtual bool supportsMode(uint32_t width, uint32_t height, uint32_t colorDepth);
                bool setMode(uint32_t width, uint32_t height, uint32_t colorDepth);
                virtual bool setTextMode();
            };

        }
    }
}

#endif //MAXOS_DRIVERS_VIDEO_VIDEO_H
