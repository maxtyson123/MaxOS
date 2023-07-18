/* multiboot.h - Multiboot header file.
 * Copyright (C) 1999,2003,2007,2008,2009  Free Software Foundation, Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ANY
 *  DEVELOPER OR DISTRIBUTOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MULTIBOOT_HEADER

#define MULTIBOOT_HEADER 1

#include <common/types.h>

namespace maxOS
{
    namespace system {

    /* How many bytes from the start of the file we search for the header. */
    #define MULTIBOOT_SEARCH                        8192

    /* The magic field should contain this. */
    #define MULTIBOOT_HEADER_MAGIC                  0x1BADB002

    /* This should be in %eax. */
    #define MULTIBOOT_BOOTLOADER_MAGIC              0x2BADB002

    /* The bits in the required part of flags field we don't support. */
    #define MULTIBOOT_UNSUPPORTED                   0x0000fffc

    /* Alignment of multiboot modules. */
    #define MULTIBOOT_MOD_ALIGN                     0x00001000

    /* Alignment of the multiboot info structure. */
    #define MULTIBOOT_INFO_ALIGN                    0x00000004

    /* Flags set in the 'flags' member of the multiboot header. */

    /* Align all boot modules on i386 page (4KB) boundaries. */
    #define MULTIBOOT_PAGE_ALIGN                    0x00000001

    /* Must pass memory information to OS. */
    #define MULTIBOOT_MEMORY_INFO                   0x00000002

    /* Must pass video information to OS. */
    #define MULTIBOOT_VIDEO_MODE                    0x00000004

    /* This flag indicates the use of the address fields in the header. */
    #define MULTIBOOT_AOUT_KLUDGE                   0x00010000

    /* Flags to be set in the 'flags' member of the multiboot info structure. */

    /* is there basic lower/upper memory information? */
    #define MULTIBOOT_INFO_MEMORY                   0x00000001
    /* is there a boot device set? */
    #define MULTIBOOT_INFO_BOOTDEV                  0x00000002
    /* is the command-line defined? */
    #define MULTIBOOT_INFO_CMDLINE                  0x00000004
    /* are there modules to do something with? */
    #define MULTIBOOT_INFO_MODS                     0x00000008

    /* These next two are mutually exclusive */

    /* is there a symbol table loaded? */
    #define MULTIBOOT_INFO_AOUT_SYMS                0x00000010
    /* is there an ELF section header table? */
    #define MULTIBOOT_INFO_ELF_SHDR                 0X00000020

    /* is there a full memory map? */
    #define MULTIBOOT_INFO_MEM_MAP                  0x00000040

    /* Is there drive info? */
    #define MULTIBOOT_INFO_DRIVE_INFO               0x00000080

    /* Is there a config table? */
    #define MULTIBOOT_INFO_CONFIG_TABLE             0x00000100

    /* Is there a boot loader name? */
    #define MULTIBOOT_INFO_BOOT_LOADER_NAME         0x00000200

    /* Is there a APM table? */
    #define MULTIBOOT_INFO_APM_TABLE                0x00000400

    /* Is there video information? */
    #define MULTIBOOT_INFO_VIDEO_INFO               0x00000800



    /* The symbol table for a.out. */
    struct multiboot_aout_symbol_table
    {
        common::uint32_t tabsize;
        common::uint32_t strsize;
        common::uint32_t addr;
        common::uint32_t reserved;
    };
    typedef struct multiboot_aout_symbol_table multiboot_aout_symbol_table_t;



    /* The section header table for ELF. */
    struct multiboot_elf_section_header_table
    {
        common::uint32_t num;
        common::uint32_t size;
        common::uint32_t addr;
        common::uint32_t shndx;
    };
    typedef struct multiboot_elf_section_header_table multiboot_elf_section_header_table_t;



    struct multiboot_info
    {
        /* Multiboot info version number */
        common::uint32_t flags;

        /* Available memory from BIOS */
        common::uint32_t mem_lower;
        common::uint32_t mem_upper;
        common::uint32_t boot_device;  // "root" partition
        common::uint32_t cmdline;      // Kernel command line

        /* Boot-Module list */
        common::uint32_t mods_count;
        common::uint32_t mods_addr;

        union
        {
            multiboot_aout_symbol_table_t aout_sym;
            multiboot_elf_section_header_table_t elf_sec;
        } u;

        /* Memory Mapping buffer */
        common::uint32_t mmap_length;
        common::uint32_t mmap_addr;

        /* Drive Info buffer */
        common::uint32_t drives_length;
        common::uint32_t drives_addr;

        common::uint32_t rom_config_table; // ROM configuration table
        common::uint32_t boot_loader_name; // Boot Loader Name

        common::uint32_t apm_table;        // APM table

        /* Video */
        common::uint32_t vbe_control_info;
        common::uint32_t vbe_mode_info;
        common::uint16_t vbe_mode;
        common::uint16_t vbe_interface_seg;
        common::uint16_t vbe_interface_off;
        common::uint16_t vbe_interface_len;
    };
    typedef struct multiboot_info multiboot_info_t;

    }
}

#endif /* ! MULTIBOOT_HEADER */
