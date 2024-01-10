//
// Created by 98max on 19/07/2023.
//

#ifndef MAXOS_VIDEO_VESA_H
#define MAXOS_VIDEO_VESA_H

#include <stdint.h>
#include <common/string.h>
#include <drivers/video/video.h>
#include <memory/memorymanagement.h>
#include <system/multiboot.h>
#include <memory/memoryIO.h>


namespace maxOS {

    namespace drivers {

        namespace video {

            /**
             * @class VideoElectronicsStandardsAssociation
             * @brief Driver for the VESA video controller, handles the rendering of pixels to the screen using VESA
             */
            class VideoElectronicsStandardsAssociation : public VideoDriver {

                private:
                    bool init();

                protected:
                    bool internal_set_mode(uint32_t width,  uint32_t height,  uint32_t) final;

                    void render_pixel_32_bit( uint32_t x,  uint32_t y,  uint32_t colour) final;
                    uint32_t get_rendered_pixel_32_bit(uint32_t x, uint32_t y) final;

                    multiboot_tag_framebuffer* m_framebuffer_info;

                    uint64_t* m_framebuffer_address;
                    uint8_t m_bpp;
                    uint16_t m_pitch;

                public:
                    VideoElectronicsStandardsAssociation(multiboot_tag_framebuffer* framebuffer_info);
                    ~VideoElectronicsStandardsAssociation();

                    bool supports_mode( uint32_t width,  uint32_t height,  uint32_t) final;

                    string get_vendor_name() final;
                    string get_device_name() final;

            };

        }
    }
}

#endif //MAXOS_VIDEO_VESA_H
