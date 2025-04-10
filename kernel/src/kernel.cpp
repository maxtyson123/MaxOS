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

extern "C" [[noreturn]] void kernel_main(unsigned long addr, unsigned long magic)
{
    // Initialise the logger
    Logger logger;
    SerialConsole serialConsole;
    logger.add_log_writer(&serialConsole);

    GlobalDescriptorTable gdt;
    Logger::INFO() << "GDT Loaded \n";

    Multiboot multiboot(addr, magic);
    Logger::INFO() << "MaxOS Booted Successfully \n";

    Logger::INFO() << "Setting up Interrupt Manager\n";
    InterruptManager interrupts;

    Logger::INFO() << "Setting up Physical Memory Manager\n";
    PhysicalMemoryManager pmm(&multiboot);

    Logger::INFO() << "Setting up Kernel Memory Manager\n";
    VirtualMemoryManager vmm;
    MemoryManager memoryManager(&vmm);

    Logger::INFO() << "Setting up VESA Driver\n";
    VideoElectronicsStandardsAssociation vesa(multiboot.framebuffer());

    Logger::INFO() << "Setting up VESA Console\n";
    VESABootConsole console(&vesa);

    Logger::HEADER() << "Initialising Hardware\n";
    DriverManager driver_manager;

    Logger::INFO() << "Setting up ACPI\n";
    AdvancedConfigurationAndPowerInterface acpi(&multiboot);

    Logger::INFO() << "Setting up APIC\n";
    AdvancedProgrammableInterruptController apic(&acpi);

    // TODO: Move to userspace PS/2 driver)
    Logger::INFO() << "Setting up Keyboard\n";
    KeyboardDriver keyboard;
    KeyboardInterpreterEN_US keyboardInterpreter;
    keyboard.connect_input_stream_event_handler(&keyboardInterpreter);
    driver_manager.add_driver(&keyboard);

    // TODO: Move to userspace PS/2 driver)
    Logger::INFO() << "Setting up Mouse \n";
    MouseDriver mouse;
    driver_manager.add_driver(&mouse);

    Logger::INFO() << "Setting up CPU \n";
    CPU cpu(&gdt);

    Logger::INFO() << "Setting up Clock \n";
    Clock kernelClock(&apic, 1);
    driver_manager.add_driver(&kernelClock);

    //Logger::INFO() << "Setting up USB \n";
    //UniversalSerialBusController USBController(&driver_manager);

    Logger::INFO() << "Finding Drivers \n";
    driver_manager.find_drivers();

    Logger::INFO() << "Resetting Devices \n";
    uint32_t reset_wait_time = driver_manager.reset_devices();

    Logger::INFO() << "Activating Interrupts \n";
    interrupts.activate();

    Logger::INFO() << "Calibrating Clock \n";
    kernelClock.calibrate();
    kernelClock.delay(reset_wait_time);

    Logger::HEADER() << "Finalisation\n";

    Logger::INFO() << "Setting up Scheduler \n";
    Scheduler scheduler(multiboot);

    Logger::INFO() << "Setting up Syscalls \n";
    SyscallManager syscalls;

    Logger::INFO() << "Initialising Drivers \n";
    driver_manager.initialise_drivers();

    Logger::INFO() << "Activating Drivers \n";
    driver_manager.activate_drivers();

    Logger::HEADER() << "MaxOS Kernel Successfully Booted\n";
    console.finish();
    scheduler.activate();

    // TODO:
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