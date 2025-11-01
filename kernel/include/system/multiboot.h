/*   multiboot2.h - Multiboot 2 header file. */
/*   Copyright (C) 1999,2003,2007,2008,2009,2010  Free Software Foundation, Inc.
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

/*  How many bytes from the start of the file we search for the header. */
#define MULTIBOOT_SEARCH                        32768
#define MULTIBOOT_HEADER_ALIGN                  8

/*  The magic field should contain this. */
#define MULTIBOOT2_HEADER_MAGIC                 0xe85250d6

/*  This should be in %eax. */
#define MULTIBOOT2_BOOTLOADER_MAGIC             0x36d76289

/*  Alignment of multiboot modules. */
#define MULTIBOOT_MOD_ALIGN                     0x00001000

/*  Alignment of the multiboot info structure. */
#define MULTIBOOT_INFO_ALIGN                    0x00000008

/*  Flags set in the 'flags' member of the multiboot header. */

#define MULTIBOOT_TAG_ALIGN                  8
#define MULTIBOOT_TAG_TYPE_END               0
#define MULTIBOOT_TAG_TYPE_CMDLINE           1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME  2
#define MULTIBOOT_TAG_TYPE_MODULE            3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO     4
#define MULTIBOOT_TAG_TYPE_BOOTDEV           5
#define MULTIBOOT_TAG_TYPE_MMAP              6
#define MULTIBOOT_TAG_TYPE_VBE               7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER       8
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS      9
#define MULTIBOOT_TAG_TYPE_APM               10
#define MULTIBOOT_TAG_TYPE_EFI32             11
#define MULTIBOOT_TAG_TYPE_EFI64             12
#define MULTIBOOT_TAG_TYPE_SMBIOS            13
#define MULTIBOOT_TAG_TYPE_ACPI_OLD          14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW          15
#define MULTIBOOT_TAG_TYPE_NETWORK           16
#define MULTIBOOT_TAG_TYPE_EFI_MMAP          17
#define MULTIBOOT_TAG_TYPE_EFI_BS            18
#define MULTIBOOT_TAG_TYPE_EFI32_IH          19
#define MULTIBOOT_TAG_TYPE_EFI64_IH          20
#define MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR    21

#define MULTIBOOT_HEADER_TAG_END  0
#define MULTIBOOT_HEADER_TAG_INFORMATION_REQUEST  1
#define MULTIBOOT_HEADER_TAG_ADDRESS  2
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS  3
#define MULTIBOOT_HEADER_TAG_CONSOLE_FLAGS  4
#define MULTIBOOT_HEADER_TAG_FRAMEBUFFER  5
#define MULTIBOOT_HEADER_TAG_MODULE_ALIGN  6
#define MULTIBOOT_HEADER_TAG_EFI_BS        7
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI32  8
#define MULTIBOOT_HEADER_TAG_ENTRY_ADDRESS_EFI64  9
#define MULTIBOOT_HEADER_TAG_RELOCATABLE  10

#define MULTIBOOT_ARCHITECTURE_I386  0
#define MULTIBOOT_ARCHITECTURE_MIPS32  4
#define MULTIBOOT_HEADER_TAG_OPTIONAL 1

#define MULTIBOOT_LOAD_PREFERENCE_NONE 0
#define MULTIBOOT_LOAD_PREFERENCE_LOW 1
#define MULTIBOOT_LOAD_PREFERENCE_HIGH 2

#define MULTIBOOT_CONSOLE_FLAGS_CONSOLE_REQUIRED 1
#define MULTIBOOT_CONSOLE_FLAGS_EGA_TEXT_SUPPORTED 2

#ifndef ASM_FILE

typedef unsigned char multiboot_uint8_t;
typedef unsigned short multiboot_uint16_t;
typedef unsigned int multiboot_uint32_t;
typedef unsigned long long multiboot_uint64_t;

/**
 * @struct multiboot_header
 * @brief Contains the Multiboot 2 header information
 */
struct multiboot_header {
	multiboot_uint32_t magic;            ///< The magic number identifying the header (Must be MULTIBOOT2_HEADER_MAGIC)
	multiboot_uint32_t architecture;        ///< The architecture the OS image is for
	multiboot_uint32_t header_length;    ///< Total length of the multiboot header
	multiboot_uint32_t checksum;            ///< The fields of this structure added together must equal 0 mod 2^32
};

/**
 * @struct multiboot_header_tag
 * @brief Common header for all multiboot header tags
 */
struct multiboot_header_tag {
	multiboot_uint16_t type;              ///< The type of the tag
	multiboot_uint16_t flags;             ///< Flags (must be 0)
	multiboot_uint32_t size;              ///< The size of the tag including this header
};

/**
 * @struct multiboot_header_tag_information_request
 * @brief A tag requesting information from the bootloader
 */
struct multiboot_header_tag_information_request {
	multiboot_uint16_t type;              ///< *copydoc multiboot_header_tag::type
	multiboot_uint16_t flags;             ///< *copydoc multiboot_header_tag::flags
	multiboot_uint32_t size;              ///< *copydoc multiboot_header_tag::size
	multiboot_uint32_t requests[0];       ///< The tags being requested
};

/**
 * @struct multiboot_header_tag_address
 * @brief A tag specifying the load addresses of the OS image
 */
struct multiboot_header_tag_address {
	multiboot_uint16_t type;                  ///< *copydoc multiboot_header_tag::type
	multiboot_uint16_t flags;                 ///< *copydoc multiboot_header_tag::flags
	multiboot_uint32_t size;                  ///< *copydoc multiboot_header_tag::size
	multiboot_uint32_t header_addr;           ///< The physical address of the multiboot header
	multiboot_uint32_t load_addr;             ///< The physical address to load the image
	multiboot_uint32_t load_end_addr;         ///< The physical address of the end of the loaded image
	multiboot_uint32_t bss_end_addr;          ///< The physical address of the end of the bss section
};

/**
 * @struct multiboot_header_tag_entry_address
 * @brief A tag specifying the entry point address of the OS image
 */
struct multiboot_header_tag_entry_address {
	multiboot_uint16_t type;                  ///< *copydoc multiboot_header_tag::type
	multiboot_uint16_t flags;                 ///< *copydoc multiboot_header_tag::flags
	multiboot_uint32_t size;                  ///< *copydoc multiboot_header_tag::size
	multiboot_uint32_t entry_addr;            ///< The physical address of the entry point
};

/**
 * @struct multiboot_header_tag_console_flags
 * @brief A tag specifying the console flags
 */
struct multiboot_header_tag_console_flags {
	multiboot_uint16_t type;                  ///< *copydoc multiboot_header_tag::type
	multiboot_uint16_t flags;                 ///< *copydoc multiboot_header_tag::flags
	multiboot_uint32_t size;                  ///< *copydoc multiboot_header_tag::size
	multiboot_uint32_t console_flags;         ///< The console flags
};

/**
 * @struct multiboot_header_tag_framebuffer
 * @brief A tag specifying the VESA framebuffer settings
 */
struct multiboot_header_tag_framebuffer {
	multiboot_uint16_t type;                  ///< *copydoc multiboot_header_tag::type
	multiboot_uint16_t flags;                 ///< *copydoc multiboot_header_tag::flags
	multiboot_uint32_t size;                  ///< *copydoc multiboot_header_tag::size
	multiboot_uint32_t width;                 ///< The width of the framebuffer in pixels
	multiboot_uint32_t height;                ///< The height of the framebuffer in pixels
	multiboot_uint32_t depth;                 ///< The depth of the framebuffer in bits per pixel
};

/**
 * @struct multiboot_header_tag_module_align
 * @brief A tag specifying the module alignment
 */
struct multiboot_header_tag_module_align {
	multiboot_uint16_t type;                  ///< *copydoc multiboot_header_tag::type
	multiboot_uint16_t flags;                 ///< *copydoc multiboot_header_tag::flags
	multiboot_uint32_t size;                  ///< *copydoc multiboot_header_tag::size
};

/**
 * @struct multiboot_header_tag_relocatable
 * @brief A tag containing information of where the OS image can be loaded
 */
struct multiboot_header_tag_relocatable {
	multiboot_uint16_t type;                  ///< *copydoc multiboot_header_tag::type
	multiboot_uint16_t flags;                 ///< *copydoc multiboot_header_tag::flags
	multiboot_uint32_t size;                  ///< *copydoc multiboot_header_tag::size
	multiboot_uint32_t min_addr;              ///< The lowest address the image can be loaded at
	multiboot_uint32_t max_addr;              ///< The highest address the image can be loaded at
	multiboot_uint32_t align;                 ///< The alignment of the image
	multiboot_uint32_t preference;            ///< The load address preference (0 = none, 1 = low, 2 = high)
};

/**
 * @struct multiboot_color
 * @brief A structure representing a color in RGB format
 */
struct multiboot_color {
	multiboot_uint8_t red;                    ///< The amount of red
	multiboot_uint8_t green;                ///< The amount of green
	multiboot_uint8_t blue;                ///< The amount of blue
};

#define MULTIBOOT_MEMORY_AVAILABLE              1
#define MULTIBOOT_MEMORY_RESERVED               2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT_MEMORY_NVS                    4
#define MULTIBOOT_MEMORY_BADRAM                 5

/**
 * @struct multiboot_mmap_entry
 * @brief An entry in the memory map containing information about a region of memory
 */
struct multiboot_mmap_entry {
	multiboot_uint64_t addr;                  ///< The starting address of the memory region
	multiboot_uint64_t len;                   ///< The length of the memory region
	multiboot_uint32_t type;                  ///< The type of memory region
	multiboot_uint32_t zero;                  ///< Must be zero
};
typedef struct multiboot_mmap_entry multiboot_memory_map_t;

/**
 * @struct multiboot_tag
 * @brief Common header for all multiboot info tags
 */
struct multiboot_tag {
	multiboot_uint32_t type;                  ///< The type of the tag
	multiboot_uint32_t size;                  ///< The size of the tag including this header
};

/**
 * @struct multiboot_tag_string
 * @brief A tag containing a null-terminated string
 */
struct multiboot_tag_string {
	multiboot_uint32_t type;                  ///< *copydoc multiboot_tag::type
	multiboot_uint32_t size;                  ///< *copydoc multiboot_tag::size
	char string[0];                           ///< The string
};

/**
 * @struct multiboot_tag_module
 * @brief A tag containing information about a loaded module
 */
struct multiboot_tag_module {
	multiboot_uint32_t type;                  ///< *copydoc multiboot_tag::type
	multiboot_uint32_t size;                  ///< *copydoc multiboot_tag::size
	multiboot_uint32_t mod_start;             ///< The starting address of the module
	multiboot_uint32_t mod_end;               ///< The ending address of the module
	char cmdline[0];                          ///< The args passed to the module
};

/**
 * @struct multiboot_tag_basic_meminfo
 * @brief A tag containing basic (legacy) memory information
 */
struct multiboot_tag_basic_meminfo {
	multiboot_uint32_t type;                  ///< *copydoc multiboot_tag::type
	multiboot_uint32_t size;                  ///< *copydoc multiboot_tag::size
	multiboot_uint32_t mem_lower;             ///< The amount of lower memory in KB
	multiboot_uint32_t mem_upper;             ///< The amount of upper memory in KB (upper memory is the memory above 1MB, below 4GB)
};

/**
 * @struct multiboot_tag_bootdev
 * @brief A tag containing information about the device the OS image was loaded from
 */
struct multiboot_tag_bootdev {
	multiboot_uint32_t type;                  ///< *copydoc multiboot_tag::type
	multiboot_uint32_t size;                  ///< *copydoc multiboot_tag::size
	multiboot_uint32_t biosdev;               ///< The BIOS disk device the OS image was loaded from
	multiboot_uint32_t slice;                 ///< The slice number of the disk device
	multiboot_uint32_t part;                  ///< The partition number of the disk device
};

/**
 * @struct multiboot_tag_mmap
 * @brief A tag containing the collection of memory map entries that outline the memory layout
 */
struct multiboot_tag_mmap {
	multiboot_uint32_t type;                  ///< *copydoc multiboot_tag::type
	multiboot_uint32_t size;                  ///< *copydoc multiboot_tag::size
	multiboot_uint32_t entry_size;            ///< The size of each entry
	multiboot_uint32_t entry_version;         ///< The version of the entry structure (must be 0)
	struct multiboot_mmap_entry entries[0];   ///< The memory map entries
};

/**
 * @struct multiboot_vbe_info_block
 * @brief A tag containing information about the VESA BIOS Extensions (VBE)
 */
struct multiboot_vbe_info_block {
	multiboot_uint8_t external_specification[512];    ///< The VBE information block as defined by the VBE standard
};

/**
 * @struct multiboot_vbe_mode_info_block
 * @brief A tag containing information about a specific VBE video mode
 */
struct multiboot_vbe_mode_info_block {
	multiboot_uint8_t external_specification[256];    ///< The VBE mode information block as defined by the VBE standard
};

/**
 * @struct multiboot_tag_vbe
 * @brief A tag containing information about the VESA BIOS Extensions (VBE)
 */
struct multiboot_tag_vbe {
	multiboot_uint32_t type;                                  ///< *copydoc multiboot_tag::type
	multiboot_uint32_t size;                                  ///< *copydoc multiboot_tag::size

	multiboot_uint16_t vbe_mode;                              ///< The VBE mode
	multiboot_uint16_t vbe_interface_seg;                     ///< The segment of the VBE interface
	multiboot_uint16_t vbe_interface_off;                     ///< The offset of the VBE interface
	multiboot_uint16_t vbe_interface_len;                     ///< The length of the VBE interface

	struct multiboot_vbe_info_block vbe_control_info;         ///< The VBE control information
	struct multiboot_vbe_mode_info_block vbe_mode_info;       ///< The VBE mode information
};

#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB     1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT     2

/**
 * @struct multiboot_tag_framebuffer_common
 * @brief Common header for framebuffer tags
 */
struct multiboot_tag_framebuffer_common {
	multiboot_uint32_t type;                        ///< *copydoc multiboot_tag::type
	multiboot_uint32_t size;                        ///< *copydoc multiboot_tag::size

	multiboot_uint64_t framebuffer_addr;            ///< The physical address of the framebuffer
	multiboot_uint32_t framebuffer_pitch;           ///< The number of bytes per horizontal line
	multiboot_uint32_t framebuffer_width;           ///< The width of the framebuffer in pixels
	multiboot_uint32_t framebuffer_height;          ///< The height of the framebuffer in pixels
	multiboot_uint8_t framebuffer_bpp;              ///< The number of bits per pixel
	multiboot_uint8_t framebuffer_type;             ///< The type of framebuffer
	multiboot_uint16_t reserved;                    ///< 2 bytes reserved
};

/**
 * @struct multiboot_tag_framebuffer
 * @brief A tag containing information about the framebuffer
 *
 * @todo should use this to setup graphix context pallet?
 */
struct multiboot_tag_framebuffer {
	struct multiboot_tag_framebuffer_common common;                 ///< *copydoc multiboot_tag_framebuffer_common

	union {
		struct {
			multiboot_uint16_t framebuffer_palette_num_colors;      ///< The number of colors in the palette
			struct multiboot_color framebuffer_palette[0];          ///< The color palette
		};
		struct {
			multiboot_uint8_t framebuffer_red_field_position;       ///< The position of the red field
			multiboot_uint8_t framebuffer_red_mask_size;            ///< The size of the red mask
			multiboot_uint8_t framebuffer_green_field_position;     ///< The position of the green field
			multiboot_uint8_t framebuffer_green_mask_size;          ///< The size of the green mask
			multiboot_uint8_t framebuffer_blue_field_position;      ///< The position of the blue field
			multiboot_uint8_t framebuffer_blue_mask_size;           ///< The size of the blue mask
		};
	};
};

/**
 * @struct multiboot_tag_elf_sections
 * @brief A tag containing information about the ELF sections of the OS image
 */
struct multiboot_tag_elf_sections {
	multiboot_uint32_t type;            ///< *copydoc multiboot_tag::type
	multiboot_uint32_t size;            ///< *copydoc multiboot_tag::size
	multiboot_uint32_t num;             ///< The number of section headers
	multiboot_uint32_t entsize;         ///< The size of each section header
	multiboot_uint32_t shndx;           ///< The section header string table index
	char sections[0];                   ///< The section headers
};

/**
 * @struct multiboot_tag_apm
 * @brief A tag containing information about the Advanced Power Management (APM) BIOS
 */
struct multiboot_tag_apm {
	multiboot_uint32_t type;        ///< *copydoc multiboot_tag::type
	multiboot_uint32_t size;        ///< *copydoc multiboot_tag::size
	multiboot_uint16_t version;     ///< The APM version
	multiboot_uint16_t cseg;        ///< The code segment
	multiboot_uint32_t offset;      ///< The offset
	multiboot_uint16_t cseg_16;     ///< The 16-bit code segment
	multiboot_uint16_t dseg;        ///< The data segment
	multiboot_uint16_t flags;       ///< The flags
	multiboot_uint16_t cseg_len;    ///< The length of the code segment
	multiboot_uint16_t cseg_16_len; ///< The length of the 16-bit code segment
	multiboot_uint16_t dseg_len;    ///< The length of the data segment
};

/**
 * @struct multiboot_tag_efi32
 * @brief A tag containing information about the EFI 32-bit system table
 */
struct multiboot_tag_efi32 {
	multiboot_uint32_t type;        ///< *copydoc multiboot_tag::type
	multiboot_uint32_t size;        ///< *copydoc multiboot_tag::size
	multiboot_uint32_t pointer;     ///< Where the EFI system table is located
};

/**
 * @struct multiboot_tag_efi64
 * @brief A tag containing information about the EFI 64-bit system table
 */
struct multiboot_tag_efi64 {
	multiboot_uint32_t type;        ///< *copydoc multiboot_tag::type
	multiboot_uint32_t size;        ///< *copydoc multiboot_tag::size
	multiboot_uint64_t pointer;     ///< Where the EFI system table is located
};

/**
 * @struct multiboot_tag_smbios
 * @brief A tag containing information about the System Management BIOS (SMBIOS)
 */
struct multiboot_tag_smbios {
	multiboot_uint32_t type;            ///< *copydoc multiboot_tag::type
	multiboot_uint32_t size;            ///< *copydoc multiboot_tag::size
	multiboot_uint8_t major;            ///< The major version of SMBIOS
	multiboot_uint8_t minor;            ///< The minor version of SMBIOS
	multiboot_uint8_t reserved[6];      ///< 6 bytes reserved
	multiboot_uint8_t tables[0];        ///< The SMBIOS tables
};

/**
 * @struct multiboot_tag_old_acpi
 * @brief A tag containing information about the old ACPI RSDP
 */
struct multiboot_tag_old_acpi {
	multiboot_uint32_t type;        ///< *copydoc multiboot_tag::type
	multiboot_uint32_t size;        ///< *copydoc multiboot_tag::size
	multiboot_uint8_t rsdp[0];      ///< The RSDP structure
};

/**
 * @struct multiboot_tag_new_acpi
 * @brief A tag containing information about the new ACPI RSDP
 */
struct multiboot_tag_new_acpi {
	multiboot_uint32_t type;        ///< *copydoc multiboot_tag::type
	multiboot_uint32_t size;        ///< *copydoc multiboot_tag::size
	multiboot_uint8_t rsdp[0];      ///< The RSDP structure
};

/**
 * @struct multiboot_tag_network
 * @brief A tag containing information about the network booting
 */
struct multiboot_tag_network {
	multiboot_uint32_t type;            ///< *copydoc multiboot_tag::type
	multiboot_uint32_t size;            ///< *copydoc multiboot_tag::size
	multiboot_uint8_t dhcpack[0];       ///< The DHCP ACK packet
};

/**
 * @struct multiboot_tag_efi_mmap
 * @brief A tag containing the EFI memory map
 */
struct multiboot_tag_efi_mmap {
	multiboot_uint32_t type;            ///< *copydoc multiboot_tag::type
	multiboot_uint32_t size;            ///< *copydoc multiboot_tag::size
	multiboot_uint32_t descr_size;      ///< The size of each EFI memory descriptor
	multiboot_uint32_t descr_vers;      ///< The version of the EFI memory descriptor
	multiboot_uint8_t efi_mmap[0];      ///< The EFI memory map
};

/**
 * @struct multiboot_tag_efi32_ih
 * @brief A tag containing the 32bit EFI image handle
 */
struct multiboot_tag_efi32_ih {
	multiboot_uint32_t type;            ///< *copydoc multiboot_tag::type
	multiboot_uint32_t size;            ///< *copydoc multiboot_tag::size
	multiboot_uint32_t pointer;         ///< Where the EFI image handle is located
};

/**
 * @struct multiboot_tag_efi64_ih
 * @brief A tag containing the 64bit EFI image handle
 */
struct multiboot_tag_efi64_ih {
	multiboot_uint32_t type;        ///< *copydoc multiboot_tag::type
	multiboot_uint32_t size;        ///< *copydoc multiboot_tag::size
	multiboot_uint64_t pointer;     ///< Where the EFI image handle is located
};

/**
 * @struct multiboot_tag_load_base_addr
 * @brief A tag containing the load base address of the OS image
 */
struct multiboot_tag_load_base_addr {
	multiboot_uint32_t type;            ///< *copydoc multiboot_tag::type
	multiboot_uint32_t size;            ///< *copydoc multiboot_tag::size
	multiboot_uint32_t load_base_addr;  ///< Where the OS image was loaded
};

namespace MaxOS {
	namespace system {

		/**
		 * @class Multiboot
		 * @brief Parses and provides access to Multiboot 2 information
		 */
		class Multiboot {
			private:
				multiboot_tag_framebuffer*      m_framebuffer;
				multiboot_tag_basic_meminfo*    m_basic_meminfo;
				multiboot_tag_string*           m_bootloader_name;
				multiboot_tag_mmap*             m_mmap;
				multiboot_tag_old_acpi*         m_old_acpi;
				multiboot_tag_new_acpi*         m_new_acpi;
				multiboot_tag_module*           m_module;

			public:
				Multiboot(unsigned long address, unsigned long magic);
				~Multiboot();

				multiboot_tag_framebuffer* framebuffer();
				multiboot_tag_basic_meminfo* basic_meminfo();
				multiboot_tag_string* bootloader_name();
				multiboot_tag_mmap* mmap();
				multiboot_tag_old_acpi* old_acpi();
				multiboot_tag_new_acpi* new_acpi();

				unsigned long start_address;    ///< The start address of the multiboot info struct
				unsigned long end_address;      ///< The end address of the multiboot info struct

				[[nodiscard]] multiboot_tag* start_tag() const;

				bool is_reserved(multiboot_uint64_t address);

		};
	}
}


#endif /*  ! ASM_FILE */

#endif /*  ! MULTIBOOT_HEADER */