/**
 * @file kernel.cpp
 * @brief The main kernel entry point for the bsp and other cores. Handles initialisation of all core systems.
 *
 * @date 2022
 * @author Max Tyson
 */

#include <cstdint>
#include <common/logger.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/clock.h>
#include <drivers/console/serial.h>
#include <drivers/console/vesaboot.h>
#include <gui/desktop.h>
#include <processes/scheduler.h>
#include <system/cpu.h>
#include <system/syscalls.h>
#include <memory/memorymanagement.h>
#include <memory/physical.h>
#include <memory/virtual.h>
#include <filesystem/vfs.h>
#include <filesystem/vfsresource.h>
#include <tests/test.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::console;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::gui;
using namespace MaxOS::processes;
using namespace MaxOS::system;
using namespace MaxOS::memory;
using namespace MaxOS::filesystem;
using namespace MaxOS::tests;

extern "C" void call_constructors();        ///< Calls the C++ static constructors
extern "C" uint8_t core_boot_info[];        ///< The boot info structure for the core being started

/**
 * @brief The main entry point for secondary cores. Sets up the core and waits to be scheduled.
 */
extern "C" [[noreturn]] void core_main() {

	auto info = (core_boot_info_t*) (core_boot_info);
	info->activated = true;
	auto core = CPU::executing_core();

	// Make sure the correct core is being setup
	ASSERT(info->id == core->id, "Current setup core isn't the core expected");
	Logger::DEBUG() << "Core " << core->id << " now in higher half \n";

	// Set up the core
	core->init();
	asm("sti");

	// Wait to be scheduled
	while(true)
		asm("nop");
}

/**
 * @brief The main kernel entry point. Initialises all core systems and starts the scheduler.
 *
 * @param addr The address of the multiboot info struct
 * @param magic The multiboot magic number
 */
extern "C" [[noreturn]] void kernel_main(unsigned long addr, unsigned long magic) {

	call_constructors();

	Logger logger;
	SerialConsole serial_console(&logger);
	Logger::INFO() << "MaxOS Booted Successfully\n";

	Logger::HEADER() << "Stage {1}: System Initialisation\n";
	Multiboot multiboot(addr, magic);
	GlobalDescriptorTable gdt;
	InterruptManager interrupts;

	Logger::HEADER() << "Stage {1.1}: Memory Initialisation\n";
	PhysicalMemoryManager pmm(&multiboot);
	VirtualMemoryManager vmm;
	MemoryManager memory_manager(&vmm);

	Logger::HEADER() << "Stage {1.2}: Console Initialisation\n";
	VideoElectronicsStandardsAssociation vesa(multiboot.framebuffer());
	VESABootConsole console(&vesa);

	Logger::HEADER() << "Stage {2}: Hardware Initialisation\n";
	CPU cpu(&gdt, &multiboot);
	Clock kernel_clock(&cpu.apic, 1);
	interrupts.activate();
	kernel_clock.calibrate();
	cpu.init_cores();

	Logger::HEADER() << "Stage {3}: Userspace Initialisation\n";
	SyscallManager syscalls;
	GlobalScheduler scheduler(multiboot);
	console.finish();
	GlobalScheduler::activate();

	//TO MOVE:
	//	VirtualFileSystem vfs;
	//	VFSResourceRegistry vfs_registry(&vfs);

	// Idle loop  (read Idle.md)
	while(true)
		asm("hlt");

}

/**
 * @todo Thread storage (when clib) & threads can use RPC
 * @todo Once kernel done, turn into mono repo and separate components
 * @todo Doxy for progs & libs
 */