/**
 * @file apic.h
 * @brief Defines classes and structures for handling the Advanced Programmable Interrupt Controller (APIC) in a multiprocessor system
 *
 * @date 18th January 2024
 * @author Max Tyson
 */

#ifndef MAXOS_HARDWARECOMMUNICATION_APIC_H
#define MAXOS_HARDWARECOMMUNICATION_APIC_H

#include <common/macros.h>
#include <hardwarecommunication/port.h>
#include <hardwarecommunication/acpi.h>
#include <memory/physical.h>

namespace MaxOS {
	namespace hardwarecommunication {

		/**
		 * @class LocalAPIC
		 * @brief Handles the local APIC for the current core
		 */
		class LocalAPIC {

			private:
				uint64_t m_apic_base{};
				uint64_t m_apic_base_high{};
				uint32_t m_id{};
				bool m_x2apic{};

			public:
				LocalAPIC();
				~LocalAPIC();

				[[nodiscard]] uint32_t read(uint32_t reg) const;
				void write(uint32_t reg, uint32_t value) const;

				[[nodiscard]] uint32_t id() const;
				void send_eoi() const;

				void send_init(uint8_t apic_id, bool assert);
				void send_startup(uint8_t apic_id, uint8_t vector);

		};

		/**
		 * @struct MADT
		 * @brief Multiple APIC Description Table (MADT) (ACPI 2 or higher). Provides information about the APICs in
		 * the system. Following this header are multiple entries describing the APICs (see MADTEntry).
		 *
		 * @typedef madt_t
		 * @brief Alias for MADT struct
		 */
		typedef struct PACKED MADT {
			ACPISDTHeader header;               ///< Common ACPI SDT header
			uint32_t local_apic_address;        ///< The physical address of the local APIC
			uint32_t flags;                     ///< Flags indicating the capabilities of the system (1 = Legacy PICs are present)
		} madt_t;

		/**
		 * @struct MADTEntry
		 * @brief An item in the MADT table
		 *
		 * @typedef madt_entry_t
		 * @brief Alias for MADTEntry struct
		 */
		typedef struct PACKED MADTEntry {
			uint8_t type;       ///< The type of the MADT entry (see MADT_TYPE)
			uint8_t length;     ///< How many bytes this entry takes up (including the type and length fields)
		} madt_entry_t;

		/**
		 * @enum MADT_TYPE
		 * @brief The types of MADT entries
		 */
		enum class MADT_TYPE {
			PROCESSOR_APIC,
			IO_APIC,
			INTERRUPT_SOURCE_OVERRIDE,
			NMI_SOURCE,
			APIC_NMIS,
			APIC_ADDRESS_OVERRIDE,
			PROCESSOR_X2APIC,
		};

		/**
		 * @struct MADT_IO_APIC
		 * @brief An entry in the MADT table describing an IO APIC
		 *
		 * @typedef madt_io_apic_t
		 * @brief Alias for MADT_IO_APIC struct
		 *
		 * @todo Rename for consistency
		 */
		typedef struct PACKED MADT_IO_APIC {
			uint8_t io_apic_id;                         ///< The ID of the IO APIC
			uint8_t reserved;                           ///< Reserved
			uint32_t io_apic_address;                   ///< The physical address of the IO APIC
			uint32_t global_system_interrupt_base;      ///< The first GSI number that this IO APIC handles
		} madt_io_apic_t;

		/**
		 * @struct MADT_PROCESSOR_APIC
		 * @brief An entry in the MADT table describing a processor local APIC
		 *
		 * @typedef madt_processor_apic_t
		 * @brief Alias for MADT_PROCESSOR_APIC struct
		 */
		typedef struct PACKED MADT_PROCESSOR_APIC {
			uint8_t processor_id;   ///< The ID for the core
			uint8_t apic_id;        ///< The ID for the APIC that the core uses
			uint32_t flags;         ///< Flags indicating the status of the core (bit 0 = core enabled, bit 1 = can be enabled)
		} madt_processor_apic_t;

		/**
		 * @union RedirectionEntry
		 * @brief An IO APIC redirection entry. Describes how an interrupt is routed.
		 *
		 * @typedef redirection_entry_t
		 * @brief Alias for RedirectionEntry union
		 */
		typedef union RedirectionEntry {
			struct PACKED {
				uint64_t vector: 8;             ///< The interrupt that will be triggered when this redirection entry is used
				uint64_t delivery_mode: 3;      ///< How the interrupt will be delvied to the core(s) (see DeliveryMode)
				uint64_t destination_mode: 1;   ///< How to interpret the destination field (0 = means its the core ID, 1 = means a bitmask of cores)
				uint64_t delivery_status: 1;    ///< Indicates whether, when setting this entry, the interrupt is still being sent
				uint64_t pin_polarity: 1;       ///< Defines the electrical signal that indicates an active interrupt line (0 = high active, 1 = low active) (must match the MADT override flags)
				uint64_t remote_irr: 1;         ///< ?
				uint64_t trigger_mode: 1;       ///< How the electrical signal is interpreted (0 = edge triggered, 1 = level triggered) (must match the MADT override flags)
				uint64_t mask: 1;               ///< Whether the interrupt is disabled or not
				uint64_t reserved: 39;          ///< Reserved bits (should be zero)
				uint64_t destination: 8;        ///< The core(s) the interrupt should be sent to (interpreted based on destination_mode)
			};          ///< Packed because the bitfields should be one uint64_t together

			uint64_t raw;   					///< The raw 64-bit value of the redirection entry
		} redirection_entry_t;

		/**
		 * @enum DeliveryMode
		 * @brief How the interrupt should be delivered (can it override others etc). Set fixed/lowest priority for normal interrupts.
		 *
		 * @todo use this
		 */
		enum DeliveryMode {
			FIXED           = 0,
			LOWEST_PRIORITY = 1,
			SMI             = 2,
			NMI             = 4,
			INIT            = 5,
			EXTINT          = 7,
		};

		/**
		 * @struct InterruptRedirect
		 * @brief Defines how an interrupt input is redirected to a specific processor via the I/O APIC
		 *
		 * @typedef interrupt_redirect_t
		 * @brief Alias for InterruptRedirect struct
		 */
		typedef struct InterruptRedirect {

			uint8_t type;           ///< The type of redirect (should be 0 for normal interrupts)
			uint8_t index;          ///< Where in the IO APIC redirection table this redirect should be placed
			uint8_t interrupt;      ///< The interrupt number to trigger when this redirect is used
			uint8_t destination;    ///< The ID of the core the interrupt should be sent to
			uint32_t flags;         ///< Configuration flags for the interrupt (see MADT override flags)
			bool mask;              ///< Should the interrupt be disabled initially

		} interrupt_redirect_t;

		/**
		 * @struct Override
		 * @brief Specifies how a specific interrupt source is mapped to a global system interrupt
		 *
		 * @typedef override_t
		 * @brief Alias for Override struct
		 */
		typedef struct PACKED Override {

			uint8_t bus;                        ///< Identifies the hardware bus the interrupt comes from (0 = ISA, 1 = PCI)
			uint8_t source;                     ///< The interrupt source number on the specified bus
			uint32_t global_system_interrupt;   ///< The global system interrupt that the source is mapped to
			uint16_t flags;                     ///< Flags describing the polarity and trigger mode of the interrupt source (see MADT override flags)

		} override_t;

		/**
		 * @class IOAPIC
		 * @brief Handles the IO APIC in the system (one per system)
		 */
		class IOAPIC {
			private:
				AdvancedConfigurationAndPowerInterface* m_acpi;
				MADT* m_madt = {nullptr};
				uint64_t m_address = 0;
				uint64_t m_address_high = 0;
				uint32_t m_version = 0;
				uint8_t m_max_redirect_entry = 0;

				uint8_t m_override_array_size = 0;
				Override m_override_array[0x10] = {};

				void read_redirect(uint8_t index, RedirectionEntry* entry);
				void write_redirect(uint8_t index, RedirectionEntry* entry);

			public:
				IOAPIC(AdvancedConfigurationAndPowerInterface* acpi);
				~IOAPIC();

				uint32_t read(uint32_t reg) const;
				void write(uint32_t reg, uint32_t value) const;

				void set_redirect(interrupt_redirect_t* redirect);
				void set_redirect_mask(uint8_t index, bool mask);

				MADTEntry* get_madt_item(MADT_TYPE type, uint8_t index);

		};

		/**
		 * @class AdvancedProgrammableInterruptController
		 * @brief Handles both the Local APIC (boot core) and IO APIC
		 *
		 * @todo Should either handle all Local APICs or none (only boot core right now, others accessed per core directly which is inconsistent)
		 */
		class AdvancedProgrammableInterruptController {

			private:
				LocalAPIC* m_local_apic;
				IOAPIC* m_io_apic;

				Port8BitSlow m_pic_master_command_port;
				Port8BitSlow m_pic_master_data_port;
				Port8BitSlow m_pic_slave_command_port;
				Port8BitSlow m_pic_slave_data_port;

				void disable_pic();

			public:
				AdvancedProgrammableInterruptController(AdvancedConfigurationAndPowerInterface* acpi);
				~AdvancedProgrammableInterruptController();

				LocalAPIC* local_apic() const;
				IOAPIC* io_apic() const;

		};

	}
}

#endif // MAXOS_HARDWARECOMMUNICATION_APIC_H
