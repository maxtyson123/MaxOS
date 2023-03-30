//
// Created by 98max on 30/03/2023.
//

#ifndef MAXOS_DRIVERS_VIDEO_VIDEO_H
#define MAXOS_DRIVERS_VIDEO_VIDEO_H

#include <drivers/driver.h>

namespace maxOS
{
    namespace drivers
    {
        namespace video
        {
            class VideoDriver : public Driver, public common::GraphicsContext
            {
                public:
                    VideoDriver();
                    ~VideoDriver();
            };
        }
    }
}

#endif //MAXOS_DRIVERS_VIDEO_VIDEO_H
