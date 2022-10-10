
#include "types.h"
#include "gdt.h"
#include "interrupts.h"
#include "keyboard.h"
#include "mouse.h"

void printf(char* str, bool clearLine = false)
{
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

    static uint8_t x = 0, y = 0;    //Cursor Location

    //Screen is 80 wide x 25 high (characters)

    if(clearLine){
        for (int x = 0; x < 80; ++x) {
            //Set everything to a space char
            VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | ' ';
        }
        x = 0;
    }

    for(int i = 0; str[i] != '\0'; ++i)     //Increment through each char as long as its not the end symbol

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



typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}



extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*multiboot_magic*/)
{
    printf("Max OS Kernel -v11 -b16         \n");
    printf("[x] Kernel Booted \n");

    printf("[ ] Setting Up Global Descriptor Table... \n");
    GlobalDescriptorTable gdt;                                                      //Setup GDT
    printf("[x] GDT Setup \n");

    printf("[ ] Setting Up Interrupt Descriptor Table... \n");
    InterruptManager interrupts(0x20, &gdt);      //Instantite the method
    KeyboardDriver keyboard(&interrupts);                                   //Setup Keyboard driver
    printf("    -Keyboard setup\n");
    MouseDriver mouse(&interrupts);                                         //Setup Mouse driver
    printf("    -Mouse setup\n");
    interrupts.Activate();                                                          //Activate as separate method from constructor as we first instantiated the method, then the hardware
    printf("[x] IDT Setup \n", true);

    while(1);                                                                       //Loop
}