/**
 * @file clock.h
 * @brief Defines drivers for the Programmable Interval Timer (PIT) and APIC Clock
 *
 * @date 10th April 2023
 * @author Max Tyson
 */

#ifndef MAXOS_DRIVERS_CLOCK_CLOCK_H
#define MAXOS_DRIVERS_CLOCK_CLOCK_H

#include <stdint.h>
#include <common/time.h>
#include <drivers/driver.h>
#include <hardwarecommunication/interrupts.h>
#include <hardwarecommunication/apic.h>
#include <common/vector.h>
#include <common/eventHandler.h>

namespace MaxOS {

	namespace drivers {

		namespace clock {

			/**
			 * @enum Channel
			 * @brief Where should the PIT raise an event to
			 *
			 * @warning DRAM_REFRESH may not exist on all systems
			 */
			enum class Channel {
				INTERRUPT,
				DRAM_REFRESH,
				SPEAKER,
			};

			/**
			 * @enum AccessMode
			 * @brief What mode should the PIT use for the selected channel
			 */
			enum class AccessMode {
				LATCH_COUNT,
				LOW_BYTE,
				HIGH_BYTE,
				LOW_HIGH_BYTE,
			};

			/**
			 * @enum OperatingMode
			 * @brief What should the PIT do
			 */
			enum class OperatingMode {
				INTERRUPT_ON_TERMINAL_COUNT,
				ONE_SHOT,
				RATE_GENERATOR,
				SQUARE_WAVE,
				SOFTWARE_STROBE,
				HARDWARE_STROBE,
			};

			/**
			 * @enum BCDMode
			 * @brief Specifies the data encoding mode used by the PIT
			 */
			enum class BCDMode {
				BINARY,
				BCD,
			};

			/**
			 * @struct PITCommand
			 * @brief The command byte to send to the PIT, specifies the channel, access mode, operating mode and BCD mode
			 */
			typedef struct PITCommand {
				uint8_t bcd_mode:       1;      ///< 0 = binary, 1 = BCD
				uint8_t operating_mode: 3;      ///< What mode the PIT should operate in
				uint8_t access_mode:    2;      ///< How the data is accessed
				uint8_t channel:        2;      ///< Which channel to configure
			} pit_command_t;


			// Forward declaration
			class Clock;

			/// How many loops to spin for calibration
			constexpr uint16_t TICK_CALIBRATE_LENGTH = 10;

			/**
			 * @class PIT
			 * @brief Driver for the Programmable Interval Timer
			 */
			class PIT : public hardwarecommunication::InterruptHandler {
				friend Clock;

				private:
					uint64_t m_ticks { 0 };

					// Ports
					hardwarecommunication::Port8Bit m_data_port;
					hardwarecommunication::Port8Bit m_command_port;

					// APIC
					hardwarecommunication::LocalAPIC* m_local_apic;
					hardwarecommunication::IOAPIC* m_io_apic;

					// Funcs
					void handle_interrupt() final;
					uint32_t ticks_per_ms();

				public:
					PIT(hardwarecommunication::AdvancedProgrammableInterruptController* apic);
					~PIT();

			};

			/**
			 * @class Clock
			 * @brief Driver for the APIC clock
			 */
			class Clock : public Driver, public hardwarecommunication::InterruptHandler{
				private:

					uint64_t m_ticks { 0 };

					bool m_binary = true;
					bool m_24_hour_clock = true;

					// Ports
					hardwarecommunication::Port8Bit m_data_port { 0x71 };
					hardwarecommunication::Port8Bit m_command_port { 0x70 };

					// APIC
					hardwarecommunication::AdvancedProgrammableInterruptController* m_apic;

					// Time between events
					uint16_t m_ticks_between_events = 0;
					uint16_t m_ticks_until_next_event = 1;
					uint64_t m_pit_ticks_per_ms = 0;

					// Other functions
					void handle_interrupt() final;
					uint8_t read_hardware_clock(uint8_t address);
					[[nodiscard]] uint8_t binary_representation(uint8_t number) const;

					inline static Clock* s_active_clock = nullptr;

				public:
					Clock(hardwarecommunication::AdvancedProgrammableInterruptController* apic, uint16_t time_between_events = 10);
					~Clock();

					/// How accurate the clock is in microseconds (how often should an interrupt be raised)
					uint64_t clock_accuracy = 1;

					void activate() override;
					void delay(uint32_t milliseconds) const;

					void calibrate(uint64_t ms_per_tick = 1);
					void setup_apic_clock(hardwarecommunication::LocalAPIC* local_apic) const;

					string vendor_name() final;
					string device_name() final;

					static Clock* active_clock();
					common::Time get_time();
			};

		}

	}
}

#endif //MAXOS_DRIVERS_CLOCK_CLOCK_H
