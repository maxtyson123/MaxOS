/**
 * @file video.h
 * @brief Defines a generic VideoDriver class for handling video modes and graphics context operations.
 *
 * @date 30th March 2023
 * @author Max Tyson
 */

#ifndef MAXOS_DRIVERS_VIDEO_VIDEO_H
#define MAXOS_DRIVERS_VIDEO_VIDEO_H

#include <stdint.h>
#include <drivers/driver.h>
#include <common/graphicsContext.h>

namespace MaxOS
{
    namespace drivers
    {
        namespace video
        {
            /**
             * @class VideoDriver
             * @brief Driver for the video controller, handles the setting of the video mode
             */
            class VideoDriver : public Driver, public common::GraphicsContext {

              protected:
                  virtual bool internal_set_mode(uint32_t width, uint32_t height, uint32_t color_depth);

              public:
                  VideoDriver();
                  ~VideoDriver();

                  virtual bool supports_mode(uint32_t width, uint32_t height, uint32_t color_depth);
                  bool set_mode(uint32_t width, uint32_t height, uint32_t color_depth);
            };

        }
    }
}

#endif //MAXOS_DRIVERS_VIDEO_VIDEO_H
