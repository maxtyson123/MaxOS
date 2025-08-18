//Common
#include <stdint.h>
#include <common/logger.h>
#include <common/buffer.h>
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

extern "C" void call_constructors();
extern "C" [[noreturn]] void kernel_main(unsigned long addr, unsigned long magic)
{
    call_constructors();

    // Initialise the logger
    Logger logger;
    SerialConsole serial_console(&logger);
    Logger::INFO() << "MaxOS Booted Successfully \n";

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
    Clock kernel_clock(cpu.apic, 1);
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

	// FS Tests (TOOD: Cant read contents of maxos created entries)
	File* file = vfs.open_file("/test/working.file");
	buffer_t fb(100);
	file->read(&fb, 100);
	Logger::DEBUG() << "FILE:" << (char*)fb.raw() << "\n";

	Logger::HEADER() << "Stage {4}: System Finalisation\n";
    Scheduler scheduler(multiboot);
    SyscallManager syscalls;
    console.finish();
    scheduler.activate();

    // Idle loop  (read Idle.md)
    while (true)
      asm("nop");
}

// TODO:
//  - EXT2 Tests:
//  - [x] Read subdirectories contents
//  - [x] Read files
//  - [x] Write files
//  - [x] Create subdirectories
//  - [ ] Delete subdirectories
//  - [ ] Rename directory
//  - [ ] Rename file
//  - [x] Create files
//  - [ ] Delete files
//  - [ ] Stress test the filesystem: 1000s of files in a directory, long nested directories, long path files, large file r/w


//  - Fix tabs (mac mess up)
//  - Userspace Files (syscalls, proper path handling, working directories, file handles)
//  - Class & Struct docstrings
//  - Logo on fail in center