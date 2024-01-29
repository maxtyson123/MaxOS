//
// Created by 98max on 10/04/2023.
//
#include <drivers/clock/clock.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::clock;

///__Handler__

ClockEventHandler::ClockEventHandler() {

}

ClockEventHandler::~ClockEventHandler() {

}

/**
 * @brief Called when the clock ticks
 * @param time The current time
 *
 */
void ClockEventHandler::on_time(common::Time const &) {

}

Event<ClockEvents>* ClockEventHandler::on_event(Event<ClockEvents>* event) {

    switch (event -> type) {
        case TIME:
          on_time(*((TimeEvent *)event)->time);
            break;
            
        default:
            break;
    }

    // Return the event
    return event;
}

///__Clock__

/**
 * @brief Constructor for the Clock class
 *
 * @param interrupt_manager The interrupt manager
 * @param time_between_events The time between events in 10ths of a second
 */
Clock::Clock(InterruptManager *interrupt_manager, uint16_t time_between_events)
: Driver(),
  InterruptHandler(interrupt_manager->hardware_interrupt_offset(), interrupt_manager),
  m_data_port(0x71), m_command_port(0x70),
  m_ticks_between_events(time_between_events)
{
  //TODO: Configure APIC Clock
}

Clock::~Clock() {

}

/**
 * @brief Handle the RTC interrupt, raising the clock event if enough time has passed
 *
 */
void Clock::handle_interrupt() {

    // Increment the number of ticks and decrement the number of ticks until the next event
    m_ticks++;
    m_ticks_until_next_event--;

    // If the number of ticks until the next event is not 0 then return
    if(m_ticks_until_next_event != 0)
        return;

    // Otherwise, reset the number of ticks until the next event
    m_ticks_until_next_event = m_ticks_between_events;

    // Wait for the clock to be ready
    while((read_hardware_clock(0xA) & 0x80) != 0);

    // Create a time object
    Time time;

    // read the time from the hardware clock
    time.year = binary_representation(read_hardware_clock(0x9)) + 2000;              // Register 9 is the year
    time.month = binary_representation(read_hardware_clock(0x8));                    // Register 8 is the month
    time.day = binary_representation(read_hardware_clock(0x7));                      // Register 7 is the day
    time.hour = binary_representation(read_hardware_clock(0x4));                     // Register 4 is the hour
    time.minute = binary_representation(read_hardware_clock(0x2));                   // Register 2 is the minute
    time.second = binary_representation(read_hardware_clock(0x0));                   // Register 0 is the second

    // If the clock is using 12hr format and PM is set then add 12 to the hour
    if(!m_24_hour_clock && (time.hour & 0x80) != 0) {

       // Convert the time to 24hr format
       time.hour = ((time.hour & 0x7F) + 12) % 24;

    }

    //Raise the clock event
    raise_event(new TimeEvent(&time));
}


/**
 * @brief read the current time from the hardware clock and return it
 *
 * @param address The address of the register to read from
 * @return The value of the register
 */
uint8_t Clock::read_hardware_clock(uint8_t address)
{
    // Send the address to the hardware clock
    m_command_port.write(address);

    // read the value from the hardware clock
    return m_data_port.read();
}

/**
 * @brief The binary representation of a number
 *
 * @param number The number to convert
 * @return The binary representation of the number if the binary coded decimal representation is used, otherwise the number
 */
uint8_t Clock::binary_representation(uint8_t number) {

    // If the binary coded decimal representation is not used, return the number
    if(m_binary)
        return number;

    // Otherwise, return the binary representation
    return ((number / 16) * 10) + (number & 0x0f);

}

/**
 * @brief Activates the clock, setting the binary coded decimal representation flag
 */
void Clock::activate() {

    // read the status register
    uint8_t status = read_hardware_clock(0xB);

    // Set the clock information
    m_24_hour_clock = status & 0x02;
    m_binary = status & 0x04;
}


/**
 * @brief Delays the program for a specified number of milliseconds (rounded to the nearest 100)
 *
 * @param milliseconds How many milliseconds to delay the program for
 */
void Clock::delay(uint32_t milliseconds) {

        // Round the number of milliseconds to the nearest 100
        uint64_t rounded_milliseconds =  ((milliseconds+99)/100);

        // Calculate the number of ticks until the delay is over
        uint64_t ticks_until_delay_is_over = m_ticks + rounded_milliseconds;

        // Wait until the number of ticks is equal to the number of ticks until the delay is over
        while(m_ticks < ticks_until_delay_is_over)
            asm volatile("nop"); // execute the "nop" assembly instruction, which does nothing, but prevents the compiler from optimizing away the loop
}

/**
 * @brief Gets the name of the vendor
 * @return The name of the vendor
 */
string Clock::get_vendor_name() {
    return "Generic";
}

/**
 * @brief Gets the name of the device
 * @return The name of the device
 */
string Clock::get_device_name() {
    return "Clock";
}


TimeEvent::TimeEvent(Time* time)
:Event(ClockEvents::TIME),
time(time)
{

}

TimeEvent::~TimeEvent() {

}
