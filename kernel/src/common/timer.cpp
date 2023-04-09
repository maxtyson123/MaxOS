//
// Created by 98max on 23/11/2022.
//

#include <common/timer.h>

using namespace maxOS;
using namespace common;


void printf(char* str, bool clearLine = false); //Forward declaration
void printfHex(uint8_t key);                    //Forward declaration
char printfInt( long num );                     //Forward declaration

Timer* Timer::activeTimer = 0;
uint32_t Timer::ticks = 0;

Timer::Timer() {
    if(activeTimer == 0)    //If there is no active timer
        activeTimer = this; //Set this timer as the active timer

}

Timer::~Timer() {
    if(activeTimer == this)             //If this is the active timer
        Timer::activeTimer = 0;         //Set the active timer to 0
}

common::uint32_t Timer::GetTicks() {
    return ticks*10;                                //Multiply by 10 to get milliseconds as the frequency is 100Hz where it should be 1000Hz for correct ms
}

/**
 * @details Wait for a certain amount of milliseconds
 *
 * @param ms The amount of milliseconds to wait
 */
void Timer::Wait(common::uint32_t ms) {
    common::uint32_t start = GetTicks();
    while(GetTicks() < start + ms);
}

/**
 * @details Update the ticks
 */
void Timer::UpdateTicks() {

    ticks++;

    return;

    printf("Tick: ", true);
    printfInt(GetTicks());

}

PIT::PIT(common::uint32_t frequency)
: channel0(0x40), channel1(0x41), channel2(0x42), command(0x43) {

    uint16_t divisor = 1193182  / frequency;         //1193182  is the frequency of the PIT

    command.Write(0x34);                        //0x34 is the command to initialize the PIT

    //Divisor has to be sent byte-wise, so split here into upper/lower bytes.
    channel0.Write(divisor & 0xFF);
    channel0.Write(divisor >> 8);

}

PIT::~PIT() {

}
