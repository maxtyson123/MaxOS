//
// Created by 98max on 19/07/2023.
//

#ifndef MAXOS_VIDEO_VESA_H
#define MAXOS_VIDEO_VESA_H

#include <common/types.h>
#include <drivers/video/video.h>
#include <memory/memorymanagement.h>


// Thanks to https://wiki.osdev.org/User:Omarrx024/VESA_Tutorial for the VESA documentation

namespace maxOS {

    namespace drivers {

        namespace video {

            struct vbe_info_structure {
                char[4] signature = "VESA";	            // must be "VESA" to indicate valid VBE support
                common::uint16_t version;			    // VBE version; high byte is major version, low byte is minor version
                common::uint32_t oem;			        // segment:offset pointer to OEM
                common::uint32_t capabilities;		    // bitfield that describes card capabilities
                common::uint32_t video_modes;		    // segment:offset pointer to list of supported video modes
                common::uint16_t video_memory;		    // amount of video memory in 64KB blocks
                common::uint16_t software_rev;		    // software revision
                common::uint32_t vendor;			    // segment:offset to card vendor string
                common::uint32_t product_name;		    // segment:offset to card model name
                common::uint32_t product_rev;		    // segment:offset pointer to product revision
                char reserved[222];		                // reserved for future expansion
                char oem_data[256];		                // OEM BIOSes store their strings in this area
            } __attribute__ ((packed));

            class VideoElectronicsStandardsAssociationDriver : public VideoDriver {
                protected:
                    bool internalSetMode(common::uint32_t width, common::uint32_t height, common::uint32_t colorDepth);
                    void renderPixel32Bit(common::uint32_t x, common::uint32_t y, common::uint32_t colour);
                    memory::MemoryManager* memoryManager;
                    common::uint8_t* framebufferAddress;

                public:
                    VideoElectronicsStandardsAssociationDriver(memory::MemoryManager* memoryManager);
                    ~VideoElectronicsStandardsAssociationDriver();

                    bool supportsMode(common::uint32_t width, common::uint32_t height, common::uint32_t colorDepth);

            };

        }
    }
}

#endif //MAXOS_VIDEO_VESA_H
