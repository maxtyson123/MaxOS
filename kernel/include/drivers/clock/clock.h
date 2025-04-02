//
// Created by 98max on 10/04/2023.
//

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

            enum class ClockEvents{
                TIME
            };

            /**
             * @class TimeEvent
             * @brief Event that is triggered when the clock ticks, holds the current time
             */
            class TimeEvent : public common::Event<ClockEvents>{
                public:
                    common::Time* time;
                    explicit TimeEvent(common::Time*);
                    ~TimeEvent();
            };

            /**
             * @class ClockEventHandler
             * @brief Handles the events triggered by the clock
             */
            class ClockEventHandler : public common::EventHandler<ClockEvents>{

                public:
                    ClockEventHandler();
                    ~ClockEventHandler();

                    common::Event<ClockEvents>* on_event(common::Event<ClockEvents>* event) override;

                    virtual void on_time(const common::Time& time);

            };


            enum class Channel {
              INTERRUPT,
              DRAM_REFRESH,
              SPEAKER,
            };

            enum class AccessMode {
              LATCH_COUNT,
              LOW_BYTE,
              HIGH_BYTE,
              LOW_HIGH_BYTE,
            };

            enum class OperatingMode {
              INTERRUPT_ON_TERMINAL_COUNT,
              ONE_SHOT,
              RATE_GENERATOR,
              SQUARE_WAVE,
              SOFTWARE_STROBE,
              HARDWARE_STROBE,
            };

            enum class BCDMode {
              BINARY,
              BCD,
            };

            typedef struct PITCommand{
              uint8_t bcd_mode        : 1;
              uint8_t operating_mode  : 3;
              uint8_t access_mode     : 2;
              uint8_t channel         : 2;
            } pit_command_t;


            // Forward declaration
            class Clock;

            /**
             * @class PIT
             * @brief Driver for the Programmable Interval Timer
             */
            class PIT: public hardwarecommunication::InterruptHandler{
              friend Clock;

              private:

                uint64_t m_ticks { 0 };
                static const uint16_t s_calibrate_ticks = 10;

              protected:

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
            class Clock: public Driver, public hardwarecommunication::InterruptHandler, public common::EventManager<ClockEvents>{
                private:

                  uint64_t m_ticks { 0 };

                protected:
                  
                    bool m_binary { true };
                    bool m_24_hour_clock { true };

                    // Ports
                    hardwarecommunication::Port8Bit m_data_port { 0x71 };
                    hardwarecommunication::Port8Bit m_command_port { 0x70 };

                    // APIC
                    hardwarecommunication::AdvancedProgrammableInterruptController* m_apic;

                    // Time between events
                    uint16_t m_ticks_between_events { 0 };
                    uint16_t m_ticks_until_next_event { 1 };

                    // Other functions
                    void handle_interrupt() final;
                    uint8_t read_hardware_clock(uint8_t address);
                    [[nodiscard]] uint8_t binary_representation(uint8_t number) const;

                public:
                    Clock(hardwarecommunication::AdvancedProgrammableInterruptController* apic, uint16_t time_between_events = 10);
                    ~Clock();

                    inline static uint64_t s_clock_accuracy = 1;

                    void activate() override;
                    void delay(uint32_t milliseconds) const;

                    void calibrate(uint64_t ms_per_tick = 1);
                    common::Time get_time();

                    string get_vendor_name() final;
                    string get_device_name() final;
            };

        }

    }
}

#endif //MAXOS_DRIVERS_CLOCK_CLOCK_H
