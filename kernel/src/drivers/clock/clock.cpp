/**
 * @file clock.cpp
 * @brief Implementation of a Clock driver for handling system time and clock interrupts
 *
 * @date 10th April 2023
 * @author Max Tyson
 */

#include <drivers/clock/clock.h>
#include <common/logger.h>

using namespace MaxOS;
using namespace MaxOS::common;
using namespace MaxOS::hardwarecommunication;
using namespace MaxOS::drivers;
using namespace MaxOS::drivers::clock;

/**
 * @brief Constructor for the Clock class
 *
 * @param apic The apic controller for this core
 * @param time_between_events The time between events in 10ths of a second
 */
Clock::Clock(AdvancedProgrammableInterruptController* apic, uint16_t time_between_events)
: InterruptHandler(0x20),
  m_apic(apic),
  m_ticks_between_events(time_between_events)
{
	Logger::INFO() << "Setting up Clock \n";
}

Clock::~Clock() = default;

/**
 * @brief Handle the clock interrupt, raising the clock event if enough time has passed
 *
 */
void Clock::handle_interrupt() {

	// Clock has ticked
	m_ticks++;
	m_ticks_until_next_event--;

	// Dont raise events until needed
	if (m_ticks_until_next_event != 0)
		return;

	// Raise the time event
//    Time time = get_time();
//    raise_event(new TimeEvent(&time));

	// Reset
	m_ticks_until_next_event = m_ticks_between_events;
}


/**
 * @brief read the current time from the hardware clock and return it
 *
 * @param address The address of the register to read from
 * @return The value of the register
 */
uint8_t Clock::read_hardware_clock(uint8_t address) {

	m_command_port.write(address);
	return m_data_port.read();
}

/**
 * @brief The binary representation of a number
 *
 * @param number The number to convert
 * @return The binary representation of the number if the binary coded decimal representation is used, otherwise the number
 */
uint8_t Clock::binary_representation(uint8_t number) const {

	// Check if the conversion needed
	if (m_binary)
		return number;

	// Convert to the binary representation
	return ((number / 16) * 10) + (number & 0x0f);
}

/**
 * @brief Activates the clock, setting the binary coded decimal representation flag
 */
void Clock::activate() {

	s_active_clock = this;

	// Get the stats from the clock
	uint8_t status = read_hardware_clock(0xB);

	// Store the clock status
	m_24_hour_clock = status & 0x02;
	m_binary = status & 0x04;
}

/**
 * @brief Delays the program for a specified number of milliseconds
 * (rounded up to the nearest degree of accuracy - ensured the delay is at least the specified number of milliseconds).
 * This on the kernel level is a busy wait, for user level see the sleep function in the Thread class.
 *
 *
 * @see Thread::sleep
 *
 * @param milliseconds How many milliseconds to delay the program for.
 */
void Clock::delay(uint32_t milliseconds) const {

	// Round the number of milliseconds UP to the nearest clock accuracy
	uint64_t rounded_milliseconds = (milliseconds + clock_accuracy - 1) / clock_accuracy;

	// Wait until the time has passed
	uint64_t ticks_until_delay_is_over = m_ticks + rounded_milliseconds;
	while (m_ticks < ticks_until_delay_is_over)
		asm volatile("nop");
}

/**
 * @brief Gets the vendor who created the device
 *
 * @return The name of the vendor
 */
string Clock::vendor_name() {
	return "Generic";
}

/**
 * @brief Gets the name of the device
 *
 * @return The name of the device
 */
string Clock::device_name() {
	return "Clock";
}

/**
 * @brief Configures the APIC clock to fire an interrupt at a specified interval in milliseconds
 *
 * @param ms_per_tick How many milliseconds per interrupt
 */
void Clock::calibrate(uint64_t ms_per_tick) {

	Logger::INFO() << "Calibrating Clock \n";
	clock_accuracy = ms_per_tick;

	// Get the ticks per ms
	PIT pit(m_apic);
	m_pit_ticks_per_ms = pit.ticks_per_ms();

	// Calibrate the BSP apic to the desired time
	setup_apic_clock(m_apic->local_apic());

	Logger::DEBUG() << "Clock: Calibrated to " << ms_per_tick << "ms per kernel tick\n";
}

/**
 * @brief Sets up the APIC clock to fire interrupts at the desired rate
 * @param local_apic The local APIC to setup the clock on
 */
void Clock::setup_apic_clock(hardwarecommunication::LocalAPIC* local_apic) const {

	// Configure the clock to periodic mode
	uint32_t lvt = 0x20 | (1 << 17);
	local_apic->write(0x320, lvt);

	// Set the initial count
	local_apic->write(0x380, m_pit_ticks_per_ms * clock_accuracy);

	// Clear the interrupt mask for the clock
	lvt &= ~(1 << 16);
	local_apic->write(0x320, lvt);
}

/**
 * @brief Reads the current time from the APIC clock (in 24hr time)
 *
 * @return The current time in a Time struct format
 */
common::Time Clock::get_time() {

	// Wait for the clock to be ready
	while (read_hardware_clock(0xA) & 0x80)
		asm volatile("nop");

	// Read the time from the clock
	Time time{};
	time.year = binary_representation(read_hardware_clock(0x9)) + 2000;
	time.month = binary_representation(read_hardware_clock(0x8));
	time.day = binary_representation(read_hardware_clock(0x7));
	time.hour = binary_representation(read_hardware_clock(0x4));
	time.minute = binary_representation(read_hardware_clock(0x2));
	time.second = binary_representation(read_hardware_clock(0x0));

	// If the clock is using 12hr format and PM is set then add 12 to the hour
	if (!m_24_hour_clock && (time.hour & 0x80) != 0)
		time.hour = ((time.hour & 0x7F) + 12) % 24;

	return time;
}

/**
 * @brief Gets the currently active clock
 * @return The clock being used by the kernel
 */
Clock *Clock::active_clock() {
	return s_active_clock;
}

/**
 * @brief Constructor for the PIT class. Registers a handler for interrupt 0x22 and initializes ports
 * @param apic The APIC controller for the BSP core
 */
PIT::PIT(AdvancedProgrammableInterruptController *apic)
: InterruptHandler(0x22),
  m_data_port(0x40),
  m_command_port(0x43),
  m_local_apic(apic->local_apic()),
  m_io_apic(apic->io_apic())
{

}

PIT::~PIT() = default;

/**
 * @brief Tick on each interrupt
 */
void PIT::handle_interrupt() {
	m_ticks++;
}

/**
 * @brief Uses the PIT to calculate how many APIC ticks are in 1 millisecond
 *
 * @return The amount of APIC ticks per millisecond
 */
uint32_t PIT::ticks_per_ms() {

	// Set the redirect for the timer interrupt
	interrupt_redirect_t redirect = {
			.type = 0x2,
			.index = 0x14,
			.interrupt = 0x22,
			.destination = 0x00,
			.flags = 0x00,
			.mask = true,
	};
	m_io_apic->set_redirect(&redirect);

	// Configure the PIT clock
	pit_command_t command = {
			.bcd_mode       = (uint8_t) BCDMode::BINARY,
			.operating_mode = (uint8_t) OperatingMode::RATE_GENERATOR,
			.access_mode    = (uint8_t) AccessMode::LOW_HIGH_BYTE,
			.channel        = (uint8_t) Channel::INTERRUPT
	};
	m_command_port.write(*(uint8_t *) &command);

	// Set the clock rate to 1 ms;
	uint16_t rate = 1193182 / 1000;
	m_data_port.write(rate & 0xFF);
	m_data_port.write(rate >> 8);

	// Stop the clock
	m_local_apic->write(0x380, 0x00);

	// Set the divisor to 2
	m_local_apic->write(0x3E0, 0x0);

	// Unmask the PIT interrupt
	m_io_apic->set_redirect_mask(redirect.index, false);

	// Calculate the number of ticks in 1 ms
	auto max = (uint32_t) -1;
	m_local_apic->write(0x380, max);

	while (m_ticks < TICK_CALIBRATE_LENGTH)
		asm volatile("nop");

	uint32_t elapsed = max - (m_local_apic->read(0x390));
	uint32_t ticks_per_ms = elapsed / TICK_CALIBRATE_LENGTH;

	Logger::DEBUG() << "Ticks per ms: " << (int) ticks_per_ms << "\n";

	// Disable the PIT interrupt again
	m_local_apic->write(0x380, 0x00);
	m_io_apic->set_redirect_mask(redirect.index, true);

	return ticks_per_ms;
}
