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

            /**
             * @class VideoGraphicsArray
             * @brief Driver for the VGA graphics controller, handles the rendering of pixels to the screen
             */
            class VideoGraphicsArray : public VideoDriver{
                protected:
                    hardwarecommunication::Port8Bit m_misc_port;
                    hardwarecommunication::Port8Bit m_crtc_index_port;
                    hardwarecommunication::Port8Bit crtc_data_port;
                    hardwarecommunication::Port8Bit m_sequence_index_port;
                    hardwarecommunication::Port8Bit m_sequence_data_port;
                    hardwarecommunication::Port8Bit m_graphics_controller_index_port;
                    hardwarecommunication::Port8Bit m_graphics_controller_data_port;
                    hardwarecommunication::Port8Bit m_attribute_controller_index_port;
                    hardwarecommunication::Port8Bit m_attribute_controller_read_port;
                    hardwarecommunication::Port8Bit m_attribute_controller_write_port;
                    hardwarecommunication::Port8Bit m_attribute_controller_reset_port;

                    void write_registers(uint8_t* registers);
                    uint8_t* get_frame_buffer_segment();

                    bool internal_set_mode(uint32_t width, uint32_t height, uint32_t colour_depth) final;
                    void render_pixel_8_bit(uint32_t x, uint32_t y, uint8_t colour) final;
                    uint8_t get_rendered_pixel_8_bit(uint32_t x, uint32_t y) final;

                public:
                       VideoGraphicsArray();
                       ~VideoGraphicsArray();

                        string get_vendor_name() final;
                        string get_device_name() final;

                       bool supports_mode(uint32_t width, uint32_t height, uint32_t colour_depth) final;
           };
        }
    }
}

#endif //MaxOS_DRIVERS_VGA_H
