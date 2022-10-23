
//Common
#include <common/types.h>
#include <common/printf.h>
#include <gdt.h>
#include <multitasking.h>
#include <memorymanagement.h>

//Hardware com
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/pci.h>

//Drivers
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>
#include <drivers/amd_am79c973.h>

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


// #define ENABLE_GRAPHICS

static Console console;

int strcmp(const char *s1, const char *s2)
{
    while ((*s1 == *s2) && *s1) { ++s1; ++s2; }
    return ((int) (unsigned char) *s1) - ((int) (unsigned char) *s2);
}


void printf_gui(char* str, Text lines[15]){

    console.put_string_gui(str,lines);

}

void printf(char* str, bool clearLine = false)
{
    console.put_string(str,clearLine);

}

void printfHex(uint8_t key){
    console.put_hex(key);
}

class PrintfKeyboardEventHandler : public KeyboardEventHandler{
    public:
        bool guiTestKeyPressed = false;
        void OnKeyDown(char* c){

           if(strcmp(c,"ARRT") == 0){

                console.moveCursor(1,0);
                return;

            }else if(strcmp(c,"ARLF") == 0){

                console.moveCursor(-1,0);
                return;

            }else if(strcmp(c,"ARDN") == 0){

                console.moveCursor(0,1);
                return;

            }else if(strcmp(c,"ARUP") == 0){

                console.moveCursor(0,-1);
                return;

            }else if(strcmp(c,"BACKSPACE") == 0){

                console.backspace();

           }else if(strcmp(c,"DEL") == 0){



           }else{

                printf(c);                          //Print Char
                printf("_");                        //Line to know where cursor at
                console.moveCursor(-1,0);      //Move behind the cursor

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


//Define what a constructor is
typedef void (*constructor)();

//Iterates over space between start_ctors and end_ctors and jumps into all function pointers
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();  //envoce constructor calls
}

//[NEW] Version sys
class Version{
    public:
       int version;            //Set based on a noticeable feature update eg. Keyboard Driver etc. [not] code comment updates etc.
       char* version_c;        //Set based on a noticeable feature update eg. Keyboard Driver etc. [not] code comment updates etc.
       int build;              //Commit Number
       char* build_c;          //Commit Number
       char* buildAuthor;      //Author of the commit (Mostlikly me, but change for pull requests and such)
        Version(){};
        ~Version(){};

};


#pragma clang diagnostic ignored "-Wwritable-strings"
extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*multiboot_magic*/)
{


    //NOTE: Will rewrite boot text stuff later
    //NOTE: Possibly rename from MaxOS to TyOSn

    Version* maxOSVer;
    maxOSVer->version = 0.18;
    maxOSVer->version_c = "0.18.1";
    maxOSVer->build = 42;
    maxOSVer->build_c = "42";
    maxOSVer->buildAuthor = "Max Tyson";

    //Print in header
    console.lim_x = 80;
    console.x = 30; console.y = 1;
    printf("* Max OS Kernel -v"); printf(maxOSVer->version_c);    printf(" * -b"); printf(maxOSVer->build_c);  printf(" * -a"); printf(maxOSVer->buildAuthor);

    //Reset Console positions
    console.x = 1; console.y = 3;

    console.ini_x = 2;
    console.ini_y = 4;

    console.lim_x = 79;
    console.lim_y = 24;



    printf("\n[x] Kernel Booted \n");

    printf("[ ] Setting Up Global Descriptor Table... \n");
    GlobalDescriptorTable gdt;                                                              //Setup GDT
    printf("[x] GDT Setup \n");

    printf("[ ] Setting Up Memory Management... \n");
    uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);                    //memupper is a field at offset 8 in the Multiboot information structure and it indicates the amount upper memory in kilobytes.
                                                                                            //Lower memory starts at address 0, and upper memory starts at address 1 megabyte. The
                                                                                            //maximum possible value for lower memory is 640 kilobytes. The value returned for upper
                                                                                            //memory is maximally the address of the first upper memory hole minus 1 megabyte.
    size_t  heap = 10*1024*1024;                                                            //Start at 10MB


    printf("heap: 0x");
    printfHex((heap >> 24) & 0xFF);
    printfHex((heap >> 16) & 0xFF);
    printfHex((heap >> 8 ) & 0xFF);
    printfHex((heap      ) & 0xFF);


    size_t  memSize = (*memupper)*1024 - heap - 10*1024;                                    //Convert memupper into MB, then subtract the hep and some padding
    MemoryManager memoryManager(heap, memSize);                                    //Memory Mangement
    printf(" memSize: 0x");
    printfHex(((size_t)memSize >> 24) & 0xFF);
    printfHex(((size_t)memSize >> 16) & 0xFF);
    printfHex(((size_t)memSize >> 8 ) & 0xFF);
    printfHex(((size_t)memSize      ) & 0xFF);


    void* allocated = memoryManager.malloc(1024);
    printf(" allocated: 0x");
    printfHex(((size_t)allocated >> 24) & 0xFF);
    printfHex(((size_t)allocated >> 16) & 0xFF);
    printfHex(((size_t)allocated >> 8 ) & 0xFF);
    printfHex(((size_t)allocated      ) & 0xFF);
    printf("\n");

    printf("[x] Memory Management Setup \n");
    //TaskManger is up here as needs to use IDT
    printf("[ ] Setting Task Manager... \n");
    TaskManager taskManager;
    printf("[x] Task Manager Setup \n");




    printf("[ ] Setting Up Interrupt Descriptor Table... \n");
    InterruptManager interrupts(0x20, &gdt, &taskManager);            //Instantiate the method



    #ifdef ENABLE_GRAPHICS
        //Desktop needs the mouse and keyboard
        // so instantiated it here
        Desktop desktop(320,200,0x00,0x00,0xA8);
    #endif

    printf("[ ] Setting Up Drivers... \n");
    DriverManager driverManager;
        //Keyboard
        #ifdef ENABLE_GRAPHICS
            KeyboardDriver keyboard(&interrupts,&desktop);   //Setup Keyboard drivers
            driverManager.AddDriver(&keyboard);
        #else
            PrintfKeyboardEventHandler printfKeyboardEventHandler;
            KeyboardDriver keyboard(&interrupts,&printfKeyboardEventHandler);   //Setup Keyboard drivers
        #endif
        printf("    -Keyboard setup\n");


        //Mouse
        #ifdef ENABLE_GRAPHICS
             MouseDriver mouse(&interrupts, &desktop);                 //Setup Mouse drivers
        #else
            MouseToConsole mouseEventHandler;
            MouseDriver mouse(&interrupts, &mouseEventHandler);                 //Setup Mouse drivers

        #endif
        driverManager.AddDriver(&mouse);
        printf("    -Mouse setup\n");

        printf("    -[ ]Setting PCI\n\n");
        PeripheralComponentInterconnectController PCIController;
        PCIController.SelectDrivers(&driverManager, &interrupts);
        printf("\n    -[x]Setup PCI\n");

        #ifdef ENABLE_GRAPHICS
            VideoGraphicsArray vga;
            Render rend(320,200);   //arguments do nothing for now. 320,200 is hardcoded into "gui/render.h"
            printf("    -VGA setup\n");
        #endif
    driverManager.ActivateAll();
    printf("[X] Drivers Setup\n");


    #ifdef ENABLE_GRAPHICS
        vga.SetMode(320,200,8);

        //Window
        Window debugConsole(&desktop, 10,10,250,150,0x00,0x00,0x00);
        Text debugConsole_title(&debugConsole,2,3,200,10,0x00,0x00,0x00,"Debug Console");
        debugConsole.AddChild(&debugConsole_title);


        //Debug console lines (note will make into app later)
        Text lines[15] = {
                Text(&debugConsole,2,20,10,10,0xA8,0x00,0x00,""),
                Text(&debugConsole,2,30,10,10,0xA8,0x00,0x00,""),
                Text(&debugConsole,2,40,10,10,0xA8,0x00,0x00,""),
                Text(&debugConsole,2,50,10,10,0xA8,0x00,0x00,""),
                Text(&debugConsole,2,60,10,10,0xA8,0x00,0x00,""),
                Text(&debugConsole,2,70,10,10,0xA8,0x00,0x00,""),
                Text(&debugConsole,2,80,10,10,0xA8,0x00,0x00,""),
                Text(&debugConsole,2,90,10,10,0xA8,0x00,0x00,""),
                Text(&debugConsole,2,90,10,10,0xA8,0x00,0x00,""),
                Text(&debugConsole,2,100,10,10,0xA8,0x00,0x00,""),
                Text(&debugConsole,2,110,10,10,0xA8,0x00,0x00,""),
                Text(&debugConsole,2,120,10,10,0xA8,0x00,0x00,""),
                Text(&debugConsole,2,130,10,10,0xA8,0x00,0x00,""),
                Text(&debugConsole,2,140,10,10,0xA8,0x00,0x00,""),
                Text(&debugConsole,2,150,10,10,0xA8,0x00,0x00,""),

        };

        for (int i = 0; i <  15; ++i) {
            debugConsole.AddChild(&lines[i]);
        }




        //Add children

        desktop.AddChild(&debugConsole);


        printf_gui("GUI is ready ............ \n",lines);
        printf_gui("Max OS\n\n",lines);
        printf_gui("LONG LINE (*(#()*$(@*#($#*@()$*#@",lines);
        //BUG: Long lines
    #endif

    amd_am79c973* eth0 = (amd_am79c973*)(driverManager.drivers[2]);
    eth0->Send((uint8_t*)"Hello Network", 13);

    //Interrupts should be the last thing as once the clock interrupt is sent the multitasker will start doing processes and task
    printf("[x] IDT Setup \n", true);
    interrupts.Activate();

    //CODE AFTER HERE (interrupts.Activate();) SHOULD BE A TASK

    while(1){
        #ifdef ENABLE_GRAPHICS
            //render new frame
            desktop.Draw(&rend);

            //display rendered frame
             rend.display(&vga);
        #endif
    }                                                                       //Loop
}
#pragma clang diagnostic pop

//GUI NEXT: buttons, window overlapping, mouse release, invert mouse colour on black/white