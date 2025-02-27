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
#include <drivers/disk/ata.h>
#include <drivers/console/console.h>
#include <drivers/console/serial.h>
#include <drivers/console/textmode.h>
#include <drivers/console/vesaboot.h>
#include <drivers/driver.h>
#include <drivers/ethernet/amd_am79c973.h>
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

// Define static constructors
extern "C" void* __dso_handle = nullptr;


//Define what a constructor is
typedef void (*constructor)();

//Iterates over space between start_ctors and end_ctors and jumps into all function pointers
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)        //Iterate over all constructors
        (*i)();                                                     //Call the constructor
}

void writing_proc(void* args)
{

    // Get the string from the args list
    string* str = (string*)args;

    while(true)
    {
        _kprintf("%s\n", str->c_str());
    }
}

extern volatile uint64_t p4_table[512];
extern "C" void kernelMain(unsigned long addr, unsigned long magic)
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
    PhysicalMemoryManager pmm(addr + mbi_size, &multiboot, p4_table);
    _kprintf("-= Physical Memory Manager set up =-\n");

    VirtualMemoryManager vmm(true);
    _kprintf("-= Virtual Memory Manager set up =-\n");

    // Initialise the VESA Driver
    VideoElectronicsStandardsAssociation vesa(multiboot.get_framebuffer());
    VideoDriver* videoDriver = (VideoDriver*)&vesa;
    videoDriver->set_mode((int)multiboot.get_framebuffer()->common.framebuffer_width,
                          (int)multiboot.get_framebuffer()->common.framebuffer_height,
                          (int)multiboot.get_framebuffer()->common.framebuffer_bpp);


    // Initialise the memory manager
    MemoryManager memoryManager(&vmm);
    _kprintf("-= Memory Manager set up =-\n");

    // Initialise Console
    VESABootConsole console(&vesa);
    console.clear();
    console.print_logo();

    // Create a stream for the console
    ConsoleArea mainConsoleArea(&console, 0, 0, console.width(), console.height(), ConsoleColour::DarkGrey, ConsoleColour::Black);
    ConsoleStream cout(&mainConsoleArea);

    // Header constants
    const string tick = (string)"[ " + ANSI_COLOURS[FG_Green] + "OK" + ANSI_COLOURS[Reset] + " ]";
    const string boot_title = string("Kernel Boot Sequence - MaxOS v") + string(VERSION_STRING) + " [build " + string(BUILD_NUMBER) + "]";
    const int boot_width = boot_title.length() + 20;
    const int sub_boot_width = boot_width;

    // Print helpers
    string out = "";
    #define log(x) out = x; cout << out; cout << (string)" " * (sub_boot_width - out.length() - 6) << tick << "\n";
    #define header(x) cout << "\n\n" << ANSI_COLOURS[FG_White] << "[" << string(x).center(sub_boot_width - 2) << "]\n" << ANSI_COLOURS[Reset];

    // Print the header
    cout << ANSI_COLOURS[FG_Blue] << (string)"=" * boot_width << "\n";
    cout << ANSI_COLOURS[FG_Cyan] << boot_title.center(boot_width) << "\n";
    cout << ANSI_COLOURS[FG_Blue] << (string)"=" * boot_width << "\n";

    // Stuff done earlier
    header("Initialising System Components")
    log("Set Up Serial Console");
    log("Parsed Multiboot");
    log("Set Up Paging");
    log("Set Up Interrupt Manager");
    log("Set Up Physical Memory Manager");
    log("Set Up Virtual Memory Manager");
    log("Set Up Memory Manager (Kernel)");
    log("Set Up Video Driver");

    Scheduler scheduler;
    log("Set Up Scheduler");

    SyscallHandler syscalls(&interrupts, 0x80);                               //Instantiate the function
    log("Set Up Syscalls");

    DriverManager driverManager;
    header("Initialising Hardware")

    AdvancedConfigurationAndPowerInterface acpi(&multiboot);
    log("Set Up ACPI");

    AdvancedProgrammableInterruptController apic(&acpi);
    interrupts.set_apic(apic.get_local_apic());
    log("Set Up APIC");


    // Keyboard
    KeyboardDriver keyboard(&interrupts);
    KeyboardInterpreterEN_US keyboardInterpreter;
    keyboard.connect_input_stream_event_handler(&keyboardInterpreter);
    driverManager.add_driver(&keyboard);
    interrupt_redirect_t keyboardRedirect = {
        .type = 0x1,
        .index = 0x12,
        .interrupt = 0x21,
        .destination = 0x00,
        .flags = 0x00,
        .mask = false,
    };
    apic.get_io_apic() -> set_redirect(&keyboardRedirect);
    log("Set Up Keyboard");

    // Mouse
    MouseDriver mouse(&interrupts);
    driverManager.add_driver(&mouse);
    interrupt_redirect_t mouseRedirect = {
        .type = 0xC,
        .index = 0x28,
        .interrupt = 0x2C,
        .destination = 0x00,
        .flags = 0x00,
        .mask = false,
    };
    apic.get_io_apic() -> set_redirect(&mouseRedirect);
    log("Set Up Mouse");


    // Clock
    Clock kernelClock(&interrupts, &apic, 1);
    driverManager.add_driver(&kernelClock);
    log("Set Up Clock");

    // Driver Selectors
    Vector<DriverSelector*> driverSelectors;

    //PCI
    PeripheralComponentInterconnectController PCIController;
    driverSelectors.push_back(&PCIController);
    log("Set Up PCI");

    header("Device Management")

    // Find the drivers
    cout << "Finding Drivers" << ANSI_COLOURS[FG_White];
    for(Vector<DriverSelector*>::iterator selector = driverSelectors.begin(); selector != driverSelectors.end(); selector++)
    {
      cout << ".";
      (*selector)->select_drivers(&driverManager, &interrupts);
    }
    cout << ANSI_COLOURS[Reset] << (string)"."*(boot_width - driverSelectors.size() - 15 - 9) << (string)"[ " + ANSI_COLOURS[FG_Green] + "FOUND" + ANSI_COLOURS[Reset] + " ]" << "\n";


    // Resetting devices
    cout << "Resetting Devices" << ANSI_COLOURS[FG_White];
    uint32_t resetWaitTime = 0;
    for(Vector<Driver*>::iterator driver = driverManager.drivers.begin(); driver != driverManager.drivers.end(); driver++)
    {
      cout << ".";
      uint32_t waitTime = (*driver)->reset();

      // If the wait time is longer than the current longest wait time, set it as the new longest wait time
      if(waitTime > resetWaitTime)
        resetWaitTime = waitTime;
    }
    cout << ANSI_COLOURS[Reset] << (string)"."*(boot_width - driverManager.drivers.size() - 17 - 9) << (string)"[ " + ANSI_COLOURS[FG_Green] + "RESET" + ANSI_COLOURS[Reset] + " ]" << "\n";


    // Interrupts
    interrupts.activate();
    log("Activating Interrupts");

    // Post interupt activation
    kernelClock.calibrate();
    kernelClock.delay(resetWaitTime);
    Time now = kernelClock.get_time();
    cout << "TIME: " << now.hour << ":" << now.minute << ":" << now.second << "\n";

    header("Finalisation")

    // Initialise the drivers
    cout <<  "Initialising Devices" << ANSI_COLOURS[FG_White];
    for(Vector<Driver*>::iterator driver = driverManager.drivers.begin(); driver != driverManager.drivers.end(); driver++)
    {
      cout << ".";
      (*driver)->initialise();
    }
    cout << ANSI_COLOURS[Reset] << (string)"."*(boot_width - driverManager.drivers.size() - 20 - 15) << (string)"[ " + ANSI_COLOURS[FG_Green] + "INITIALISED" + ANSI_COLOURS[Reset] + " ]" << "\n";


    // activate the drivers
    cout << "Activating Devices" << ANSI_COLOURS[FG_White];
    for(Vector<Driver*>::iterator driver = driverManager.drivers.begin(); driver != driverManager.drivers.end(); driver++)
    {
      cout << ".";
      (*driver)->activate();
    }
    cout << ANSI_COLOURS[Reset] << (string)"."*(boot_width - driverManager.drivers.size() - 18 - 13) << (string)"[ " + ANSI_COLOURS[FG_Green] + "ACTIVATED" + ANSI_COLOURS[Reset] + " ]" << "\n";


    // Print the footer
    cout << "\n\n";
    cout << ANSI_COLOURS[FG_Blue] << (string)"-" * boot_width << "\n";
    cout << ANSI_COLOURS[FG_Cyan] << string(" -- Kernel Ready --").center(boot_width) << "\n";
    cout << ANSI_COLOURS[FG_Blue] << (string)"-" * boot_width << "\n";


    // Start the scheduler
    Process* p1 = new Process("Test Process 1", writing_proc, (void*)new string("Hello from Process 1"));
    Process* p2 = new Process("Test Process 2", writing_proc, (void*)new string("Hello from Process 2"));
    Process* p3 = new Process("Test Process 3", writing_proc, (void*)new string("Hello from Process 3"));

    scheduler.activate();

    // TODO (TEST AFTER FIXING UBSAN AND GDB ERRORS):
    // - Fix debugging
    // - Fix GPE
    // - Test clean up used frames when Process dies

    // Wait
    while (true);

}