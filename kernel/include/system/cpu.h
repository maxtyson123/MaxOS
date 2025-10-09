//
// Created by 98max on 17/01/2024.
//

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

		/// Style Guide Note: CPU abbreviation is preferred over Central Processing Unit

		typedef struct {
			uint64_t r15;
			uint64_t r14;
			uint64_t r13;
			uint64_t r12;
			uint64_t r11;
			uint64_t r10;
			uint64_t r9;
			uint64_t r8;
			uint64_t rdi;
			uint64_t rsi;
			uint64_t rbp;
			uint64_t rdx;
			uint64_t rcx;
			uint64_t rbx;
			uint64_t rax;

			uint64_t interrupt_number;
			uint64_t error_code;

			uint64_t rip;
			uint64_t cs;
			uint64_t rflags;
			uint64_t rsp;
			uint64_t ss;
		} __attribute__((__packed__)) cpu_status_t;

		typedef struct tss {
			uint32_t reserved0;
			uint64_t rsp0;
			uint64_t rsp1;
			uint64_t rsp2;
			uint64_t reserved1;
			uint64_t reserved2;
			uint64_t ist1;
			uint64_t ist2;
			uint64_t ist3;
			uint64_t ist4;
			uint64_t ist5;
			uint64_t ist6;
			uint64_t ist7;
			uint64_t reserved3;
			uint16_t reserved4;
			uint16_t io_bitmap_offset;
		}__attribute__((__packed__)) tss_t;

		// Osdev Wiki: https://wiki.osdev.org/CPUID
		enum class CPU_FEATURE_ECX
				: int32_t {
			SSE3 = 1 << 0,
			PCLMUL = 1 << 1,
			DTES64 = 1 << 2,
			MONITOR = 1 << 3,
			DS_CPL = 1 << 4,
			VMX = 1 << 5,
			SMX = 1 << 6,
			EST = 1 << 7,
			TM2 = 1 << 8,
			SSSE3 = 1 << 9,
			CID = 1 << 10,
			SDBG = 1 << 11,
			FMA = 1 << 12,
			CX16 = 1 << 13,
			XTPR = 1 << 14,
			PDCM = 1 << 15,
			PCID = 1 << 17,
			DCA = 1 << 18,
			SSE4_1 = 1 << 19,
			SSE4_2 = 1 << 20,
			X2APIC = 1 << 21,
			MOVBE = 1 << 22,
			POPCNT = 1 << 23,
			TSC = 1 << 24,
			AES = 1 << 25,
			XSAVE = 1 << 26,
			OSXSAVE = 1 << 27,
			AVX = 1 << 28,
			F16C = 1 << 29,
			RDRAND = 1 << 30,
			HYPERVISOR = 1 << 31,
		};

		enum class CPU_FEATURE_EDX
				: int32_t {
			FPU = 1 << 0,
			VME = 1 << 1,
			DE = 1 << 2,
			PSE = 1 << 3,
			TSC = 1 << 4,
			MSR = 1 << 5,
			PAE = 1 << 6,
			MCE = 1 << 7,
			CX8 = 1 << 8,
			APIC = 1 << 9,
			SEP = 1 << 11,
			MTRR = 1 << 12,
			PGE = 1 << 13,
			MCA = 1 << 14,
			CMOV = 1 << 15,
			PAT = 1 << 16,
			PSE36 = 1 << 17,
			PSN = 1 << 18,
			CLFLUSH = 1 << 19,
			DS = 1 << 21,
			ACPI = 1 << 22,
			MMX = 1 << 23,
			FXSR = 1 << 24,
			SSE = 1 << 25,
			SSE2 = 1 << 26,
			SS = 1 << 27,
			HTT = 1 << 28,
			TM = 1 << 29,
			IA64 = 1 << 30,
			PBE = 1 << 31
		};


		typedef struct StackFrame {
			StackFrame* next;
			uintptr_t rip;
		}__attribute__((__packed__)) stack_frame_t;

		typedef struct CoreBootInfo{
			uint64_t stack;
			uint64_t p4_table;
			uint8_t  id;
			bool     activated;
			void*    gdt_64_base;
		}__attribute__((__packed__)) core_boot_info_t;

		class CPU;
		class Core{

			friend class CPU;
			protected:
				hardwarecommunication::madt_processor_apic_t* m_madt;

				bool m_enabled = false;
				bool m_can_enable = false;
				bool m_bsp = false;

				uint8_t m_apic_id;

				static inline size_t s_stack_size = 16384;
				uint64_t m_stack;

			    void init_tss();
				void init_sse();

		public:
				explicit Core(hardwarecommunication::madt_processor_apic_t* madt_item);
				~Core();

				bool xsave_enabled = false;
				bool avx_enabled = false;

				void wake_up(CPU* cpu);
				void init();

				uint8_t id;
				tss_t tss = {};
				bool active = false;

				hardwarecommunication::LocalAPIC* local_apic;
				GlobalDescriptorTable* gdt;
				processes::Scheduler* scheduler;
		};


		class CPU {

			public:

				CPU(GlobalDescriptorTable* gdt, Multiboot* multiboot);
				~CPU();

				hardwarecommunication::AdvancedConfigurationAndPowerInterface acpi;
				hardwarecommunication::AdvancedProgrammableInterruptController apic;

				static inline Core* panic_core = nullptr;
				static inline common::Spinlock panic_lock;

				static cpu_status_t* prepare_for_panic(cpu_status_t* status = nullptr, const string& msg = "");
				static void PANIC(const char* message, cpu_status_t* status = nullptr);
				[[noreturn]] static void halt();

				inline static common::Vector<Core*> cores;
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
