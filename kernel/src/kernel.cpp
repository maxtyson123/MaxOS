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

//SYSTEM
#include <system/process.h>
#include <system/cpu.h>
#include <system/syscalls.h>
#include <memory/memorymanagement.h>
#include <system/multithreading.h>

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
using namespace MaxOS::system;
using namespace MaxOS::memory;
using namespace MaxOS::filesystem;

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


bool check_multiboot_flag(uint32_t flags, uint32_t bit)
{
    return (flags & (1 << bit)) > 0;
}

void print_boot_header(Console* console){

  // Make the header
  ConsoleArea consoleHeader(console, 0, 0, console -> width(), 1, ConsoleColour::Blue, ConsoleColour::LightGrey);
  ConsoleStream headerStream(&consoleHeader);

  // Calculate the header
  string header = string("MaxOS v") + string(VERSION_STRING) + " [build " + string(BUILD_NUMBER) + "]";
  int headerPadding = (console -> width() - header.length()) / 2;

  // Print the headers
  for(int i = 0; i < headerPadding; i++)
        headerStream << " ";

  headerStream << header;

  for (int i1 = 0; i1 < headerPadding; ++i1) {
        headerStream << " ";
  }

}

extern volatile uint64_t p4_table[512];
extern "C" void kernelMain(unsigned long addr, unsigned long magic)
{

    // Initialise the serial console
    SerialConsole serialConsole;

    // Make the multiboot header
    Multiboot multiboot(addr);

    _kprintf("MaxOS booted\n");

    //GlobalDescriptorTable gdt;
    //_kprintf("GDT set up\n");

    InterruptManager interrupts(0x20, 0);
    _kprintf("IDT set up\n");

    uint32_t mbi_size = *(uint32_t *) (addr + MemoryManager::s_higher_half_offset);
    PhysicalMemoryManager pmm(addr + mbi_size, &multiboot, p4_table);
    _kprintf("Physical Memory Manager set up \n");
    _kprintf("Verify Page Root Address: 0x%x\n", &p4_table);

    AdvancedConfigurationAndPowerInterface acpi(&multiboot);
    _kprintf("ACPI set up\n");

    AdvancedProgrammableInterruptController apic(&acpi);
    _kprintf("APIC set up\n");

    interrupts.activate();
    _kprintf("IDT activated\n");

    // TODO: 64 bit architecture rewrite
    //  - Fix Paging
    //  - Finish ACPI
    //  - Memory Allocation
    //  - Convert old codebase
    _kprintf("KERNEL DONE\n");
    while (true) {
         //TODO: This causes a Double Fault and then infinte General Protection Faults
         system::CPU::halt();
    }

    // Init memory management
    MemoryManager memoryManager(multiboot.get_mmap());
    // TODO: Alot needs to be page mapped and higher halfed


    // Initialise the VESA Driver
    VideoElectronicsStandardsAssociation vesa(multiboot.get_framebuffer());
    VideoDriver* videoDriver = (VideoDriver*)&vesa;
    videoDriver->set_mode((int)multiboot.get_framebuffer()->common.framebuffer_width,
                          (int)multiboot.get_framebuffer()->common.framebuffer_height,
                          (int)multiboot.get_framebuffer()->common.framebuffer_bpp);

    // Initialise Console
    VESABootConsole console(&vesa);
    console.clear();
    console.print_logo();

    // Create a stream for the console
    ConsoleArea mainConsoleArea(&console, 0, 1, console.width(), console.height(), ConsoleColour::DarkGrey, ConsoleColour::Black);
    ConsoleStream cout(&mainConsoleArea);

    if(magic == MULTIBOOT2_BOOTLOADER_MAGIC)
        cout << "Multiboot2 Bootloader Detected\n";

    return;

    // Print the header
    print_boot_header(&console);

    // Print the build info
    cout << "BUILD INFO: " << VERSION_NAME << " on " << BUILD_DATE.year << "-" << BUILD_DATE.month << "-" << BUILD_DATE.day << " at " << BUILD_DATE.hour << ":" << BUILD_DATE.minute << ":" << BUILD_DATE.second << " " << " (commit " << GIT_REVISION << " on " << GIT_BRANCH << " by " << GIT_AUTHOR << ")\n";

    // Where the areas should start
    cout.set_cursor(cout.m_cursor_x, cout.m_cursor_y + 1); //Move the cursor down one (so the header is not overwritten
    uint32_t areaStart = cout.m_cursor_y;

    // Make the system setup stream
    ConsoleArea systemSetupHeader(&console, 0, areaStart, console.width(), 1, ConsoleColour::LightGrey, ConsoleColour::Black);
    ConsoleStream systemSetupHeaderStream(&systemSetupHeader);
    systemSetupHeaderStream << "Setting up system";

    //Setup GDT
    // TODO:  GlobalDescriptorTable gdt;
    cout << "-- Set Up GDT\n";
    systemSetupHeaderStream << ".";

    // Print that the memory has been set up
    cout << "-- Set Up Memory Management\n";
    systemSetupHeaderStream << ".";

    ThreadManager threadManager;
    cout << "-- Set Up Thread Management\n";
    systemSetupHeaderStream << ".";

    //TODO: InterruptManager interrupts(0x20, &gdt, &threadManager, &cout);            //Instantiate the function
    cout << "-- Set Up Interrupts\n";
    systemSetupHeaderStream << ".";

    SyscallHandler syscalls(&interrupts, 0x80);                               //Instantiate the function
    cout << "-- Set Up System Calls\n";
    systemSetupHeaderStream << ".";

    cout << "\n";
    systemSetupHeaderStream << "[ DONE ]";

    // Make the device setup stream
    ConsoleArea deviceSetupHeader(&console, 0, cout.m_cursor_y, console.width(), 1, ConsoleColour::LightGrey, ConsoleColour::Black);
    ConsoleStream deviceSetupHeaderStream(&deviceSetupHeader);
    deviceSetupHeaderStream << "Setting up devices";
    
    DriverManager driverManager;

    //Keyboard
    KeyboardDriver keyboard(&interrupts);
    KeyboardInterpreterEN_US keyboardInterpreter;
    keyboard.connect_input_stream_event_handler(&keyboardInterpreter);
    driverManager.add_driver(&keyboard);
    cout << "-- Set Up Keyboard\n";
    deviceSetupHeaderStream << ".";

    //Mouse
    MouseDriver mouse(&interrupts);
    driverManager.add_driver(&mouse);
    cout << "-- Set Up Mouse\n";
    deviceSetupHeaderStream << ".";

    //Clock
    Clock kernelClock(&interrupts, 1);
    driverManager.add_driver(&kernelClock);
    cout << "-- Set Up Clock\n";
    deviceSetupHeaderStream << ".";

    //Driver Selectors
    Vector<DriverSelector*> driverSelectors;

    //Make the stream on the side for the PCI
    ConsoleArea pciConsoleArea(&console, console.width() - 45, areaStart+1, 45, console.height()/2, ConsoleColour::DarkGrey, ConsoleColour::Black);
    ConsoleStream pciConsoleStream(&pciConsoleArea);
    console.put_string(console.width() - 45, areaStart, "                 PCI Devices                 ", ConsoleColour::LightGrey, ConsoleColour::Black);
    
    //PCI
    PeripheralComponentInterconnectController PCIController(&pciConsoleStream);
    driverSelectors.push_back(&PCIController);
    cout << "-- Set Up PCI\n";
    deviceSetupHeaderStream << ".";

    //USB
    //UniversalSerialBusController USBController(&nullStream);
    //driverSelectors.pushBack(&USBController);
    //cout << "-- Set Up USB\n";
    //deviceSetupHeaderStream << ".";

    // Find the drivers
    cout << "-- Finding Drivers";
    for(Vector<DriverSelector*>::iterator selector = driverSelectors.begin(); selector != driverSelectors.end(); selector++)
    {
        cout << ".";
        (*selector)->select_drivers(&driverManager, &interrupts);
    }
    cout << " Found\n";
    deviceSetupHeaderStream << ".";

    cout << "\n";
    deviceSetupHeaderStream << "[ DONE ]";

    // Make the activation stream
    ConsoleArea activationHeader(&console, 0, cout.m_cursor_y, console.width(), 1, ConsoleColour::LightGrey, ConsoleColour::Black);
    ConsoleStream activationHeaderStream(&activationHeader);
    activationHeaderStream << "Initializing Hardware";

    // Resetting devices
    cout << "-- Resetting Devices";
    uint32_t resetWaitTime = 0;
    for(Vector<Driver*>::iterator driver = driverManager.drivers.begin(); driver != driverManager.drivers.end(); driver++)
    {
        cout << ".";
        uint32_t waitTime = (*driver)->reset();

        // If the wait time is longer than the current longest wait time, set it as the new longest wait time
        if(waitTime > resetWaitTime)
            resetWaitTime = waitTime;
    }
    cout << " Reset\n";
    activationHeaderStream << ".";

    // Interrupts
    interrupts.activate();
    kernelClock.delay(resetWaitTime);                                            //Wait for the devices to reset (has to be done after interrupts are activated otherwise the clock interrupt wont trigger)
    cout << "-- Activated Interrupts\n";
    activationHeaderStream << ".";

    // Initialise the drivers
    cout << "-- Initializing Devices";
    for(Vector<Driver*>::iterator driver = driverManager.drivers.begin(); driver != driverManager.drivers.end(); driver++)
    {
        cout << ".";
        (*driver)->initialise();
    }
    cout << " Initialised\n";
    activationHeaderStream << ".";

    // activate the drivers
    cout << "-- Activating Devices";
    for(Vector<Driver*>::iterator driver = driverManager.drivers.begin(); driver != driverManager.drivers.end(); driver++)
    {
        cout << ".";
        (*driver)->activate();
    }
    cout << " Activated\n";
    activationHeaderStream << ".";

    cout << "\n";
    activationHeaderStream << "[ DONE ]";

    // Make the network setup stream
    ConsoleArea networkSetupHeader(&console, 0, cout.m_cursor_y, console.width(), 1, ConsoleColour::LightGrey, ConsoleColour::Black);
    ConsoleStream networkSetupHeaderStream(&networkSetupHeader);
    networkSetupHeaderStream << "Setting up network";

    // Make the stream on the side for the network
    ConsoleArea networkConsoleArea(&console, console.width() - 40, 2 + console.height()/2, 45,
        console.height()/2, ConsoleColour::DarkGrey, ConsoleColour::Black);
    ConsoleStream networkConsoleStream(&networkConsoleArea);
    console.put_string(console.width() - 40, 1 + console.height() / 2,
                       "                 Network                    ",
                       ConsoleColour::LightGrey, ConsoleColour::Black);

    // Get the driver
    EthernetDriver* ethernetDriver = (EthernetDriver*)driverManager.drivers[4];
    ethernetDriver->m_driver_message_stream = &networkConsoleStream;
    cout << "Got Ethernet Driver: " << ethernetDriver->get_device_name() << "\n";
    networkSetupHeaderStream << ".";

    // Ethernet Frame Handler
    EthernetFrameHandler ethernetFrameHandler(ethernetDriver, &networkConsoleStream);
    cout << "-- Set Up Ethernet Frame Handler\n";
    networkSetupHeaderStream << ".";

    // IPv4 (using qemu's default network settings)
    SubnetMask subnetMask = InternetProtocolHandler::CreateSubnetMask(255, 255, 255, 0);
    InternetProtocolAddress defaultGateway = InternetProtocolHandler::CreateInternetProtocolAddress(10, 0, 2, 2);
    InternetProtocolAddress ipAddress = InternetProtocolHandler::CreateInternetProtocolAddress(10, 0, 2, 15);
    InternetProtocolHandler internetProtocolHandler(&ethernetFrameHandler, ipAddress, defaultGateway, subnetMask, &networkConsoleStream);
    cout << "-- Set Up IPv4\n";
    networkSetupHeaderStream << ".";

    // ARP
    AddressResolutionProtocol arp(&ethernetFrameHandler, &internetProtocolHandler, &networkConsoleStream);
    cout << "-- Set Up ARP\n";
    networkSetupHeaderStream << ".";

    // ICMP
    InternetControlMessageProtocol icmp(&internetProtocolHandler, &networkConsoleStream);
    cout << "-- Set Up ICMP\n";
    networkSetupHeaderStream << ".";

    // TCP
    TransmissionControlProtocolHandler tcp(&internetProtocolHandler, &networkConsoleStream);
    cout << "-- Set Up TCP\n";
    networkSetupHeaderStream << ".";

    // UDP
    UserDatagramProtocolHandler udp(&internetProtocolHandler, &networkConsoleStream);
    cout << "-- Set Up UDP\n";
    networkSetupHeaderStream << ".";
    cout << "\n";
    networkSetupHeaderStream << "[ DONE ]";

#define GUI
#ifdef GUI
    Desktop desktop(videoDriver);
    mouse.connect_event_handler(&desktop);
    keyboardInterpreter.connect_event_handler(&desktop);
    kernelClock.connect_event_handler(&desktop);

    Window testWindow(150,10, 200, 150, "Test Window");
    widgets::InputBox testInputBox(10, 10, 150, 20, "test");

    class InputBoxStream : public widgets::InputBoxEventHandler
    {
        ConsoleStream* stream;
        public:
        InputBoxStream(ConsoleStream* stream)
        {
            this->stream = stream;
        }
        ~InputBoxStream()
        {
            this->stream = nullptr;
        }

        void on_input_box_text_changed(string newText)
        {
            *stream << "Input Box Changed: " << newText << "\n";
        }
    };
    InputBoxStream inputBoxStream(&cout);
    testInputBox.connect_event_handler(&inputBoxStream);
    testWindow.add_child(&testInputBox);
    desktop.add_child(&testWindow);

    Window testWindow2(350,100, 200, 150, "Test Window 2");
    desktop.add_child(&testWindow2);

#endif

    // Wait
    while (true);

}