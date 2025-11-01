/**
 * @file kernel.cpp
 * @brief The main kernel entry point for the bsp and other cores. Handles initialisation of all core systems.
 *
 * @date 2022
 * @author Max Tyson
 */

#include <stdint.h>
#include <common/logger.h>
#include <hardwarecommunication/interrupts.h>
#include <drivers/console/serial.h>
#include <drivers/console/vesaboot.h>
#include <drivers/driver.h>
#include <drivers/video/vesa.h>
#include <gui/desktop.h>
#include <processes/scheduler.h>
#include <system/cpu.h>
#include <system/syscalls.h>
#include <memory/memorymanagement.h>
#include <memory/physical.h>
#include <memory/virtual.h>
#include <filesystem/vfs.h>
#include <filesystem/vfsresource.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::peripherals;
using namespace MaxOS::drivers::video;
using namespace MaxOS::drivers::clock;
using namespace MaxOS::drivers::console;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::gui;
using namespace MaxOS::processes;
using namespace MaxOS::system;
using namespace MaxOS::memory;
using namespace MaxOS::filesystem;

extern "C" void call_constructors();        ///< Calls the C++ static constructors
extern "C" uint8_t core_boot_info[];        ///< The boot info structure for the core being started

/**
 * @brief The main entry point for secondary cores. Sets up the core and waits to be scheduled.
 */
extern "C" [[noreturn]] void core_main(){

	auto info = (core_boot_info_t*)(core_boot_info);
	info -> activated = true;
	auto core = CPU::executing_core();

	// Make sure the correct core is being setup
	ASSERT(info->id == core->id, "Current setup core isn't the core expected");
	Logger::DEBUG() << "Core " << core->id << " now in higher half \n";

	// Set up the core
	core -> init();
	asm("sti");

	// Wait to be scheduled
	while (true)
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

	// Initialise the logger
	Logger logger;
	SerialConsole serial_console(&logger);
	Logger::INFO() << "MaxOS Booted Successfully 0x\n";

	Logger::HEADER() << "Stage {1}: System Initialisation\n";
	Multiboot multiboot(addr, magic);
	GlobalDescriptorTable gdt;
	InterruptManager interrupts;

	Logger::HEADER() << "Stage {1.1}: Memory Initialisation\n";
	PhysicalMemoryManager pmm(&multiboot);
	VirtualMemoryManager vmm;
	MemoryManager memoryManager(&vmm);

	Logger::HEADER() << "Stage {1.2}: Console Initialisation\n";
	VideoElectronicsStandardsAssociation vesa(multiboot.framebuffer());
	VESABootConsole console(&vesa);

	Logger::HEADER() << "Stage {2}: Hardware Initialisation\n";
	VirtualFileSystem vfs;
	CPU cpu(&gdt, &multiboot);
	Clock kernel_clock(&cpu.apic, 1);
	DriverManager driver_manager;
	driver_manager.add_driver(&kernel_clock);
	driver_manager.find_drivers();
	uint32_t reset_wait_time = driver_manager.reset_devices();

	Logger::HEADER() << "Stage {3}: Device Finalisation\n";
	interrupts.activate();
	kernel_clock.calibrate();
	kernel_clock.delay(reset_wait_time);
	driver_manager.initialise_drivers();
	driver_manager.activate_drivers();
	cpu.init_cores();

	Logger::HEADER() << "Stage {4}: System Finalisation\n";
	GlobalScheduler scheduler(multiboot);
	VFSResourceRegistry vfs_registry(&vfs);
	SyscallManager syscalls;
	console.finish();
	GlobalScheduler::activate();

	// Idle loop  (read Idle.md)
	while (true)
		asm("hlt");

}

/**
 * @todo Clean up warnings
 * @todo Test suite of common functions & other statics (paths)
 * @todo Thread storage (when clib)
 */