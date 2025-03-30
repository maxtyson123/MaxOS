//Common
#include <stdint.h>
#include <common/version.h>
#include <common/kprint.h>

//Hardware com
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/pci.h>
#include <hardwarecommunication/acpi.h>
#include <hardwarecommunication/apic.h>

//Drivers
#include <drivers/console/console.h>
#include <drivers/console/serial.h>
#include <drivers/console/vesaboot.h>
#include <drivers/driver.h>
#include <drivers/peripherals/keyboard.h>
#include <drivers/peripherals/mouse.h>
#include <drivers/video/vesa.h>
#include <drivers/video/video.h>

//GUI
#include <gui/desktop.h>
#include <gui/window.h>
#include <gui/widgets/text.h>
#include <gui/widgets/button.h>
#include <gui/widgets/inputbox.h>

//NET
#include <net/ethernetframe.h>
#include <net/arp.h>
#include <net/ipv4.h>
#include <net/icmp.h>
#include <net/udp.h>
#include <net/tcp.h>

//PROCESS
#include <processes/process.h>
#include <processes/scheduler.h>

//SYSTEM
#include <system/cpu.h>
#include <system/syscalls.h>
#include <memory/memorymanagement.h>

//MEMORY
#include <memory/physical.h>
#include <memory/virtual.h>

//FILESYSTEM
#include <filesystem/msdospart.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::peripherals;
using namespace MaxOS::drivers::ethernet;
using namespace MaxOS::drivers::video;
using namespace MaxOS::drivers::clock;
using namespace MaxOS::drivers::console;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::gui;
using namespace MaxOS::net;
using namespace MaxOS::processes;
using namespace MaxOS::system;
using namespace MaxOS::memory;
using namespace MaxOS::filesystem;


//TODO: Rework cmake to have debug and prod targets

ConsoleStream* active_stream = nullptr;

extern volatile uint64_t p4_table[512];
extern "C" [[noreturn]] void kernelMain(unsigned long addr, unsigned long magic)  // Only place where it is allowed to not use snake_case
{
    // Initialise the serial console
    SerialConsole serialConsole;

    // Confirm the bootloader
    ASSERT(magic == MULTIBOOT2_BOOTLOADER_MAGIC, "Multiboot2 Bootloader Not Detected");

    // Make the multiboot header
    Multiboot multiboot(addr);
    _kprintf("-= MaxOS booted =-\n");

    InterruptManager interrupts;
    _kprintf("-= IDT set up =-\n");

    uint32_t mbi_size = *(uint32_t *) (addr + MemoryManager::s_higher_half_kernel_offset);
    PhysicalMemoryManager pmm(addr + mbi_size, &multiboot, (uint64_t*)p4_table);
    _kprintf("-= Physical Memory Manager set up =-\n");

    VirtualMemoryManager vmm(true);
    _kprintf("-= Virtual Memory Manager set up =-\n");

    // Initialise the VESA Driver
    VideoElectronicsStandardsAssociation vesa(multiboot.get_framebuffer());
    auto* videoDriver = (VideoDriver*)&vesa;
    videoDriver->set_mode((int)multiboot.get_framebuffer()->common.framebuffer_width,
                          (int)multiboot.get_framebuffer()->common.framebuffer_height,
                          (int)multiboot.get_framebuffer()->common.framebuffer_bpp);


    // Initialise the memory manager
    MemoryManager memoryManager(&vmm);
    MemoryManager::s_kernel_memory_manager = &memoryManager;
    _kprintf("-= Memory Manager set up =-\n");

    // Initialise Console
    VESABootConsole console(&vesa);
    console.clear();
    console.print_logo();

    // Create a stream for the console
    ConsoleArea mainConsoleArea(&console, 0, 0, console.width(), console.height(), ConsoleColour::DarkGrey, ConsoleColour::Black);
    ConsoleStream cout(&mainConsoleArea);
    active_stream = &cout;

    // Header constants
    const string tick = (string)"[ " + ANSI_COLOURS[FG_Green] + "OK" + ANSI_COLOURS[Reset] + " ]";
    const string boot_title = string("Kernel Boot Sequence - MaxOS v") + string(VERSION_STRING) + " [build " + string(BUILD_NUMBER) + "]";
    const int boot_width = boot_title.length() + 20;
    const int sub_boot_width = boot_width;

    // Print helpers
    string out = "";
    #define log(x) out = x; cout << out; cout << (string)" " * (sub_boot_width - out.length() - 6) << tick << "\n"
    #define header(x) cout << "\n\n" << ANSI_COLOURS[FG_White] << "[" << string(x).center(sub_boot_width - 2) << "]\n" << ANSI_COLOURS[Reset]

    // Print the header
    cout << ANSI_COLOURS[FG_Blue] << (string)"=" * boot_width << "\n";
    cout << ANSI_COLOURS[FG_Cyan] << boot_title.center(boot_width) << "\n";
    cout << ANSI_COLOURS[FG_Blue] << (string)"=" * boot_width << "\n";

    // Stuff done earlier
    header("Initialising System Components");
    log("Set Up Serial Console");
    log("Parsed Multiboot");
    log("Set Up Paging");
    log("Set Up Interrupt Manager");
    log("Set Up Physical Memory Manager");
    log("Set Up Virtual Memory Manager");
    log("Set Up Memory Manager (Kernel)");
    log("Set Up Video Driver");


    header("Initialising Hardware");
    DriverManager driverManager(&interrupts);

    AdvancedConfigurationAndPowerInterface acpi(&multiboot);
    log("Set Up ACPI");

    AdvancedProgrammableInterruptController apic(&acpi);
    interrupts.set_apic(apic.get_local_apic());
    log("Set Up APIC");

    // Keyboard
    KeyboardDriver keyboard(&interrupts, apic.get_io_apic());
    KeyboardInterpreterEN_US keyboardInterpreter;
    keyboard.connect_input_stream_event_handler(&keyboardInterpreter);
    driverManager.add_driver(&keyboard);
    log("Set Up Keyboard");

    // Mouse
    MouseDriver mouse(&interrupts, apic.get_io_apic());
    driverManager.add_driver(&mouse);
    log("Set Up Mouse");

    // CPU
    CPU cpu;
    cpu.init_tss();
    log("Set Up CPU");

    // Clock
    Clock kernelClock(&interrupts, &apic, 1);
    driverManager.add_driver(&kernelClock);
    log("Set Up Clock");

    //PCI
    PeripheralComponentInterconnectController pciController(&driverManager);
    log("Set Up PCI");

    //USB
    //UniversalSerialBusController USBController(&driverManager);
    //log("Set Up USB");

    header("Device Management");

    // Find the drivers
    driverManager.find_drivers();
    log("Found Drivers");

    // Reset the devices
    uint32_t reset_wait_time = driverManager.reset_devices();
    log("Reset Devices");

    // Interrupts
    interrupts.activate();
    log("Activating Interrupts");

    // Post interrupt activation
    kernelClock.calibrate();
    kernelClock.delay(reset_wait_time);
    log("Calibrated Clock");

    header("Finalisation");

    Scheduler scheduler(&interrupts, multiboot);
    log("Set Up Scheduler");

    SyscallManager syscalls(&interrupts);
    log("Set Up Syscalls");

    // Initialise the drivers
    driverManager.initialise_drivers();
    log("Initialised Drivers");

    // Activate the drivers
    driverManager.activate_drivers();
    log("Activated Drivers");

    // Print the footer
    cout << "\n\n";
    cout << ANSI_COLOURS[FG_Blue] << (string)"-" * boot_width << "\n";
    cout << ANSI_COLOURS[FG_Cyan] << string(" -- Kernel Ready --").center(boot_width) << "\n";
    cout << ANSI_COLOURS[FG_Blue] << (string)"-" * boot_width << ANSI_COLOURS[Reset] << "\n";
    cout.set_cursor(0, console.height() - 1);

    // Start the Scheduler & updates the clock handler
    scheduler.activate();


    // TODO:
    //       - clean up large functions, all enums use enum class, update notes, public variables check up, includes fix up, old code review, types, const referencing, classes
    //       - PCI to drivers page in osdev book, ubsan section maybe
    //       - Look at the event handler system again?


    /// Boot Done ///
    _kprintf("%h%s[System Booted]%s MaxOS v%s\n", ANSI_COLOURS[FG_Green], ANSI_COLOURS[Reset], VERSION_STRING);


    /// How this idle works:
    ///  I was debugging along and released that on the first ever schedule it will
    ///  set current_thread->execution_state = cpu_state; where it is assumed cpu_state
    ///  is the current thread state (ie what we have just been scheduling) and
    ///  thus saves it in that thread. However, as the first thread has not had a
    ///  chance to be scheduled yet, the current state is not the expected first
    ///  thread's state and instead is the cpu state of the kernel.
    ///  Now I could either fix that or leave it in as a cool way of never fully
    ///  leaving kernelMain and  also having a idle_proc
    while (true){

      // Print the ticks (debugging)
      //_kprintf("%hTick: %d\r", scheduler.get_ticks());

      // yield ? wait until figured out the task manager cpu %

      // Make sure the compiler doesn't optimise the loop away
      asm("nop");

    }
}