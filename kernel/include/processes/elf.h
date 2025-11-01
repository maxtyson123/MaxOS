/**
 * @file elf.h
 * @brief Defines structures and enums for handling ELF (Executable and Linkable Format) files in MaxOS.
 *
 * @date 21st March 2025
 * @author Max Tyson
 */

#ifndef MAXOS_PROCESSES_ELF_H
#define MAXOS_PROCESSES_ELF_H

#include <stdint.h>
#include <stddef.h>

#include <memory/memorymanagement.h>
#include <memory/memoryIO.h>

namespace MaxOS {
	namespace processes {

		/// The expected ELF magic number at the start of the file
		constexpr char ELF_MAGIC[4] = { 0x7F, 'E', 'L', 'F' };

		/**
		 * @enum ELFIdentification
		 * @brief The indexes of the ELF identification array
		 */
		enum class ELFIdentification {
			Magic0,
			Magic1,
			Magic2,
			Magic3,
			Class,
			Data,
			Version,
			OSABI,
			ABIVersion,
			Padding,
		};

		/**
		 * @enum ELFClass
		 * @brief The class of the ELF file (32-bit or 64-bit)
		 */
		enum class ELFClass {
			Invalid,
			Bits32,
			Bits64
		};

		/**
		 * @enum ELFData
		 * @brief The data encoding of the ELF file (little-endian or big-endian)
		 */
		enum class ELFData {
			InvalidData,
			LittleEndian,
			BigEndian
		};

		/**
		 * @enum ELFType
		 * @brief The type of the ELF file (executable, shared object, etc.)
		 */
		enum class ELFType {
			None,
			Relocatable,
			Executable,
			Shared,
			Core,
			ProcessorSpecificLow = 0xFF0,
			ProcessorSpecificHigh = 0xFFF
		};

		/**
		 * @enum ELFMachine
		 * @brief The target architecture of the ELF file
		 */
		enum class ELFMachine {
			NoMachine,
			ATnTWe32100,
			SPARC,
			x86,
			MIPS        = 0x8,
			PowerPC     = 0x14,
			ARM         = 0x28,
			SuperH      = 0x2A,
			IA_64       = 0x32,
			x86_64      = 0x3E,
			AArch64     = 0xB7,
			RISC_V      = 0xF3
		};

		/**
		 * @enum ELFVersion
		 * @brief The version of the ELF file
		 */
		enum class ELFVersion {
			Invalid,
			Current
		};

		/**
		 * @struct ELFHeader64
		 * @brief The header of a 64-bit ELF file
		 */
		typedef struct ELFHeader64 {
			unsigned char identification[16];   ///< Identifies the file as an ELF file and contains metadata about the file (class, data encoding, version, OS ABI, etc.)
			uint16_t type;                      ///< The type of the ELF file (see ELFType)
			uint16_t machine;                   ///< The target architecture of the ELF file (see ELFMachine)
			uint32_t version;                   ///< The version of the ELF file (see ELFVersion)
			uint64_t entry;                     ///< The starting RIP address where the program begins execution
			uint64_t program_header_offset;     ///< The offset in the file where the program header table starts
			uint64_t section_header_offset;     ///< The offset in the file where the section header table starts
			uint32_t flags;                     ///< Architecture-specific flags
			uint16_t elf_header_size;           ///< The size of this ELF header
			uint16_t program_header_size;       ///< The size of each entry in the program header table
			uint16_t program_header_count;      ///< The number of entries in the program header table
			uint16_t section_header_size;       ///< The size of each entry in the section header table
			uint16_t section_header_count;      ///< The number of entries in the section header table
			uint16_t section_header_string_table_index; ///< Where the section header string table is located (index into the section header table)
		} elf_64_header_t;

		/**
		 * @enum ELFProgramType
		 * @brief The type of a program header in the ELF file
		 */
		enum class ELFProgramType {
			Null,
			Load,
			Dynamic,
			Interpreter,
			Note,
			SharedLibrary,
			HeaderTable,
			TableProcessorSpecificLow = 0x70000000,
			TableProcessorSpecificHigh = 0x7FFFFFFF
		};

		/**
		 * @enum ELFProgramFlags
		 * @brief The flags of a program header in the ELF file
		 */
		enum ELFProgramFlags {
			ELFExecute  = (1 << 0),
			ELFWrite    = (1 << 1),
			ELFRead     = (1 << 2)
		};

		/**
		 * @struct ELFProgramHeader64
		 * @brief The header for the program segments in a 64-bit ELF file
		 */
		typedef struct ELFProgramHeader64 {
			uint32_t type;                  ///< The type of the program header (see ELFProgramType)
			uint32_t flags;                 ///< The flags of the program header (see ELFProgramFlags)
			uint64_t offset;                ///< The offset in the file where the segment is located
			uint64_t virtual_address;       ///< Where the segment is to be loaded in memory
			uint64_t physical_address;      ///< Reserved for physical address (not used in modern systems)
			uint64_t file_size;             ///< The size of the segment in the file
			uint64_t memory_size;           ///< The size of the segment in memory (should be >= file_size)
			uint64_t align;                 ///< The required alignment of the segment in memory (normally 2^n)
		} elf_64_program_header_t;

		/**
		 * @enum ELFSectionType
		 * @brief The type of a section in the ELF file
		 */
		enum class ELFSectionType {
			Null,
			ProgramBits,
			SymbolTable,
			StringTable,
			RelocationAddends,
			SymbolHashTable,
			DynamicLinkingTable,
			SectionNote,
			NoBits,
		};

		/**
		 * @struct ELFSectionHeader64
		 * @brief The header for the sections in a 64-bit ELF file
		 */
		typedef struct ELFSectionHeader64 {
			uint32_t name_index;            ///< The index into the section header string table where the name of this section is located
			uint32_t type;                  ///< The type of the section (see ELFSectionType)
			uint64_t sh_flags;              ///< The flags of the section (see ELFSectionFlags)
			uint64_t virtual_address;       ///< Where the section is to be loaded in memory
			uint64_t offset;                ///< The offset in the file where the section is located
			uint64_t size;                  ///< The size of the section in the file
			uint32_t link;                  ///< Section index link (meaning depends on section type)
			uint32_t info;                  ///< Extra information (meaning depends on section type)
			uint64_t address_alignment;     ///< The required alignment of the section in memory (normally 2^n)
			uint64_t entry_size;            ///< The size of each entry in the section if it contains a table of fixed-size entries
		} elf_64_section_header_t;

		/**
		 * @class ELF64
		 * @brief Handles the loading and parsing of 64-bit ELF files
		 */
		class ELF64 {
			private:
				uintptr_t m_elf_header_address;

				void load_program_headers() const;

			public:
				ELF64(uintptr_t elf_header_address);
				~ELF64();

				void load();
				bool is_valid() const;

				[[nodiscard]] elf_64_header_t* header() const;
				elf_64_program_header_t* get_program_header(size_t index) const;
				elf_64_section_header_t* get_section_header(size_t index) const;

				static uint64_t to_vmm_flags(uint32_t type);

		};

	}

}

#endif // MAXOS_PROCESSES_ELF_H
