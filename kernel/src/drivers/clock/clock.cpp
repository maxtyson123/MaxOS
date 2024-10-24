//
// Created by 98max on 10/04/2023.
//
#include <drivers/clock/clock.h>
#include <common/kprint.h>

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
Clock::Clock(InterruptManager *interrupt_manager, AdvancedProgrammableInterruptController* apic, uint16_t time_between_events)
: Driver(),
  InterruptHandler(0x22, interrupt_manager),
  m_data_port(0x40),
  m_command_port(0x43),
  m_local_apic(apic -> get_local_apic()),
  m_io_apic(apic -> get_io_apic()),
  m_ticks_between_events(time_between_events)
{

  // Set the redirect for the timer interrupt
  interrupt_redirect_t redirect = {
      .type = 0x2,
      .index = 0x14,
      .interrupt = 0x22,
      .destination = 0x00,
      .flags = 0x00,
      .mask = true,
  };
  m_io_apic -> set_redirect(&redirect);

  // Configure the PIT clock
  PITCommand command = {
      .bcd_mode = BINARY,
      .operating_mode = MODE_2,
      .access_mode = LOW_HIGH_BYTE,
      .channel = CHANNEL_0,
  };
  m_command_port.write(*(uint8_t *)&command);

  // Set the clock rate to 1 ms;
  uint16_t rate = 1193182 / 1000;
  m_data_port.write(rate & 0xFF);
  m_data_port.write(rate >> 8);

  // Stop the clock (write 0 as the initial count)
  m_local_apic -> write(0x380, 0x00);

  // Set the divisor to 2
  m_local_apic -> write(0x3E0, 0x0);

  // Unmask the PIT interrupt
  m_io_apic ->set_redirect_mask(redirect.index, false);
  apic -> enable_pic_pit();

  // Calculate the number of ticks in 1 ms
  uint32_t max = (uint32_t) - 1;
  m_local_apic -> write(0x380, max);

  while (m_ticks < 10)
    asm volatile("nop");

  uint32_t elapsed = max - (m_local_apic -> read(0x390));
  uint32_t ticks_per_ms = elapsed / 10;

  _kprintf("Ticks per ms: %d\n", ticks_per_ms);

  // Disable the PIT interrupt again
  m_local_apic -> write(0x380, 0x00);
  m_io_apic -> set_redirect_mask(redirect.index, true);

  // Set the initial count
  m_local_apic -> write(0x380, 100);

  // Enable the APIC timer interrupt
  m_local_apic -> write(0x320, 0x20000 | 0x20);

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
