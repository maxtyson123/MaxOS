//
// Created by 98max on 21/10/2022.
//

#include <common/printf.h>

using namespace maxOS;
using namespace maxOS::common;

Console::Console() {

    //Set default limits
    lim_x = 80;
    lim_y = 26;

    //Draw Top

    x = 0; y = 0; put_string("******************************************************************************** ");
    x = 0; y = 1; put_string("* Debug Console                ******                                          * ");
    x = 0; y = 2; put_string("******************************************************************************** ");

    //Draw Side
    for (int i = 3; i < 23; ++i) {
        x = 0; y = i; put_string("*                                                                              * ");
    }

    //Draw bottom
    x = 0; y = 24; put_string("******************************************************************************** ");


}

Console::~Console() {

}


void Console::put_string_gui(char* str, gui::Text lines[15]){

    static uint8_t gx = 0, gy = 0;    //Cursor Location
    static char lineText[15][29];
    //Debug Console is 29 wide x 15 high

    for(int i = 0; str[i] != '\0'; ++i){     //Increment through each char as long as it's not the end symbol

        switch (str[i]) {

            case '\n':      //If newline
                gy++;        //New Line
                gx = 0;      //Reset Width pos
                break;

            default:        //(This also stops the \n from being printed)
                lineText[gy][gx] = str[i];
                gx++;

        }

        if(gx >= 29){    //If at edge of screen

            gy++;
            gx = 0;
        }

        //If at bottom of screen then clear and restart
        if(gy >= 15){
            for (int y = 0; y < 15; ++y) {
                for (int x = 0; x < 29; ++x) {
                    //Set everything to a space char
                    lineText[y][x] = ' ';
                }
            }

            gx = 0;
            gy = 0;

        }
    }
    for (int i = 0; i < 15; ++i) {
        lines[i].UpdateText(lineText[i]);
    }

}

void Console::put_string(char* str, bool clearLine)
{
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;  //Spit the video memory into an array of 16 bit, 4 bit for foreground, 4 bit for background, 8 bit for character



    //Screen is 80 wide x 25 high (characters)
    static char displayed[25][80];

    if(clearLine){
        for (int x = 0; x < lim_x; ++x) {
            //Set everything to a space char
            displayed[y][x] = ' ';
        }
        x = ini_x;
    }

    for(int i = 0; str[i] != '\0'; ++i){     //Increment through each char as long as it's not the end symbol

        switch (str[i]) {

            case '\n':      //If newline
                y++;             //New Line
                x = ini_x;      //Reset Width pos
                break;

            default:        //(This also stops the \n from being printed)
                displayed[y][x] = str[i];
                x++;
        }



        if(x >= lim_x){    //If at edge of screen
            y++;        //New Line
            x = ini_x;      //Reset Width pos
        }

        //If at bottom of screen then clear and restart
        if(y >= lim_y){

            for (int y = ini_y; y < lim_y; ++y) {
                for (int x = ini_x; x < lim_x; ++x) {
                    displayed[y][x] = displayed[y+1][x];            //Shift up by 1
                }
            }

            x = ini_x;
            y = lim_y-1;

            //Draw the empty line
            for (int x = 1; x < 79; ++x) {
                displayed[y][x] = ' ';
            }

            //Draw the border
            displayed[y][0] = '*';
            displayed[y][79] = '*';

        }
    }

    //Write to video memory
    for (int y = 0; y < 25; ++y) {
        for (int x = 0; x < 80; ++x) {
            VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0xFF00) | displayed[y][x];
        }
    }

}

void Console::put_hex(uint8_t key){
    char *foo = "00";
    char *hex = "0123456789ABCDEF";

    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    put_string(foo);
}

void Console::moveCursor(uint8_t C_x, uint8_t C_y){

    //Move cursor by specified amount
    x += C_x;
    y += C_y;

    //Check width overflow
    if(x >= 79) {
        x = 0;
        y++;
    }
    if(x <= 0){
        x = 79;
        y--;
    }

    //Y doesn't need an overflow as scrolling


}

void Console::backspace(){
    put_string(" ");                //Remove the cursor pointer in front
    moveCursor(-2,0);          //Go back 2
    put_string(" ");                //Remove that character
    moveCursor(-1,0);          //Go back to the characters position

    if(x <= 0){
        y--;
        x = 78;
    }

    put_string("_");                 //Line to know where cursor at
    moveCursor(-1,0);          //Move behind the cursor

}

