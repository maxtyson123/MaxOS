//
// Created by 98max on 21/03/2025.
//

#ifndef MAXOS_PROCESSES_ELF_H
#define MAXOS_PROCESSES_ELF_H

#include <stdint.h>
#include <stddef.h>

#include <memory/memorymanagement.h>
#include <memory/memoryIO.h>

namespace MaxOS
{
  namespace processes {

      /// Style Guide Note: ELF is consider as one of the more standard names so use that instead of Executable and Linkable Format

      enum class ElfIdentification{
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

      enum class ElfClass{
          Invalid = 0,
          Bits32 = 1,
          Bits64 = 2
      };

      enum class ElfData{
          InvalidData,
          LittleEndian,
          BigEndian
      };

      enum class ElfType{
          None,
          Relocatable,
          Executable,
          Shared,
          Core,
          ProcessorSpecificLow  = 0xFF0,
          ProcessorSpecificHigh = 0xFFF
      };

      enum class ElfMachine{
          NoMachine,
          ATnTWe32100,
          SPARC,
          x86,
          MIPS      = 0x8,
          PowerPC   = 0x14,
          ARM       = 0x28,
          SuperH    = 0x2A,
          IA_64     = 0x32,
          x86_64    = 0x3E,
          AArch64   = 0xB7,
          RISC_V    = 0xF3
      };

      enum class ElfVersion{
          Invalid,
          Current
      };

      typedef struct {
        unsigned char identification[16];
        uint16_t      type;
        uint16_t      machine;
        uint32_t      version;
        uint64_t      entry;
        uint64_t      program_header_offset;
        uint64_t      section_header_offset;
        uint32_t      flags;
        uint16_t      elf_header_size;
        uint16_t      program_header_size;
        uint16_t      program_header_count;
        uint16_t      section_header_size;
        uint16_t      section_header_count;
        uint16_t      section_header_string_table_index;
      } elf_64_header_t;

      ///      Program Header

      enum class ElfProgramType{
          Null,
          Load,
          Dynamic,
          Interpreter,
          Note,
          SharedLibrary,
          HeaderTable,
          TableProcessorSpecificLow  = 0x70000000,
          TableProcessorSpecificHigh = 0x7FFFFFFF
      };

      enum ElfProgramFlags{
          ElfExecute = (1 << 0),
          ElfWrite   = (1 << 1),
          ElfRead    = (1 << 2)
      };

      typedef struct {
        uint32_t     type;
        uint32_t     flags;
        uint64_t     offset;
        uint64_t     virtual_address;
        uint64_t     physical_address;
        uint64_t     file_size;
        uint64_t     memory_size;
        uint64_t     align;
      } elf_64_program_header_t;

      ///      Section Header

      enum class ElfSectionType{
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

      typedef struct {
        uint32_t   name_index;
        uint32_t   type;
        uint64_t   sh_flags;
        uint64_t   virtual_address;
        uint64_t   offset;
        uint64_t   size;
        uint32_t   link;
        uint32_t   info;
        uint64_t   address_alignment;
        uint64_t   entry_size;
      } elf_64_section_header_t;

      /**
       * @class Elf64
       * @brief Handles the loading and parsing of 64-bit ELF files
       */
      class Elf64
      {
        private:
          uintptr_t m_elf_header_address;

          static char elf_magic[4];

          void load_program_headers();


        public:
            Elf64(uintptr_t elf_header_address);
            ~Elf64();

            void load();
            bool is_valid();

            [[nodiscard]] elf_64_header_t* header() const;
            elf_64_program_header_t* get_program_header(size_t index);
            elf_64_section_header_t* get_section_header(size_t index);

            static uint64_t to_vmm_flags(uint32_t type);

      };

  }

}

#endif // MAXOS_PROCESSES_ELF_H
