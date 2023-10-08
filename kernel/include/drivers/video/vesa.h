//
// Created by 98max on 19/07/2023.
//

#ifndef MAXOS_VIDEO_VESA_H
#define MAXOS_VIDEO_VESA_H

#include <common/types.h>
#include <drivers/video/video.h>
#include <memory/memorymanagement.h>
#include <system/multiboot.h>
#include <memory/memoryIO.h>

// Thanks to https://wiki.osdev.org/User:Omarrx024/VESA_Tutorial for the VESA documentation

namespace maxOS {

    namespace drivers {

        namespace video {

            typedef struct vbe_info_structure {
                char signature[4];	// must be "VESA" to indicate valid VBE support
                 common::uint16_t version;			// VBE version; high byte is major version, low byte is minor version
                 common::uint32_t oem;			// segment:offset pointer to OEM
                 common::uint32_t capabilities;		// bitfield that describes card capabilities
                 common::uint32_t video_modes;		// segment:offset pointer to list of supported video modes
                 common::uint16_t video_memory;		// amount of video memory in 64KB blocks
                 common::uint16_t software_rev;		// software revision
                 common::uint32_t vendor;			// segment:offset to card vendor string
                 common::uint32_t product_name;		// segment:offset to card model name
                 common::uint32_t product_rev;		// segment:offset pointer to product revision
                char reserved[222];		// reserved for future expansion
                char oem_data[256];		// OEM BIOSes store their strings in this area
            } __attribute__ ((packed)) vesa_control_info_t;

            typedef struct vbe_mode_info_structure {
                 common::uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
                 common::uint8_t window_a;			// deprecated
                 common::uint8_t window_b;			// deprecated
                 common::uint16_t granularity;		// deprecated; used while calculating bank numbers
                 common::uint16_t window_size;
                 common::uint16_t segment_a;
                 common::uint16_t segment_b;
                 common::uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
                 common::uint16_t pitch;			// number of bytes per horizontal line
                 common::uint16_t width;			// width in pixels
                 common::uint16_t height;			// height in pixels
                 common::uint8_t w_char;			// unused...
                 common::uint8_t y_char;			// ...
                 common::uint8_t planes;
                 common::uint8_t bpp;			// bits per pixel in this mode
                 common::uint8_t banks;			// deprecated; total number of banks in this mode
                 common::uint8_t memory_model;
                 common::uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
                 common::uint8_t image_pages;
                 common::uint8_t reserved0;

                 common::uint8_t red_mask;
                 common::uint8_t red_position;
                 common::uint8_t green_mask;
                 common::uint8_t green_position;
                 common::uint8_t blue_mask;
                 common::uint8_t blue_position;
                 common::uint8_t reserved_mask;
                 common::uint8_t reserved_position;
                 common::uint8_t direct_color_attributes;

                 common::uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
                 common::uint32_t off_screen_mem_off;
                 common::uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
                 common::uint8_t reserved1[206];
            } __attribute__ ((packed)) vesa_mode_info_t;

            typedef struct vbe2_pmi_table {
                 common::uint16_t set_window;		// offset in table for protected mode code for function 0x4F05
                 common::uint16_t set_display_start;	// offset in table for protected mode code for function 0x4F07
                 common::uint16_t set_pallette;		// offset in table for protected mode code for function 0x4F09
            } __attribute__ ((packed)) vbe2_pmi_table_t;

            class VideoElectronicsStandardsAssociationDriver : public VideoDriver {

                private:
                    vesa_control_info_t* vesaControlInfo;
                    vesa_mode_info_t* vesaModeInfo;
                    vbe2_pmi_table_t* vbe2PmiTable;
                    common::uint16_t vesaMode;

                    bool init();

                protected:
                    bool internalSetMode( common::uint32_t width,  common::uint32_t height,  common::uint32_t colorDepth);

                    void renderPixel32Bit( common::uint32_t x,  common::uint32_t y,  common::uint32_t colour);
                    common::uint32_t getRenderedPixel32Bit(common::uint32_t x, common::uint32_t y);

                    system::multiboot_info_t* multibootInfo;

                    common::uint32_t* framebufferAddress;
                    common::uint8_t bpp;			// bits per pixel in this mode
                    common::uint16_t pitch;			// number of bytes per horizontal line

                public:
                    VideoElectronicsStandardsAssociationDriver(system::multiboot_info_t* mb_info);
                    ~VideoElectronicsStandardsAssociationDriver();

                    bool supportsMode( common::uint32_t width,  common::uint32_t height,  common::uint32_t colorDepth);

                    common::string getVendorName();
                    common::string getDeviceName();

            };

        }
    }
}

#endif //MAXOS_VIDEO_VESA_H
