//
// Created by 98max on 10/04/2023.
//
#include <drivers/clock/clock.h>

using namespace maxOS;
using namespace maxOS::common;
using namespace maxOS::hardwarecommunication;
using namespace maxOS::drivers;
using namespace maxOS::drivers::clock;

///__Handler__

ClockEventHandler::ClockEventHandler() {

}

ClockEventHandler::~ClockEventHandler() {

}

/**
 * @details Called when the clock ticks
 * @param time The current time
 *
 */
void ClockEventHandler::onTime(const Time &time) {

}

void ClockEventHandler::onEvent(Event<ClockEvents>* event) {

    switch (event -> type) {
        case TIME:
            onTime(*((TimeEvent*)event) -> time);
            break;
            
        default:
            break;
    }
}

///__Clock__

/**
 * @details Constructor for the Clock class
 *
 * @param interruptManager The interrupt manager
 * @param timeBetweenEvents The time between events in 10ths of a second
 */
Clock::Clock(InterruptManager *interruptManager, uint16_t timeBetweenEvents)
: Driver(),
  InterruptHandler(interruptManager -> HardwareInterruptOffset(), interruptManager),
  dataPort(0x71),
  commandPort(0x70)
{
    // Set the default values
    ticks = 0;
    ticksBetweenEvents = timeBetweenEvents;
    ticksUntilNextEvent = 1;
}

Clock::~Clock() {

}

/**
 * @details Handle the RTC interrupt, It increments the number of ticks and calls the event clockEventHandlers if the number of ticks is equal to the number of ticks between events
 * @param esp The stack pointer
 */
uint32_t Clock::HandleInterrupt(uint32_t esp) {

    // Increment the number of ticks and decrement the number of ticks until the next event
    ticks++;
    ticksUntilNextEvent--;

    // If the number of ticks until the next event is not 0 then return
    if(ticksUntilNextEvent != 0)
        return esp;

    // Otherwise, reset the number of ticks until the next event
    ticksUntilNextEvent = ticksBetweenEvents;

    // Create a time object
    Time time;

    // Read the time from the hardware clock
    time.year = binaryRepresentation(readHardwareClock(0x9)) + 2000;              // Register 9 is the year
    time.month = binaryRepresentation(readHardwareClock(0x8));                    // Register 8 is the month
    time.day = binaryRepresentation(readHardwareClock(0x7));                      // Register 7 is the day
    time.hour = binaryRepresentation(readHardwareClock(0x4));                     // Register 4 is the hour
    time.minute = binaryRepresentation(readHardwareClock(0x2));                   // Register 2 is the minute
    time.second = binaryRepresentation(readHardwareClock(0x0));                   // Register 0 is the second

    //Raise the clock event
    TimeEvent* event = new TimeEvent(&time);
    raiseEvent(event);
    //TODO: delete event;

    return esp;
}


/**
 * @details read the current time from the hardware clock and return it
 *
 * @param address The address of the register to read from
 * @return The value of the register
 */
uint8_t Clock::readHardwareClock(uint8_t address)
{
    // Check if address is for a time or date register (address < 10), and disable updates to ensure accurate value is read.
    if(address < 10)
    {
        // If the address is for a time or date register, disable updates
        commandPort.Write(0xa);

        // Wait until any updates are finished
        while((dataPort.Read() & (1 << 7)) != 0)
            asm volatile("nop"); // execute the "nop" assembly instruction, which does nothing, but prevents the compiler from optimizing away the loop
    }

    // Send the address to the hardware clock's command port
    commandPort.Write(address);

    // Read the value from the hardware clock's data port and return it
    return dataPort.Read();
}

/**
 * @details The binary representation of a number, if the binary coded decimal representation is used
 * @param number The number to convert
 * @return The binary representation of the number or the number itself if the binary coded decimal representation is not used
 */
common::uint8_t Clock::binaryRepresentation(common::uint8_t number) {

    // If the binary coded decimal representation is not used, return the number
    if(!binaryCodedDecimalRepresentation)
        return number;

    // Otherwise, return the binary representation
    return (number & 0xf) + ((number >> 4) & 0xf) * 10; // And the number with 0xf to get the last 4 bits, then shift it 4 bits to the right and do the same thing

}

/**
 * @details Activates the clock, reading the status register and setting the binary coded decimal representation flag
 */
void Clock::activate() {

    // Read the status register
    uint8_t status = readHardwareClock(0xb);

    // If the fourth bit is 0 the binary coded decimal representation is used
    bool useBinaryCodedDecimal = (status & 4) == 0;


    // Convert status to binary
    // 00001011 = 0x0B (status as an example)
    // 00000100 = 0x04 (binary mask used to extract fourth bit)
    // 00000000 = 0x00 (result of bitwise AND operation with binary mask)
    // 00000000 == 0 (check if fourth bit is 0 to determine if BCD is used)


    // If the binary coded decimal representation is used, set the binary coded decimal representation flag
    binaryCodedDecimalRepresentation = useBinaryCodedDecimal;
}


/**
 * @details Delays the program for a specified number of milliseconds (rounded to the nearest 100) by waiting for the number of ticks to equal the number of ticks until the delay is over
 *
 * @param milliseconds How many milliseconds to delay the program for
 */
void Clock::delay(common::uint32_t milliseconds) {

        // Round the number of milliseconds to the nearest 100
        uint64_t roundedMilliseconds =  ((milliseconds+99)/100);

        // Calculate the number of ticks until the delay is over
        uint64_t ticksUntilDelayIsOver = ticks + roundedMilliseconds;

        // Wait until the number of ticks is equal to the number of ticks until the delay is over
        while(ticks < ticksUntilDelayIsOver)
            asm volatile("nop"); // execute the "nop" assembly instruction, which does nothing, but prevents the compiler from optimizing away the loop
}

common::string Clock::getVendorName() {
    return "Generic";
}

common::string Clock::getDeviceName() {
    return "Clock";
}




TimeEvent::TimeEvent(common::Time* time)
:Event(ClockEvents::TIME)
{
    this -> time = time;
}

TimeEvent::~TimeEvent() {

}
