#include "types.h"
#include "gdt.h"
#include "interrupts.h"

void printf(char* str)
{
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;

    static uint8_t x = 0, y = 0;    //Cursor Location

    //Screen is 80 wide x 25 high (characters)

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
    printf("Maxs Test Kernel -v9 -b12         \n");
    printf("[x] Kernel Booted \n");

    printf("[ ] Setting Up Global Discriptor Table... \n");
    GlobalDescriptorTable gdt; //Setup GDT
    printf("[x] GDT Setup \n");

    printf("[ ] Setting Up Interrupt Discriptor Table... \n");
    InterruptManager interrupts(&gdt);//Setup IDT
    //Setup Hardware
    //--
    interrupts.Activate(); //Activate as separate method from constructor as we first instantiated the method, then the hardware
    printf("[x] IDT Setup \n");

    while(1);                  //Loop
}