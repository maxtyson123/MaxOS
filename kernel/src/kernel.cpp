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
#include <console/serial.h>
#include <console/framebuffer.h>
#include <processes/scheduler.h>
#include <system/cpu.h>
#include <system/syscalls.h>
#include <memory/memorymanagement.h>
#include <memory/physical.h>
#include <memory/virtual.h>
#include <tests/test.h>

#include "runtime/gdbstub.h"

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::console;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::processes;
using namespace MaxOS::system;
using namespace MaxOS::memory;
using namespace MaxOS::tests;
using namespace MaxOS::runtime;

extern "C" void call_constructors(); ///< Calls the C++ static constructors
extern "C" uint8_t core_boot_info[]; ///< The boot info structure for the core being started

/**
 * @brief The main entry point for secondary cores. Sets up the core and waits to be scheduled.
 */
extern "C" [[noreturn]] void core_main()
{
	auto info = (core_boot_info_t*)(core_boot_info);
	info->activated = true;
	auto core = CPU::executing_core();

	// Make sure the correct core is being setup
	ASSERT(info->id == core->id, "Current setup core isn't the core expected");
	Logger::DEBUG() << "Core " << core->id << " now in higher half \n";

	// Set up the core
	core->init();
	asm("sti");

	// Wait to be scheduled
	while (true)
		CPU::halt();
}

/**
 * @brief The main kernel entry point. Initialises all core systems and starts the scheduler.
 *
 * @param addr The address of the multiboot info struct
 * @param magic The multiboot magic number
 */
extern "C" [[noreturn]] void kernel_main(unsigned long addr, unsigned long magic)
{
	call_constructors();

	Logger logger;
	SerialConsole serial_console(&logger);
	Logger::INFO() << "MaxOS Booted Successfully\n";

	Logger::HEADER() << "Stage {1}: System Initialisation\n";
	Multiboot multiboot(addr, magic);
	GlobalDescriptorTable gdt;
	InterruptManager interrupts;
	ExceptionHandlers exceptions;

	Logger::HEADER() << "Stage {1.1}: Memory Initialisation\n";
	PhysicalMemoryManager pmm(&multiboot);
	VirtualMemoryManager vmm;
	MemoryManager memory_manager(&vmm);

	Logger::HEADER() << "Stage {1.2}: Console Initialisation\n";
	FramebufferConsole console(multiboot.framebuffer());

	Logger::HEADER() << "Stage {2}: Hardware Initialisation\n";
	CPU cpu(&gdt, &multiboot);
	Clock kernel_clock(&cpu.apic, 1);
	interrupts.activate();
	kernel_clock.calibrate();
	cpu.init_cores();

	Logger::HEADER() << "Stage {3}: Userspace Initialisation\n";
	SyscallManager syscalls;
	GlobalScheduler scheduler(multiboot);
	// console.finish();
	GlobalScheduler::activate();

	// Idle loop  (read Idle.md)
	while (true)
		asm("hlt");
}

/**
 * @todo less copy on program creation
 * @todo kernel flamegraph
 * @todo better erroring with bridge calls
 * @todo resource attr instead of flag fuckery
 *
 * Longterm:
 * @todo Thread storage (when clib) & threads can use RPC
 * @todo LibC
 * @todo Once kernel done, turn into mono repo and separate components
 * @todo Doxy for progs & libs
 */

// Finish LibDriver
//
// init flow:
// % parse multiboot for init program, fileserver and ramdisk
// % schedule init program and fileserver
// % init program tells fileserver to load & parse ramdisk
// % init (via fileserver) reads ramdisk and starts all under "run/*" (driver manager, disks)
// = = =
// - - DM scans via selectors and builds list of initial devices
// - - DM trys to init those devices (early ret, no fs)
// - - DM loop:
// - - - handle rpc calls (new devices added (ie from usb), device messages such as init suc/fail)
// - - - try init devices that havent been started
// = = =
// - - FS loop:
// - - - handle rpc calls (io, dm event: new disk)
// - - - try parse new disk
// = = =
// - - DSK instance get info (args, fail then req from manager)
// - - DSK init
// - - DSK tell DM init done
// - - DSK run server()
// = = =
// - any future steps
// % mark self as unschedulable forever
