
//Common
#include <common/types.h>
#include <gdt.h>

//Hardware com
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/pci.h>

//Drivers
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>

//GUI
#include <gui/desktop.h>
#include <gui/window.h>
#include <gui/render.h>
#include <gui/widgets/text.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::drivers;
using namespace maxOS::hardwarecommunication;
using namespace maxOS::gui;

void printf(char* str, bool clearLine = false)
{
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;  //Spit the video memory into an array of 16 bit, 4 bit for foreground, 4 bit for background, 8 bit for character

    static uint8_t x = 0, y = 0;    //Cursor Location

    //Screen is 80 wide x 25 high (characters)

    if(clearLine){
        for (int x = 0; x < 80; ++x) {
            //Set everything to a space char
            VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';
        }
        x = 0;
    }

    for(int i = 0; str[i] != '\0'; ++i)     //Increment through each char as long as it's not the end symbol

        switch (str[i]) {

            case '\n':      //If newline
                y++;        //New Line
                x = 0;      //Reset Width pos
                break;

            default:        //(This also stops the \n from being printed)
                VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | str[i];
                x++;
        }
        


        if(x >= 80){    //If at edge of screen
            y++;        //New Line
            x = 0;      //Reset Width pos
        }

        //If at bottom of screen then clear and restart
        if(y >= 25){
            for (int y = 0; y < 25; ++y) {
                for (int x = 0; x < 80; ++x) {
                    //Set everything to a space char
                    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';
                }
            }
            
            x = 0;
            y = 0;

        }
}

void printfHex(uint8_t key){
    char *foo = "00";
    char *hex = "0123456789ABCDEF";

    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    printf(foo);
}

class PrintfKeyboardEventHandler : public KeyboardEventHandler{
    public:
        bool guiTestKeyPressed = false;
        void OnKeyDown(char c){
            char* foo = " ";
            foo[0] = c;
            printf(foo);
            if(c == 't'){
                guiTestKeyPressed = true;
            }
        }
};

class MouseToConsole: public MouseEventHandler{

        int8_t x, y;
    public:
        MouseToConsole()
        {
            static uint16_t* VideoMemory = (uint16_t*)0xb8000;
            x = 40;
            y = 12;
            //Show the initial cursor
            VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4           //Get High 4 bits and shift to right (Foreground becomes Background)
                                  | (VideoMemory[80*y+x] & 0xF000) >> 4         //Get Low 4 bits and shift to left (Background becomes Foreground)
                                  | (VideoMemory[80*y+x] & 0x00FF);             //Keep the last 8 bytes the same (The character)
        }

        void OnMouseMove(int x_offset, int y_offset){

            static uint16_t* VideoMemory = (uint16_t*)0xb8000;

            //Show old cursor
            VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4           //Get High 4 bits and shift to right (Foreground becomes Background)
                                  | (VideoMemory[80*y+x] & 0xF000) >> 4         //Get Low 4 bits and shift to left (Background becomes Foreground)
                                  | (VideoMemory[80*y+x] & 0x00FF);             //Keep the last 8 bytes the same (The character)

            x += x_offset;     //Movement on the x-axis
            y += y_offset;     //Movement on the y-axis (note, mouse passes the info inverted)

            //Make sure mouse position not out of bounds
            if(x < 0) x = 0;
            if(x >= 80) x = 79;

            if(y < 0) y = 0;
            if(y >= 25) y = 24;



            //Show the new cursor by inverting the current character
            VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4           //Get High 4 bits and shift to right (Foreground becomes Background)
                                  | (VideoMemory[80*y+x] & 0xF000) >> 4         //Get Low 4 bits and shift to left (Background becomes Foreground)
                                  | (VideoMemory[80*y+x] & 0x00FF);             //Keep the last 8 bytes the same (The character)
        }

};

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

//[NEW] Version sys
class Version{
    public:
       int version;            //Set based on a noticeable feature update eg. Keyboard Driver etc. [not] code comment updates etc.
       char* version_c;       //Set based on a noticeable feature update eg. Keyboard Driver etc. [not] code comment updates etc.
       int build;              //Commit Number
       char* build_c;          //Commit Number
       char* buildAuthor;      //Auther of the commit (Mostlikly me, but change for pull requsts and such)
        Version(){};
        ~Version(){};

};


#pragma clang diagnostic ignored "-Wwritable-strings"
extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*multiboot_magic*/)
{
    //NOTE: Will rewrite boot text stuff later
    //NOTE: Possibly rename from MaxOS to TyOSn

    Version* maxOSVer;
    maxOSVer->version = 0.15;
    maxOSVer->version_c = "0.15.3";
    maxOSVer->build = 32;
    maxOSVer->build_c = "34";
    maxOSVer->buildAuthor = "Max Tyson";

    printf("Max OS Kernel -v"); printf(maxOSVer->version_c);    printf(" -b"); printf(maxOSVer->build_c);  printf(" -a"); printf(maxOSVer->buildAuthor);


    printf("\n[x] Kernel Booted \n");

    printf("[ ] Setting Up Global Descriptor Table... \n");
    GlobalDescriptorTable gdt;                                                              //Setup GDT
    printf("[x] GDT Setup \n");


    printf("[ ] Setting Up Interrupt Descriptor Table... \n");
    InterruptManager interrupts(0x20, &gdt);            //Instantiate the method

    printf("[ ] Setting Up Interrupt Descriptor Table... \n");

    //Desktop needs the mouse and keyboard
    // so instantiated it here
    Desktop desktop(320,200,0x00,0x00,0xA8);

    printf("[ ] Setting Up Drivers... \n");
    DriverManager driverManager;

        // PrintfKeyboardEventHandler printfKeyboardEventHandler;
        // KeyboardDriver keyboard(&interrupts,&printfKeyboardEventHandler);   //Setup Keyboard drivers
    KeyboardDriver keyboard(&interrupts,&desktop);   //Setup Keyboard drivers
    driverManager.AddDriver(&keyboard);
    printf("    -Keyboard setup\n");

      //  MouseToConsole mouseEventHandler;
      //  MouseDriver mouse(&interrupts, &mouseEventHandler);                 //Setup Mouse drivers
    MouseDriver mouse(&interrupts, &desktop);                 //Setup Mouse drivers
    driverManager.AddDriver(&mouse);
    printf("    -Mouse setup\n");

    printf("    -[ ]Setting PCI\n\n");
    PeripheralComponentInterconnectController PCIController;
    PCIController.SelectDrivers(&driverManager, &interrupts);
    printf("\n    -[x]Setup PCI\n");

    VideoGraphicsArray vga;
    Render rend(320,200);   //arguments do nothing for now. 320,200 is hardcoded into "gui/render.h"
    printf("    -VGA setup\n");

    driverManager.ActivateAll();
    printf("[X] Drivers Setup\n");

    vga.SetMode(320,200,8);

    //Window
    Window debugConsole(&desktop, 10,10,200,150,0x00,0x00,0x00);
    Text debugConsole_title(&debugConsole,2,4,200,10,0x00,0x00,0x00,"Debug Console");
    debugConsole.AddChild(&debugConsole_title);

    Text testText(&debugConsole,2,20,10,10,0xA8,0x00,0x00,"Hello");


    //Add children
    debugConsole.AddChild(&testText);
    desktop.AddChild(&debugConsole);


    printf("GUI is ready ... \n");


    printf("[x] IDT Setup \n", true);
    interrupts.Activate();


    testText.UpdateText("Max OS ");

    while(1){

        //render new frame
        desktop.Draw(&rend);

        //display rendered frame
         rend.display(&vga);
    }                                                                       //Loop
}
#pragma clang diagnostic pop

//NEXT: Text and buttons, window overlapping, mouse release, invert mouse colour on black/white