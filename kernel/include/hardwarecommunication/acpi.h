/**
 * @file acpi.h
 * @brief Defines an Advanced Configuration And Power Interface (ACPI class for handling ACPI table parsing and retrieval
 *
 * @date 18th January 2024
 * @author Max Tyson
 */

#ifndef MAXOS_HARDWARECOMMUNICATION_ACPI_H
#define MAXOS_HARDWARECOMMUNICATION_ACPI_H

#include <stdint.h>
#include <stddef.h>
#include <system/multiboot.h>
#include <common/string.h>
#include <memory/memorymanagement.h>
#include <memory/memoryIO.h>
#include <memory/physical.h>

namespace MaxOS {
	namespace hardwarecommunication {

		/**
		 * @struct RSDPDescriptor
		 * @brief Root System Description Pointer (RSDP) structure for ACPI 1.
		 *
		 * @typedef rsdp_descriptor_t
		 * @brief Alias for RSDPDescriptor struct
		 */
		typedef struct PACKED RSDPDescriptor {
			char signature[8];              ///< Indicates a valid RSDP structure if it contains "RSD PTR " (null-terminated)
			uint8_t checksum;               ///< This byte added to all bytes in the table must equal zero for the table to be valid
			char OEMID[6];                  ///< A string supplied by the OEM to identify the OEM
			uint8_t revision;               ///< The version of the ACPI specification that is supported (higher means more features and is backwards compatible)
			uint32_t rsdt_address;          ///< The physical address of the RSDT (@deprecated in ACPI 2 and above)
		} rsdp_descriptor_t;

		/**
		 * @struct RSDPDescriptor2
		 * @brief Root System Description Pointer (RSDP) structure for ACPI 2 and above
		 *
		 * @typedef rsdp_descriptor2_t
		 * @brief Alias for RSDPDescriptor2 struct
		 */
		typedef struct PACKED RSDPDescriptor2 {
			RSDPDescriptor version_1_info;      ///< The ACPI 1 RSDP structure (see rsdp_descriptor_t)
			uint32_t length;                    ///< The total length of the RSDP structure (including the version 1 part)
			uint64_t xsdt_address;              ///< The physical address of the XSDT
			uint8_t extended_checksum;          ///< This byte added to all bytes in the table must equal zero for the table to be valid (including the version 1 part)
			uint8_t reserved[3];                ///< Reserved, must be zero
		} rsdp_descriptor2_t;

		/**
		 * @struct ACPISDTHeader
		 * @brief Common header for all ACPI System Description Tables (SDTs)
		 *
		 * @typedef acpi_sdt_header_t
		 * @brief Alias for ACPISDTHeader struct
		 */
		typedef struct PACKED ACPISDTHeader {
			char signature[4];              ///< The signature that identifies the type of the table
			uint32_t length;                ///< The size of the entire table, including the header, in bytes
			uint8_t revision;               ///< The version of the ACPI specification that is supported (higher means more features and is backwards compatible)
			uint8_t checksum;               ///< This byte added to all bytes in the table must equal zero mod 0x100 for the table to be valid (including the header)
			char OEM_id[6];                 ///< A string supplied by the OEM to identify the OEM
			char OEM_table_id[8];           ///< A string supplied by the OEM to identify the particular table
			uint32_t OEM_revision;          ///< The revision number of the table as supplied by the OEM
			uint32_t creator_id;            ///< A vendor ID of the utility that created the table
			uint32_t creator_revision;      ///< The revision number of the utility that created the table
		} acpi_sdt_header_t;

		/**
		 * @struct RSDT
		 * @brief Root System Description Table (RSDT) structure for ACPI 1. Contains the header and an array of pointers to other SDTs
		 *
		 * @typedef rsdt_t
		 * @brief Alias for RSDT struct
		 */
		typedef struct RSDT {
			ACPISDTHeader header;   ///< The common header for all ACPI SDTs (see acpi_sdt_header_t)
			uint32_t pointers[];    ///< An array of physical addresses pointing to other ACPI SDTs
		} rsdt_t;

		/**
		 * @struct XSDT
		 * @brief Extended System Description Table (XSDT) structure for ACPI 2 and above. Contains the header and an array of pointers to other SDTs
		 *
		 * @typedef xsdt_t
		 * @brief Alias for XSDT struct
		 */
		typedef struct XSDT {
			ACPISDTHeader header;   ///< The common header for all ACPI SDTs (see acpi_sdt_header_t)
			uint64_t pointers[];    ///< An array of physical addresses pointing to other ACPI SDTs
		} xsdt_t;

		/**
		 * @class AdvancedConfigurationAndPowerInterface
		 * @brief Handles ACPI table parsing and retrieval
		 */
		class AdvancedConfigurationAndPowerInterface {
			private:
				bool m_using_new_acpi = false;
				acpi_sdt_header_t* m_header;

				xsdt_t* m_xsdt;
				rsdt_t* m_rsdt;

				rsdp_descriptor_t * m_rsdp;
				rsdp_descriptor2_t * m_rsdp2;

				static bool validate(const char* descriptor, size_t length);

				void map_tables(uint8_t size_of_table);
				memory::virtual_address_t* map_descriptor(uint64_t physical);

				uint64_t get_rsdt_pointer(size_t index);

				bool valid_checksum();

			public:
				AdvancedConfigurationAndPowerInterface(system::Multiboot* multiboot);
				~AdvancedConfigurationAndPowerInterface();

				acpi_sdt_header_t* find(const char* signature);
		};

	}
}

#endif // MAXOS_HARDWARECOMMUNICATION_ACPI_H
