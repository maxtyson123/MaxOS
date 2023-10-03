int buildCount = 109;
// This is the build counter, it is incremented every time the build script is run. Started 27/09/2023, Commit 129

//Common
#include <common/types.h>

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
#include <drivers/console/textmodeconsole.h>

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


    //Initialize Console
    TextModeConsole console;
    console.clear();

    // Make a debug console area on the bottom 4 lines of the screen
    ConsoleArea debugConsoleArea(&console, 0, console.getHeight() - 4, console.getWidth(), 4, Green, DarkGrey);
    ConsoleStream debugStream(&debugConsoleArea);

    // Make a main console area on the top of the screen
    ConsoleArea mainConsoleArea(&console, 0, 1, console.getWidth(), console.getHeight() - 4);
    ConsoleStream cout(&mainConsoleArea);

    // Make a null stream
    ConsoleArea nullConsoleArea(&console, 0, 0, 0, 0);
    ConsoleStream nullStream(&nullConsoleArea);

    console.putString(0,0,"                                  Max OS v0.0.1                                 ", Blue, LightGrey);
    console.putString(0,console.getHeight() - 5,"Debug Log: ", DarkGrey, Black);

    cout << "Build: ";
    cout << buildCount;
    cout << "\n";



    // Check if the bootloader is valid
    if (multiboot_magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        cout << "Invalid bootloader \n";
        cout << "Got Magic: ";
        cout << (uint32_t)multiboot_magic;
        cout << ", Expected Magic: ";
        cout << MULTIBOOT_BOOTLOADER_MAGIC;
        return;
    }
    debugStream << "Valid Multiboot Magic, ";

    cout << "Setting up system";


    //Setup GDT
    GlobalDescriptorTable gdt(multibootHeader);
    debugStream << "Set up GDT, ";
    cout << ".";

    // Setup Memory
    uint32_t memupper = multibootHeader.mem_upper;                    //memupper is a field at offset 8 in the Multiboot information structure and it indicates the amount upper memory in kilobytes.
                                                                                            //Lower memory starts at address 0, and upper memory starts at address 1 megabyte. The
                                                                                            //maximum possible value for lower memory is 640 kilobytes. The value returned for upper
                                                                                            //memory is maximally the address of the first upper memory hole minus 1 megabyte.
    size_t  heap = 10*1024*1024;                                                            //Start at 10MB

    size_t  memSize = memupper*1024 - heap - 10*1024;                                    //Convert memupper into MB, then subtract the hep and some padding
    MemoryManager memoryManager(heap, memSize);                                //Memory Mangement
    void* allocated = memoryManager.malloc(1024);
    debugStream << "Set Up Memory Management, ";
    cout << ".";

    ThreadManager threadManager;
    debugStream << "Set Up Memory Management, ";
    cout << ".";

    InterruptManager interrupts(0x20, &gdt, &threadManager, &cout);            //Instantiate the function
    debugStream << "Set Up Interrupts, ";
    cout << ".";

    SyscallHandler syscalls(&interrupts, 0x80);                               //Instantiate the function
    debugStream << "Set Up System Calls, ";
    cout << ".";

    cout << "[ Done ]\n";

    cout << "Setting up devices";

    serial serialLog(&interrupts);
    serialLog.Write("\n",-1);
    serialLog.Write("Serial Log Started\n");
    serialLog.Write("MaxOS Started\n",7);
    debugStream << "Set Up Serial Log, ";
    cout << ".";

    DriverManager driverManager;

    //Keyboard
    KeyboardToStream kbhandler(&cout);
    KeyboardDriver keyboard(&interrupts);
    KeyboardInterpreterEN_US usKeyboard;
    keyboard.connectInputStreamEventHandler(&usKeyboard);
    driverManager.AddDriver(&keyboard);
    debugStream << "Set Up Keyboard, ";
    cout << ".";

    //Mouse
    MouseDriver mouse(&interrupts);
    driverManager.AddDriver(&mouse);
    debugStream << "Set Up Mouse, ";
    cout << ".";

    //Clock
    Clock kernelClock(&interrupts, 1);
    driverManager.AddDriver(&kernelClock);

    //PCI
    PeripheralComponentInterconnectController PCIController(&nullStream);
    
    PCIController.SelectDrivers(&driverManager, &interrupts);

    debugStream << "Set Up PCI, ";
    cout << ".";

    cout << "[ DONE ] \n";

    cout << "Activating Everything";

    // Interrupts
    interrupts.Activate();
    debugStream << "Activated Interrupts, ";
    cout << ".";

    //Drivers
    driverManager.ActivateAll();
    debugStream << "Activated Drivers, ";
    cout << ".";

    cout << "[ DONE ] \n";

    cout << "Setting Up Graphics";

    VideoElectronicsStandardsAssociationDriver vesa(&memoryManager, (multiboot_info_t *)&multibootHeader);
    VideoDriver* videoDriver = (VideoDriver*)&vesa;
    videoDriver -> setMode(1024, 768, 32);
    debugStream << "Got Video Driver\n";
    cout << ".";

    Desktop desktop(videoDriver);
    mouse.connectMouseEventHandler(&desktop);
    usKeyboard.connectKeyboardEventHandler(&desktop);
    kernelClock.connectClockEventHandler(&desktop);
    debugStream << "Connected Desktop\n";
    cout << ".";

    widgets::Text testLabel(0, 0, 120, 20, "Hello World");

    Window testWindow2(&testLabel, "Window 2");
    testWindow2.move(10,10);
    desktop.addChild(&testWindow2);

    Window testWindow(150,10, 150, 150, "Window 1");
    desktop.addChild(&testWindow);

    cout << "[ DONE ] \n";

    while(true);

}




#pragma clang diagnostic pop

//MaxOS - May the source be with you...
//TODO: Fix networking later TCP/UDP listening, - sending works
