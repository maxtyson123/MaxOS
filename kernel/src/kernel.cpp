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


//TODO: Rework cmake to have debug and prod targets

extern "C" [[noreturn]] void kernelMain(unsigned long addr, unsigned long magic)  // Only place where it is allowed to not use snake_case
{
    // Initialise the logger
    Logger logger;
    SerialConsole serialConsole;
    logger.add_log_writer(&serialConsole);

    // Make the multiboot header
    Multiboot multiboot(addr, magic);
    Logger::INFO() << "MaxOS Booted Successfully \n";

    // Initialise the IDT
    InterruptManager interrupts;
    Logger::INFO() << "Interrupt Manager set up \n";

    // Initialise the PMM
    PhysicalMemoryManager pmm(&multiboot);
    Logger::INFO() << "Physical Memory Manager set up \n";

    // Initialise the Memory Manager
    VirtualMemoryManager vmm;
    MemoryManager memoryManager(&vmm);
    Logger::INFO() << "Virtual Memory Manager set up \n";

    // Initialise the VESA Driver
    VideoElectronicsStandardsAssociation vesa(multiboot.framebuffer());
    Logger::INFO() << "VESA Driver set up \n";

    // Initialise Console
    VESABootConsole console(&vesa);
    Logger::INFO() << "Console set up \n";

    // Stuff done earlier
    Logger::HEADER() << "Initialising System Components \n";
    Logger::INFO() << "Set Up Serial Console \n";
    Logger::INFO() << "Parsed Multiboot \n";
    Logger::INFO() << "Set Up Paging \n";
    Logger::INFO() << "Set Up Interrupt Manager \n";
    Logger::INFO() << "Set Up Physical Memory Manager \n";
    Logger::INFO() << "Set Up Virtual Memory Manager \n";
    Logger::INFO() << "Set Up Memory Manager (Kernel) \n";
    Logger::INFO() << "Set Up Video Driver \n";

    Logger::HEADER() << "Initialising Hardware"; Logger::Endline();
    DriverManager driver_manager;

    AdvancedConfigurationAndPowerInterface acpi(&multiboot);
    Logger::INFO() << "Set Up ACPI \n";

    AdvancedProgrammableInterruptController apic(&acpi);
    Logger::INFO() << "Set Up APIC \n";

    // Keyboard (TODO: Move to userspace PS/2 driver)
    KeyboardDriver keyboard;
    KeyboardInterpreterEN_US keyboardInterpreter;
    keyboard.connect_input_stream_event_handler(&keyboardInterpreter);
    driver_manager.add_driver(&keyboard);
    Logger::INFO() << "Set Up Keyboard \n";

    // Mouse (TODO: Move to userspace PS/2 driver)
    MouseDriver mouse;
    driver_manager.add_driver(&mouse);
    Logger::INFO() << "Set Up Mouse \n";

    // CPU
    CPU cpu;
    Logger::INFO() << "Set Up CPU \n";

    // Clock
    Clock kernelClock(&apic, 1);
    driver_manager.add_driver(&kernelClock);
    Logger::INFO() << "Set Up Clock \n";

    //USB
    //UniversalSerialBusController USBController(&driver_manager);
    //Logger::INFO() << "Set Up USB \n";

    Logger::HEADER() << "Device Management"; Logger::Endline();

    // Find the drivers
    driver_manager.find_drivers();
    Logger::INFO() << "Found Drivers \n";

    // Reset the devices
    uint32_t reset_wait_time = driver_manager.reset_devices();
    Logger::INFO() << "Reset Devices \n";

    // Interrupts
    interrupts.activate();
    Logger::INFO() << "Activating Interrupts \n";

    // Post interrupt activation
    kernelClock.calibrate();
    kernelClock.delay(reset_wait_time);
    Logger::INFO() << "Calibrated Clock \n";

    Logger::HEADER() << "Finalisation"; Logger::Endline();

    Scheduler scheduler(multiboot);
    Logger::INFO() << "Set Up Scheduler \n";

    SyscallManager syscalls;
    Logger::INFO() << "Set Up Syscalls \n";

    // Initialise the drivers
    driver_manager.initialise_drivers();
    Logger::INFO() << "Initialised Drivers \n";

    // Activate the drivers
    driver_manager.activate_drivers();
    Logger::INFO() << "Activated Drivers \n";

    // Kernel Boot done
    Logger::HEADER() << "MaxOS Kernel Successfully Booted"; Logger::Endline();
    console.finish();

    // Start the Scheduler & updates the clock handler
    scheduler.activate();

    // TODO:
    //       -   Rewrite boot text again to have progress bar
    //       -   Rewrite boot script to be in c++ where possible


    /// How this idle works:
    ///  I was debugging along and released that on the first ever schedule it will
    ///  set current_thread->execution_state = cpu_state; where it is assumed cpu_state
    ///  is the current thread state (ie what we have just been scheduling) and
    ///  thus saves it in that thread. However, as the first thread has not had a
    ///  chance to be scheduled yet, the current state is not the expected first
    ///  thread's state and instead is the cpu state of the kernel.
    ///  Now I could either fix that or leave it in as a cool way of never fully
    ///  leaving kernelMain and  also having a idle_proc
    while (true)
      asm("nop");
}