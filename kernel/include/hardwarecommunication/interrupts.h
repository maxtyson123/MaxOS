/**
 * @file interrupts.h
 * @brief Defines a InterruptManager and InterruptHandler for managing hardware and software interrupts
 *
 * @date 10th July 2022
 * @author Max Tyson
 */

#ifndef MAX_OS_HARDWARECOMMUNICATION_INTERRUPTS_H
#define MAX_OS_HARDWARECOMMUNICATION_INTERRUPTS_H

#include <cstdint>
#include <hardwarecommunication/port.h>
#include <system/gdt.h>
#include <inputStream.h>
#include <outputStream.h>
#include <map.h>
#include <system/cpu.h>
#include <hardwarecommunication/apic.h>


namespace MaxOS::hardwarecommunication {

	class InterruptManager;

	/**
	 * @class InterruptHandler
	 * @brief Handles a certain interrupt number
	 */
	class InterruptHandler {
		protected:
			uint8_t m_interrupt_number;     ///<  The interrupt number this handler handles

			explicit InterruptHandler(uint8_t interrupt_number, int64_t redirect = -1, uint64_t redirect_index = 0);
			~InterruptHandler();

		public:
			virtual void handle_interrupt();
			virtual system::cpu_status_t* handle_interrupt(system::cpu_status_t* status);

	};

	/**
	 * @struct IDTR
	 * @brief A struct that holds the Interrupt Descriptor Table Register (IDTR)
	 *
	 * @typedef idtr_t
	 * @brief Alias for IDTR struct
	 */
	typedef struct PACKED IDTR {
		uint16_t limit; ///< The size of the IDT
		uint64_t base;  ///< The start address of the IDT
	} idtr_t;

	/**
	 * @struct InterruptDescriptor
	 * @brief An entry in the Interrupt Descriptor Table (IDT)
	 *
	 * @typedef interrupt_descriptor_t
	 * @brief Alias for InterruptDescriptor struct
	 */
	typedef struct PACKED InterruptDescriptor {

		uint16_t address_low_bits;    ///< The low bits of the handler function address
		uint16_t segment_selector;    ///< The code segment selector in the GDT to use when this interrupt fires
		uint8_t ist;                  ///< The offset into the Interrupt Stack Table (0 if not used) (see TSS)
		uint8_t flags;                ///< Flags: The type of gate this entry represents (0xE for interrupt gate, 0xF for trap gate), The privilege level required to invoke this interrupt and whether the interrupt is present (@todo substruct this)
		uint16_t address_mid_bits;    ///< The middle bits of the handler function address
		uint32_t address_high_bits;   ///< The high bits of the handler function address
		uint32_t reserved;            ///< Reserved, must be zero

	} interrupt_descriptor_t;

	constexpr uint16_t HARDWARE_INTERRUPT_OFFSET = 0x20;    ///< The offset in the IDT where interrupts from hardware start
	constexpr uint16_t MAX_INTERRUPT_HANDLERS = 256;        ///< The maximum number of interrupt handlers

	/**
	 * @class InterruptManager
	 * @brief Handles all interrupts and passes them to the correct handler
	 *
	 * @todo Remove the max array of handler make vector, add static handlers for page faults and gpe. Make it so that multiple handlers can be registered for the same interrupt
	 * @todo Have a single stub so that we don't need to have 256 different functions for each interrupt
	 */
	class InterruptManager {
			friend class InterruptHandler;

		private:
			AdvancedProgrammableInterruptController* m_apic = nullptr;

			static system::cpu_status_t* page_fault(system::cpu_status_t* status);
			static system::cpu_status_t* general_protection_fault(system::cpu_status_t* status);

		protected:

			inline static InterruptManager* s_active_interrupt_manager = nullptr;                                   ///< The interrupt manger that is currently handling interrupts and is loaded into the IDTR

			InterruptHandler* m_interrupt_handlers[MAX_INTERRUPT_HANDLERS] = { };                                    ///< A map of index-to-interrupt of interrupt handlers to fire when an interrupt occurs
			inline static InterruptDescriptor s_interrupt_descriptor_table[MAX_INTERRUPT_HANDLERS] = { };            ///< The Interrupt Descriptor Table (IDT)

			static void set_interrupt_descriptor_table_entry(uint8_t interrupt, void (* handler)(), uint8_t descriptor_privilege_level);

			//Various Interrupts
			static void HandleInterruptRequest0x00();   ///< Stub (see interrupts.s)
			static void HandleInterruptRequest0x01();   ///< Stub (see interrupts.s)
			static void HandleInterruptRequest0x02();   ///< Stub (see interrupts.s)
			static void HandleInterruptRequest0x03();   ///< Stub (see interrupts.s)
			static void HandleInterruptRequest0x04();   ///< Stub (see interrupts.s)
			static void HandleInterruptRequest0x05();   ///< Stub (see interrupts.s)
			static void HandleInterruptRequest0x06();   ///< Stub (see interrupts.s)
			static void HandleInterruptRequest0x07();   ///< Stub (see interrupts.s)
			static void HandleInterruptRequest0x08();   ///< Stub (see interrupts.s)
			static void HandleInterruptRequest0x09();   ///< Stub (see interrupts.s)
			static void HandleInterruptRequest0x0A();   ///< Stub (see interrupts.s)
			static void HandleInterruptRequest0x0B();   ///< Stub (see interrupts.s)
			static void HandleInterruptRequest0x0C();   ///< Stub (see interrupts.s)
			static void HandleInterruptRequest0x0D();   ///< Stub (see interrupts.s)
			static void HandleInterruptRequest0x0E();   ///< Stub (see interrupts.s)
			static void HandleInterruptRequest0x80();   ///< Stub (see interrupts.s)
			static void HandleInterruptRequest0x0F();   ///< Stub (see interrupts.s)
			static void HandleInterruptRequest0x31();   ///< Stub (see interrupts.s)
			static void HandleInterruptRequest0x60();   ///< Stub (see interrupts.s)

			// Exceptions
			static void HandleException0x00();          ///< Stub (see interrupts.s)
			static void HandleException0x01();          ///< Stub (see interrupts.s)
			static void HandleException0x02();          ///< Stub (see interrupts.s)
			static void HandleException0x03();          ///< Stub (see interrupts.s)
			static void HandleException0x04();          ///< Stub (see interrupts.s)
			static void HandleException0x05();          ///< Stub (see interrupts.s)
			static void HandleException0x06();          ///< Stub (see interrupts.s)
			static void HandleException0x07();          ///< Stub (see interrupts.s)
			static void HandleInterruptError0x08();     ///< Stub (see interrupts.s)
			static void HandleException0x09();          ///< Stub (see interrupts.s)
			static void HandleInterruptError0x0A();     ///< Stub (see interrupts.s)
			static void HandleInterruptError0x0B();     ///< Stub (see interrupts.s)
			static void HandleInterruptError0x0C();     ///< Stub (see interrupts.s)
			static void HandleInterruptError0x0D();     ///< Stub (see interrupts.s)
			static void HandleInterruptError0x0E();     ///< Stub (see interrupts.s)
			static void HandleException0x0F();          ///< Stub (see interrupts.s)
			static void HandleException0x10();          ///< Stub (see interrupts.s)
			static void HandleInterruptError0x11();     ///< Stub (see interrupts.s)
			static void HandleException0x12();          ///< Stub (see interrupts.s)
			static void HandleException0x13();          ///< Stub (see interrupts.s)
			static void HandleException0x14();          ///< Stub (see interrupts.s)
			static void HandleException0x15();          ///< Stub (see interrupts.s)
			static void HandleException0x16();          ///< Stub (see interrupts.s)
			static void HandleException0x17();          ///< Stub (see interrupts.s)
			static void HandleException0x18();          ///< Stub (see interrupts.s)
			static void HandleException0x19();          ///< Stub (see interrupts.s)
			static void HandleException0x1A();          ///< Stub (see interrupts.s)
			static void HandleException0x1B();          ///< Stub (see interrupts.s)
			static void HandleException0x1C();          ///< Stub (see interrupts.s)
			static void HandleException0x1D();          ///< Stub (see interrupts.s)
			static void HandleException0x1E();          ///< Stub (see interrupts.s)
			static void HandleException0x1F();          ///< Stub (see interrupts.s)

			static system::cpu_status_t* HandleInterrupt(system::cpu_status_t* status);
			system::cpu_status_t* handle_interrupt_request(system::cpu_status_t* status);

		public:
			InterruptManager();
			~InterruptManager();

			static InterruptManager* active_interrupt_manager();

			void set_interrupt_handler(uint8_t interrupt, InterruptHandler* handler);
			void remove_interrupt_handler(uint8_t interrupt);

			void set_apic(AdvancedProgrammableInterruptController* apic);
			AdvancedProgrammableInterruptController* active_apic();

			static void load_current();
			void activate();
			static void deactivate();

			/**
			 * @brief Force the CPU to return from an interrupt (see interrupts.s)
			 * @param state the state to return to
			 */
			static void ForceInterruptReturn(system::cpu_status_t* state);

	};
}


#endif //MAX_OS_HARDWARECOMMUNICATION_INTERRUPTS_H
