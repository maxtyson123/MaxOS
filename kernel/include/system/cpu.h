/**
 * @file cpu.h
 * @brief Defines Central Processing Unit (CPU) structures and functions for managing CPU state and features
 *
 * @date 17th January 2024
 * @author Max Tyson
 */

#ifndef MAXOS_SYSTEM_CPU_H
#define MAXOS_SYSTEM_CPU_H

#include <cpuid.h>
#include <stddef.h>
#include <stdint.h>
#include <system/gdt.h>
#include <hardwarecommunication/acpi.h>
#include <hardwarecommunication/apic.h>
#include <memory/physical.h>

// Forward declare
namespace MaxOS {
	namespace processes {
		class Scheduler;
	}
}

namespace MaxOS {

	namespace system {

		/**
		 * @struct CPUStatus
		 * @brief Structure representing the CPU state during an interrupt
		 *
		 * @typedef cpu_status_t
		 * @brief Alias for CPUStatus struct
		 */
		typedef struct PACKED CPUStatus {

			uint64_t r15;   ///< Register r15
			uint64_t r14;   ///< Register r14
			uint64_t r13;   ///< Register r13
			uint64_t r12;   ///< Register r12
			uint64_t r11;   ///< Register r11
			uint64_t r10;   ///< Register r10
			uint64_t r9;    ///< Register r9
			uint64_t r8;    ///< Register r8
			uint64_t rdi;   ///< Register rdi (used for first argument)
			uint64_t rsi;   ///< Register rsi (used for second argument)
			uint64_t rbp;   ///< Register rbp (base pointer)
			uint64_t rdx;   ///< Register rdx (used for third argument)
			uint64_t rcx;   ///< Register rcx (used for fourth argument)
			uint64_t rbx;   ///< Register rbx (base register)
			uint64_t rax;   ///< Register rax (accumulator)

			uint64_t interrupt_number;  ///< The interrupt number
			uint64_t error_code;        ///< The error code (if applicable)

			uint64_t rip;       ///< Instruction pointer
			uint64_t cs;        ///< Code segment
			uint64_t rflags;    ///< Flags register
			uint64_t rsp;       ///< Stack pointer
			uint64_t ss;        ///< Stack segment

		} cpu_status_t;

		/**
		 * @struct TaskStateSegment
		 * @brief Structure representing the Task State Segment (TSS)
		 *
		 * @typedef tss_t
		 * @brief Alias for TaskStateSegment struct
		 */
		typedef struct PACKED TaskStateSegment {

			uint32_t reserved0;     ///< Unused, must be zero
			uint64_t rsp0;          ///< Stack pointer for ring 0
			uint64_t rsp1;          ///< Stack pointer for ring 1
			uint64_t rsp2;          ///< Stack pointer for ring 2
			uint64_t reserved1;     ///< Unused, must be zero
			uint64_t reserved2;     ///< Unused, must be zero
			uint64_t ist1;          ///< Interrupt Stack Table entry 1
			uint64_t ist2;          ///< Interrupt Stack Table entry 2
			uint64_t ist3;          ///< Interrupt Stack Table entry 3
			uint64_t ist4;          ///< Interrupt Stack Table entry 4
			uint64_t ist5;          ///< Interrupt Stack Table entry 5
			uint64_t ist6;          ///< Interrupt Stack Table entry 6
			uint64_t ist7;          ///< Interrupt Stack Table entry 7
			uint64_t reserved3;     ///< Unused, must be zero
			uint16_t reserved4;     ///< Unused, must be zero
			uint16_t io_bitmap_offset;  ///< Offset to the I/O bitmap

		} tss_t;

		/**
		 * @enum CPU_FEATURE_ECX
		 * @brief CPU features indicated by the ECX register after calling CPUID with EAX = 1
		 *
		 * @see https://wiki.osdev.org/CPUID as the original author
		 */
		enum class CPU_FEATURE_ECX : int32_t {
			SSE3        = 1 << 0,
			PCLMUL      = 1 << 1,
			DTES64      = 1 << 2,
			MONITOR     = 1 << 3,
			DS_CPL      = 1 << 4,
			VMX         = 1 << 5,
			SMX         = 1 << 6,
			EST         = 1 << 7,
			TM2         = 1 << 8,
			SSSE3       = 1 << 9,
			CID         = 1 << 10,
			SDBG        = 1 << 11,
			FMA         = 1 << 12,
			CX16        = 1 << 13,
			XTPR        = 1 << 14,
			PDCM        = 1 << 15,
			PCID        = 1 << 17,
			DCA         = 1 << 18,
			SSE4_1      = 1 << 19,
			SSE4_2      = 1 << 20,
			X2APIC      = 1 << 21,
			MOVBE       = 1 << 22,
			POPCNT      = 1 << 23,
			TSC         = 1 << 24,
			AES         = 1 << 25,
			XSAVE       = 1 << 26,
			OSXSAVE     = 1 << 27,
			AVX         = 1 << 28,
			F16C        = 1 << 29,
			RDRAND      = 1 << 30,
			HYPERVISOR  = 1 << 31,
		};

		/**
		 * @enum CPU_FEATURE_EDX
		 * @brief CPU features indicated by the EDX register after calling CPUID with EAX = 1
		 *
		 * @see https://wiki.osdev.org/CPUID as the original author
		 */
		enum class CPU_FEATURE_EDX : int32_t {
			FPU     = 1 << 0,
			VME     = 1 << 1,
			DE      = 1 << 2,
			PSE     = 1 << 3,
			TSC     = 1 << 4,
			MSR     = 1 << 5,
			PAE     = 1 << 6,
			MCE     = 1 << 7,
			CX8     = 1 << 8,
			APIC    = 1 << 9,
			SEP     = 1 << 11,
			MTRR    = 1 << 12,
			PGE     = 1 << 13,
			MCA     = 1 << 14,
			CMOV    = 1 << 15,
			PAT     = 1 << 16,
			PSE36   = 1 << 17,
			PSN     = 1 << 18,
			CLFLUSH = 1 << 19,
			DS      = 1 << 21,
			ACPI    = 1 << 22,
			MMX     = 1 << 23,
			FXSR    = 1 << 24,
			SSE     = 1 << 25,
			SSE2    = 1 << 26,
			SS      = 1 << 27,
			HTT     = 1 << 28,
			TM      = 1 << 29,
			IA64    = 1 << 30,
			PBE     = 1 << 31
		};

		/**
		 * @struct StackFrame
		 * @brief A snapshot of the a frame in the call stack
		 *
		 * @typedef stack_frame_t
		 * @brief Alias for StackFrame struct
		 */
		typedef struct PACKED StackFrame {

			StackFrame* next;   ///< Pointer to the next stack frame (up the call stack)
			uintptr_t rip;      ///< The instruction pointer at this frame

		} stack_frame_t;

		/**
		 * @struct CoreBootInfo
		 * @brief Information needed when booting a core
		 *
		 * @typedef core_boot_info_t
		 * @brief Alias for CoreBootInfo struct
		 */
		typedef struct PACKED CoreBootInfo {

			uint64_t stack;         ///< The stack pointer for the core
			uint64_t p4_table;      ///< The physical address of the P4 page table
			uint8_t id;             ///< The ID of the core
			bool activated;         ///< Whether the core has been activated
			void* gdt_64_base;      ///< The base of the 64-bit GDT

		} core_boot_info_t;

		/// The size of the stack allocated for booting a core (should align with the startup assembly code for the kernel)
		constexpr size_t BOOT_STACK_SIZE = 16384;

		class CPU;

		/**
		 * @class Core
		 * @brief Represents a CPU core in the system
		 */
		class Core {

			friend class CPU;

			protected:
				hardwarecommunication::madt_processor_apic_t* m_madt;   ///< The MADT entry for this core

				bool m_enabled = false;     ///< Whether the core is enabled
				bool m_can_enable = false;  ///< Whether the core can be enabled
				bool m_bsp = false;         ///< Whether this core is the bootstrap processor

				uint8_t m_apic_id;          ///< The ID of the apic for this core
				uint64_t m_stack;           ///< The stack pointer for this core

				void init_tss();
				void init_sse();

			public:
				explicit Core(hardwarecommunication::madt_processor_apic_t* madt_item);
				~Core();

				bool xsave_enabled = false; ///< Whether XSAVE is enabled
				bool avx_enabled = false;   ///< Whether AVX is enabled

				void wake_up(CPU* cpu);
				void init();

				uint8_t id;                 ///< The ID of this core
				tss_t tss = { };            ///< The Task State Segment for this core
				bool active = false;        ///< Whether this core is active

				hardwarecommunication::LocalAPIC* local_apic;   ///< The local APIC for this core
				GlobalDescriptorTable* gdt;                     ///< The GDT for this core
				processes::Scheduler* scheduler;                ///< The scheduler for this core
		};

		/**
		 * @class CPU
		 * @brief Manages the CPU and its cores
		 */
		class CPU {

			public:

				CPU(GlobalDescriptorTable* gdt, Multiboot* multiboot);
				~CPU();

				hardwarecommunication::AdvancedConfigurationAndPowerInterface acpi;     ///< The ACPI interface for the CPU
				hardwarecommunication::AdvancedProgrammableInterruptController apic;    ///< The APIC interface for the CPU

				static inline Core* panic_core = nullptr;   ///< The core that triggered the panic
				static inline common::Spinlock panic_lock;  ///< Lock to prevent multiple panics at once

				static cpu_status_t* prepare_for_panic(cpu_status_t* status = nullptr, const string &msg = "");
				static void PANIC(const char* message, cpu_status_t* status = nullptr);
				[[noreturn]] static void halt();

				inline static common::Vector<Core*> cores;  ///< The list of CPU cores in the system (populated during initialization, includes the BSP and cores that failed to start)
				void find_cores();
				void init_cores();
				static Core* executing_core();

				static bool check_nx();

				static void get_status(cpu_status_t* status);
				static void set_status(cpu_status_t* status);
				static void print_registers(cpu_status_t* status);

				static uint64_t read_msr(uint32_t msr);
				static void write_msr(uint32_t msr, uint64_t value);

				static void cpuid(uint32_t leaf, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx);
				static bool check_cpu_feature(CPU_FEATURE_ECX feature);
				static bool check_cpu_feature(CPU_FEATURE_EDX feature);

				static void stack_trace(size_t);
		};
	}
}


#endif // MAXOS_SYSTEM_CPU_H
