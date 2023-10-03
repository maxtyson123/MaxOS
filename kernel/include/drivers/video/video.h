//
// Created by 98max on 30/03/2023.
//

#ifndef MAXOS_DRIVERS_VIDEO_VIDEO_H
#define MAXOS_DRIVERS_VIDEO_VIDEO_H

#include <drivers/driver.h>
#include <common/graphicsContext.h>
#include <common/types.h>

namespace maxOS
{
    namespace drivers
    {
        namespace video
        {
            class VideoDriver : public Driver, public common::GraphicsContext {

            protected:
                virtual bool internalSetMode(common::uint32_t width, common::uint32_t height, common::uint32_t colorDepth);

            public:
                VideoDriver();
                ~VideoDriver();

                virtual bool supportsMode(common::uint32_t width, common::uint32_t height, common::uint32_t colorDepth);
                bool setMode(common::uint32_t width, common::uint32_t height, common::uint32_t colorDepth);
                virtual bool setTextMode();
            };

        }
    }
}

#endif //MAXOS_DRIVERS_VIDEO_VIDEO_H
