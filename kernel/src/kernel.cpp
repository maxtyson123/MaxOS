//Common
#include <stdint.h>
#include <common/logger.h>

//Hardware com
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/acpi.h>
#include <hardwarecommunication/apic.h>

//Drivers
#include <drivers/console/serial.h>
#include <drivers/console/vesaboot.h>
#include <drivers/driver.h>
#include <drivers/peripherals/keyboard.h>
#include <drivers/peripherals/mouse.h>
#include <drivers/video/vesa.h>

//GUI
#include <gui/desktop.h>

//PROCESS
#include <processes/scheduler.h>

//SYSTEM
#include <system/cpu.h>
#include <system/syscalls.h>
#include <memory/memorymanagement.h>

//MEMORY
#include <memory/physical.h>
#include <memory/virtual.h>


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

extern "C" void call_constructors();
extern "C" [[noreturn]] void kernel_main(unsigned long addr, unsigned long magic)
{
    call_constructors();

    // Initialise the logger
    Logger logger;
    SerialConsole serialConsole(&logger);
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
    DriverManager driver_manager;
    AdvancedConfigurationAndPowerInterface acpi(&multiboot);
    AdvancedProgrammableInterruptController apic(&acpi);
    CPU cpu(&gdt);
    Clock kernelClock(&apic, 1);
    driver_manager.add_driver(&kernelClock);
    driver_manager.find_drivers();
    uint32_t reset_wait_time = driver_manager.reset_devices();

    Logger::HEADER() << "Stage {3}: Device Finalisation\n";
    interrupts.activate();
    kernelClock.calibrate();
    kernelClock.delay(reset_wait_time);
    driver_manager.initialise_drivers();
    driver_manager.activate_drivers();

    Logger::HEADER() << "Stage {4}: System Finalisation\n";
    Scheduler scheduler(multiboot);
    SyscallManager syscalls;
    console.finish();
    scheduler.activate();

    // TODO:
    //       -   Clean up scripts dir
    //       -   Fix the random elf crash

    // Idle loop  (read Idle.md)
    while (true)
      asm("nop");
}