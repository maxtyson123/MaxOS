int buildCount = 247;
// This is the build counter, it is incremented every time the build script is run. Started 27/09/2023, Commit 129

//Common
#include <common/types.h>
#include <common/logo.h>

//Hardware com
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/pci.h>
#include <hardwarecommunication/serial.h>

//Drivers
#include <drivers/driver.h>
#include <drivers/peripherals/keyboard.h>
#include <drivers/peripherals/mouse.h>
#include <drivers/video/video.h>
#include <drivers/ata.h>
#include <drivers/ethernet/amd_am79c973.h>
#include <drivers/video/vesa.h>
#include <drivers/console/console.h>
#include <drivers/console/textmode.h>
#include <drivers/console/vesaboot.h>

//GUI
#include <gui/desktop.h>
#include <gui/window.h>
#include <gui/widgets/text.h>

//NET
#include <net/etherframe.h>
#include <net/arp.h>
#include <net/ipv4.h>
#include <net/icmp.h>
#include <net/udp.h>
#include <net/tcp.h>

//SYSTEM
#include <system/process.h>
#include <system/gdt.h>
#include <system/syscalls.h>
#include <memory/memorymanagement.h>
#include <system/multithreading.h>

//MEMORY
#include <memory/memorymanagement.h>

//FILESYSTEM
#include <filesystem/msdospart.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::drivers;
using namespace maxOS::drivers::peripherals;
using namespace maxOS::drivers::ethernet;
using namespace maxOS::drivers::video;
using namespace maxOS::drivers::clock;
using namespace maxOS::drivers::console;
using namespace maxOS::hardwarecommunication;
using namespace maxOS::gui;
using namespace maxOS::net;
using namespace maxOS::system;
using namespace maxOS::memory;
using namespace maxOS::filesystem;


#define ENABLE_GRAPHICS

class KeyboardToStream : public KeyboardEventHandler
{
    OutputStream* stream;
public:
    KeyboardToStream(OutputStream* stream)
    {
        this->stream = stream;
    }

    void OnKeyboardKeyPressed(KeyCode keyCode, KeyboardState)
    {
        if(31 < keyCode && keyCode < 127)
            stream->writeChar((char)keyCode);
    }
};

class MouseToConsole: public MouseEventHandler{

    Console* console;
    int x;
    int y;
    uint8_t buttons;

public:
    MouseToConsole(Console* console)
    {
        this->console = console;
        buttons = 0;
        x = console->getWidth()*2;
        y = console->getHeight()*2;
    }
    

    void onMouseMoveEvent(int8_t x, int8_t y)
    {

        console->invertColors(this->x/4,this->y/4);

        this->x += x;
        if(this->x < 0)
            this->x = 0;
        if(this->x >= console->getWidth()*4)
            this->x = console->getWidth()*4-1;

        this->y += y;
        if(this->y < 0)
            this->y = 0;
        if(this->y >= console->getHeight()*4)
            this->y = console->getHeight()*4-1;

        console->invertColors(this->x/4,this->y/4);
    }


};

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


#pragma clang diagnostic ignored "-Wwritable-strings"

//TODO: Rewrite multiboot to use the one from the manual: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Example-OS-code
extern "C" void kernelMain(const multiboot_info& multibootHeader, uint32_t multiboot_magic)
{

    // Initialize the VESA Driver
    VideoElectronicsStandardsAssociationDriver vesa((multiboot_info_t *)&multibootHeader);
    VideoDriver* videoDriver = (VideoDriver*)&vesa;
    videoDriver -> setMode(1024, 768, 32);

    //Initialize Console
    VESABootConsole console(&vesa);
    //TextModeConsole console;
    console.clear();

    char* logo = header_data;

    // Print the logo to center of the screen
    uint32_t centerX = videoDriver->getWidth()/2;
    uint32_t centerY = videoDriver->getHeight()/2;
    for (int logoY = 0; logoY < logo_height; ++logoY) {
        for (int logoX = 0; logoX < logo_width; ++logoX) {

            // Store the pixel in the logo
            uint8_t pixel[3] = {0};

            // Get the pixel from the logo
            LOGO_HEADER_PIXEL(logo, pixel);

            // Draw the pixel
            videoDriver->putPixel(centerX - logo_width/2 + logoX, centerY - logo_height/2 + logoY, maxOS::common::Colour(pixel[0], pixel[1], pixel[2]));
        }
    }


    // Make the header
    ConsoleArea consoleHeader(&console, 0, 0, console.getWidth(), 1, ConsoleColour::Blue, ConsoleColour::LightGrey);
    ConsoleStream headerStream(&consoleHeader);

    // Write the header
    headerStream << "                                                    Max OS v0.01 [build " << buildCount << "]                                                    " ;

    // Make a main console area at the top of the screen
    ConsoleArea mainConsoleArea(&console, 0, 1, console.getWidth(), console.getHeight(), ConsoleColour::DarkGrey, ConsoleColour::Black);
    ConsoleStream cout(&mainConsoleArea);

    // Make a null stream
    ConsoleArea nullConsoleArea(&console, 0, 0, 0, 0);
    ConsoleStream nullStream(&nullConsoleArea);

    // Check if the bootloader is valid
    if (multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        cout << "Invalid bootloader \n";
        cout << "Got Magic: " << (uint32_t)multiboot_magic << ", Expected Magic: " << MULTIBOOT_BOOTLOADER_MAGIC;
        return;
    }
    cout << "Valid Multiboot Magic\n";

    cout << "\n\n";

    // Make the system setup stream
    ConsoleArea systemSetupHeader(&console, 0, cout.cursorY, console.getWidth(), 1, ConsoleColour::LightGrey, ConsoleColour::Black);
    ConsoleStream systemSetupHeaderStream(&systemSetupHeader);
    systemSetupHeaderStream << "Setting up system";

    //Setup GDT
    GlobalDescriptorTable gdt(multibootHeader);
    cout << "-- Set Up GDT\n";
    systemSetupHeaderStream << ".";

    // Setup Memory
    uint32_t memupper = multibootHeader.mem_upper;
    size_t  heap = 10*1024*1024;                                                          //Start at 10MB
    size_t  memSize = memupper*1024 - heap - 10*1024;                                    //Convert memupper into MB, then subtract the hep and some padding
    MemoryManager memoryManager(heap, memSize);                                //Memory Mangement
    cout << "-- Set Up Memory Management\n";
    systemSetupHeaderStream << ".";

    ThreadManager threadManager;
    cout << "-- Set Up Thread Management\n";
    systemSetupHeaderStream << ".";

    InterruptManager interrupts(0x20, &gdt, &threadManager, &cout);            //Instantiate the function
    cout << "-- Set Up Interrupts\n";
    systemSetupHeaderStream << ".";

    SyscallHandler syscalls(&interrupts, 0x80);                               //Instantiate the function
    cout << "-- Set Up System Calls\n";
    systemSetupHeaderStream << ".";

    cout << "\n";
    systemSetupHeaderStream << "[ DONE ]";

    // Make the device setup stream
    ConsoleArea deviceSetupHeader(&console, 0, cout.cursorY, console.getWidth(), 1, ConsoleColour::LightGrey, ConsoleColour::Black);
    ConsoleStream deviceSetupHeaderStream(&deviceSetupHeader);
    deviceSetupHeaderStream << "Setting up devices";
    
    DriverManager driverManager;

    //Keyboard
    KeyboardToStream kbhandler(&cout);
    KeyboardDriver keyboard(&interrupts);
    KeyboardInterpreterEN_US usKeyboard;
    keyboard.connectInputStreamEventHandler(&usKeyboard);
    driverManager.addDriver(&keyboard);
    cout << "-- Set Up Keyboard\n";
    deviceSetupHeaderStream << ".";

    //Mouse
    MouseDriver mouse(&interrupts);
    driverManager.addDriver(&mouse);
    cout << "-- Set Up Mouse\n";
    deviceSetupHeaderStream << ".";

    //Clock
    Clock kernelClock(&interrupts, 1);
    driverManager.addDriver(&kernelClock);
    cout << "-- Set Up Clock\n";
    deviceSetupHeaderStream << ".";

    //Driver Selectors
    Vector<DriverSelector*> driverSelectors;

    //PCI
    PeripheralComponentInterconnectController PCIController(&nullStream);
    driverSelectors.pushBack(&PCIController);
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
        (*selector)->selectDrivers(&driverManager, &memoryManager, &interrupts, 0);
    }
    cout << " Found\n";
    deviceSetupHeaderStream << ".";

    cout << "\n";
    deviceSetupHeaderStream << "[ DONE ]";

    // Make the activation stream
    ConsoleArea activationHeader(&console, 0, cout.cursorY, console.getWidth(), 1, ConsoleColour::LightGrey, ConsoleColour::Black);
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
    interrupts.Activate();
    kernelClock.delay(resetWaitTime);                                            //Wait for the devices to reset (has to be done after interrupts are activated otherwise the clock interrupt wont trigger)
    cout << "-- Activated Interrupts\n";
    activationHeaderStream << ".";

    // Initialize the drivers
    cout << "-- Initializing Devices";
    for(Vector<Driver*>::iterator driver = driverManager.drivers.begin(); driver != driverManager.drivers.end(); driver++)
    {
        cout << ".";
        (*driver)->activate();
    }
    cout << " Initialized\n";
    activationHeaderStream << ".";

    // Activate the drivers
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



    // Run the GUI 
#ifdef GUI
    Desktop desktop(videoDriver);
    mouse.connectMouseEventHandler(&desktop);
    usKeyboard.connectKeyboardEventHandler(&desktop);
    kernelClock.connectClockEventHandler(&desktop);

    widgets::Text testLabel(0, 0, 120, 20, "Hello World");

    Window testWindow2(&testLabel, "Window 2");
    testWindow2.move(10,10);
    desktop.addChild(&testWindow2);

    Window testWindow(150,10, 150, 150, "Window 1");
    desktop.addChild(&testWindow);
#endif

    // Loop forever
    while (true);
}




#pragma clang diagnostic pop

//MaxOS - May the source be with you...
//TODO: Fix networking later TCP/UDP listening, - sending works
