int buildCount = 466;
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

    void onKeyDown(drivers::peripherals::KeyCode keyDownCode, drivers::peripherals::KeyboardState keyDownState)
    {
        // If the key is a printable character, write it to the stream
        if(31 < keyDownCode && keyDownCode < 127)
            stream->writeChar((char)keyDownCode);

        // If it is a backspace, delete the last character
        if(keyDownCode == KeyCode::backspace)
            stream->write("\b \b");

        // If it is a newline the prompt
        if(keyDownCode == KeyCode::enter)
        {
            stream->write("\nMaxOS> ");
        }

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

    // Initialise the VESA Driver
    VideoElectronicsStandardsAssociationDriver vesa((multiboot_info_t *)&multibootHeader);
    VideoDriver* videoDriver = (VideoDriver*)&vesa;
    videoDriver -> setMode(1024, 768, 32);

    //Initialise Console
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
    cout << "\n";

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
    KeyboardDriver keyboard(&interrupts);
    KeyboardInterpreterEN_US keyboardInterpreter;
    keyboard.connectInputStreamEventHandler(&keyboardInterpreter);
    KeyboardToStream kbhandler(&cout);
    //keyboardInterpreter.connectEventHandler(&kbhandler);
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

    //Make the stream on the side for the PCI
    ConsoleArea pciConsoleArea(&console, console.getWidth() - 45, 2, 45, console.getHeight()/2, ConsoleColour::DarkGrey, ConsoleColour::Black);
    ConsoleStream pciConsoleStream(&pciConsoleArea);
    console.putString(console.getWidth() - 45, 1, "                 PCI Devices                 ", ConsoleColour::LightGrey, ConsoleColour::Black);
    
    //PCI
    PeripheralComponentInterconnectController PCIController(&pciConsoleStream);
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
        (*selector)->selectDrivers(&driverManager, &interrupts, 0);
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

    // Initialise the drivers
    cout << "-- Initializing Devices";
    for(Vector<Driver*>::iterator driver = driverManager.drivers.begin(); driver != driverManager.drivers.end(); driver++)
    {
        cout << ".";
        (*driver)->initialise();
    }
    cout << " Initialised\n";
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


    // Make the network setup stream
    ConsoleArea networkSetupHeader(&console, 0, cout.cursorY, console.getWidth(), 1, ConsoleColour::LightGrey, ConsoleColour::Black);
    ConsoleStream networkSetupHeaderStream(&networkSetupHeader);
    networkSetupHeaderStream << "Setting up network";

    // Make the stream on the side for the network
    ConsoleArea networkConsoleArea(&console, console.getWidth() - 40, 2 + console.getHeight()/2, 45, console.getHeight()/2, ConsoleColour::DarkGrey, ConsoleColour::Black);
    ConsoleStream networkConsoleStream(&networkConsoleArea);
    console.putString(console.getWidth() - 40, 1 + console.getHeight()/2, "                 Network                    ", ConsoleColour::LightGrey, ConsoleColour::Black);

    // Get the driver
    EthernetDriver* ethernetDriver = (EthernetDriver*)driverManager.drivers[4];
    ethernetDriver->driverMessageStream = &networkConsoleStream;
    cout << "Got Ethernet Driver: " << ethernetDriver->getDeviceName() << "\n";
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
    InternetControlMessageProtocol icmp(&internetProtocolHandler);
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


    // Run the network
#define NETWORK
#ifdef NETWORK

    // TCPtoStream
    class TCPtoStream: public TransmissionControlProtocolPayloadHandler{

        ConsoleStream* stream;
        public:
        TCPtoStream(ConsoleStream* stream)
        {
            this->stream = stream;
        }
        ~TCPtoStream()
        {
            this->stream = nullptr;
        }

        void handleTransmissionControlProtocolPayload(TransmissionControlProtocolSocket* socket, uint8_t* data, uint16_t payloadLength)
        {
            *stream << "TCP Payload Received: ";
            for(uint16_t i = 0; i < payloadLength; i++)
            {
                *stream << data[i];
            }
            *stream << "\n";
        }

        void Connected(TransmissionControlProtocolSocket* socket)
        {
            *stream << "TCP Connection Established\n";
        }

        void Disconnected(TransmissionControlProtocolSocket* socket)
        {
            *stream << "TCP Connection Closed\n";
        }
    };
    
    TCPtoStream tcpToStream(&networkConsoleStream);
    TransmissionControlProtocolSocket* tcpSocket = tcp.Listen(1234);
    tcpSocket -> connectEventHandler(&tcpToStream);
    cout << "Listening on TCP Port 1234\n";
    // For sending: ncat -l 127.0.0.1 1234
    // For receiving: ncat 127.0.0.1 1234

#endif

    // Run the GUI

//#define GUI
#ifdef GUI
    Desktop desktop(videoDriver);
    mouse.connectEventHandler(&desktop);
    keyboardInterpreter.connectEventHandler(&desktop);
    kernelClock.connectEventHandler(&desktop);

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

        void onInputBoxTextChanged(common::string newText)
        {
            *stream << "Input Box Changed: " << newText << "\n";
        }
    };
    InputBoxStream inputBoxStream(&cout);
    testInputBox.connectEventHandler(&inputBoxStream);
    testWindow.addChild(&testInputBox);
    desktop.addChild(&testWindow);

    Window testWindow2(350,100, 200, 150, "Test Window 2");
    desktop.addChild(&testWindow2);

#endif

    // Loop forever
    while (true);
}




#pragma clang diagnostic pop

//MaxOS - May the source be with you...
//TODO: Fix networking later TCP/UDP listening, - sending works
